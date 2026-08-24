// SPDX-License-Identifier: GPL-2.0
/*
 * APU-side mailbox self-test driver.
 *
 * Sends crafted test frames (with deliberately bad CRC/seq/cookie) to the RPU
 * self-test handler and collects per-case PASS/FAIL verdicts.  All code is
 * gated by MBOX_ENABLE_SELFTEST; zero footprint when the flag is off.
 *
 * Trigger: echo <rpu_id> > /proc/visp_mbox/selftest
 * Requires: mbox_integrity module param = 1
 */

#ifdef MBOX_ENABLE_SELFTEST

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include "../visp_mbox_driver.h"
#include "mbox_cmd.h"
#include "mbox_api.h"
#include "mbox_crc.h"
#include "mbox_selftest.h"
#include "mbox_selftest_types.h"
#include "mbox_seq.h"
#include "sensor_cmd.h"

#define SELFTEST_RESP_TIMEOUT_MS	2000
#define SELFTEST_COOKIE_NOT_USED	MBOX_COOKIE_NOT_USED

/* Per-module state — single suite run at a time */
static DECLARE_COMPLETION(st_response_received);
static mbox_post_msg st_last_response;
static bool st_active;

bool visp_mbox_selftest_active(void)
{
	return st_active;
}

/* -----------------------------------------------------------------------
 * Raw send: bypasses integrity stamping (CRC/seq) so test frames can carry
 * deliberately corrupt fields.  Must hold rpu->write_lock; does NOT advance
 * outbound_seq.
 * -----------------------------------------------------------------------
 */
static int selftest_raw_send(struct rpu_dev *rpu, mbox_post_msg *msg,
			     mbox_core_id receiver_id)
{
	int ret;

	if (msg->size > MAX_PAYLOAD_SIZE) {
		dev_err(rpu->dev, "selftest: frame too large (%u)\n", msg->size);
		return -EINVAL;
	}

	mutex_lock(&rpu->write_lock);
	ret = vpi_mbox_post(rpu->apu_tx_ctrl, msg, receiver_id, NULL);
	mutex_unlock(&rpu->write_lock);
	if (ret == VPI_SUCCESS && rpu->tx_chan) {
		int ipi_ret = mbox_send_message(rpu->tx_chan, NULL);

		if (ipi_ret < 0) {
			dev_err(rpu->dev,
				"selftest: mailbox IPI send failed %d\n", ipi_ret);
			ret = ipi_ret;
		} else {
			/* Positive mailbox cookies are successful sends. */
			ret = VPI_SUCCESS;
		}
	}

	if (ret != VPI_SUCCESS)
		dev_err(rpu->dev, "selftest: vpi_mbox_post failed %d\n", ret);
	return ret;
}

/* -----------------------------------------------------------------------
 * Fill the 16-byte STMB descriptor at payload_data[0] (outer offset 48).
 * -----------------------------------------------------------------------
 */
static void selftest_fill_desc(payload_packet *pkt, mbox_st_case_e case_id,
			       mbox_st_verdict_e expected, u32 pre_action)
{
	mbox_selftest_desc_t *desc = (mbox_selftest_desc_t *)pkt->payload;

	desc->magic      = MBOX_ST_DESC_MAGIC;
	desc->case_id    = (u32)case_id;
	desc->expected   = (u32)expected;
	desc->pre_action = pre_action;
}

/* -----------------------------------------------------------------------
 * Build and raw-send one test frame.
 *
 * @seq_override: if >= 0, overrides seq_counter with this value (enables
 *                crafting bad/specific seq numbers).
 * @crc_override: if != 0xFFFFFFFF, overwrites the checksum field after the
 *                CRC has been calculated (enables bad-CRC frames).
 * @cookie_val:   cookie to use in the Payload_packet.
 * -----------------------------------------------------------------------
 */
static int selftest_send_case(struct rpu_dev *rpu, mbox_core_id dest,
			      mbox_st_case_e case_id,
			      mbox_st_verdict_e expected,
			      u32 pre_action,
			      s64 seq_override,
			      u32 crc_override,
			      u32 cookie_val)
{
	mbox_post_msg *msg;
	payload_packet *pkt;

	msg = visp_get_tx_buffer(rpu);
	if (!msg)
		return -ENOMEM;

	memset(msg, 0, sizeof(*msg));
	msg->msg_id = APU_2_RPU_MB_CMD_MBOX_SELFTEST;

	pkt = (payload_packet *)msg->payload;
	pkt->type        = 0; /* CMD */
	pkt->cookie      = cookie_val;
	pkt->payload_size = sizeof(mbox_selftest_desc_t);
	pkt->seq_counter = (u32)rpu->outbound_seq; /* default; overridden below */

	selftest_fill_desc(pkt, case_id, expected, pre_action);

	visp_mbox_set_message_size(msg, pkt);

	/* Apply sequence overrides before calculating the frame CRC. */
	if (seq_override >= 0)
		pkt->seq_counter = (u32)seq_override;

	/* Stamp a valid CRC over the final frame contents. */
	msg->checksum = visp_mbox_calculate_checksum(msg);

	if (crc_override != 0xFFFFFFFF) {
		if (case_id == MBOX_ST_CASE_CRC_HIGH_BITS)
			msg->checksum |= crc_override;
		else
			msg->checksum = crc_override;
	}

	reinit_completion(&st_response_received);

	if (selftest_raw_send(rpu, msg, dest)) {
		visp_free_tx_buffer(rpu, msg);
		return -EIO;
	}
	visp_free_tx_buffer(rpu, msg);

	return 0;
}

/* -----------------------------------------------------------------------
 * Send BEGIN (well-formed, normal write_mboxcmd path) and wait for ACK.
 * -----------------------------------------------------------------------
 */
static int selftest_send_begin(struct rpu_dev *rpu, mbox_core_id dest)
{
	mbox_post_msg *msg;
	payload_packet *pkt;
	long wait_ret;
	int ret;

	msg = visp_get_tx_buffer(rpu);
	if (!msg)
		return -ENOMEM;

	memset(msg, 0, sizeof(*msg));
	msg->msg_id = APU_2_RPU_MB_CMD_MBOX_SELFTEST_BEGIN;
	pkt = (payload_packet *)msg->payload;
	pkt->type = 0;
	pkt->cookie = SELFTEST_COOKIE_NOT_USED;
	pkt->payload_size = 0;
	visp_mbox_set_message_size(msg, pkt);
	msg->checksum = visp_mbox_calculate_checksum(msg);
	reinit_completion(&st_response_received);

	ret = selftest_raw_send(rpu, msg, dest);
	visp_free_tx_buffer(rpu, msg);
	if (ret)
		return ret;

	wait_ret = wait_for_completion_timeout(&st_response_received,
					       msecs_to_jiffies(SELFTEST_RESP_TIMEOUT_MS));
	if (!wait_ret)
		return -ETIMEDOUT;

	if (((payload_packet *)st_last_response.payload)->resp_field.processed_cmdid !=
	     APU_2_RPU_MB_CMD_MBOX_SELFTEST_BEGIN)
		return -EPROTO;

	return 0;
}

/* -----------------------------------------------------------------------
 * Send END (well-formed) and wait for summary response.
 * -----------------------------------------------------------------------
 */
static int selftest_send_end(struct rpu_dev *rpu, mbox_core_id dest)
{
	mbox_post_msg *msg;
	payload_packet *pkt;
	long wait_ret;
	int ret;

	msg = visp_get_tx_buffer(rpu);
	if (!msg)
		return -ENOMEM;

	memset(msg, 0, sizeof(*msg));
	msg->msg_id = APU_2_RPU_MB_CMD_MBOX_SELFTEST_END;
	pkt = (payload_packet *)msg->payload;
	pkt->type = 0;
	pkt->cookie = SELFTEST_COOKIE_NOT_USED;
	pkt->payload_size = 0;
	visp_mbox_set_message_size(msg, pkt);
	msg->checksum = visp_mbox_calculate_checksum(msg);
	reinit_completion(&st_response_received);

	ret = selftest_raw_send(rpu, msg, dest);
	visp_free_tx_buffer(rpu, msg);
	if (ret)
		return ret;

	wait_ret = wait_for_completion_timeout(&st_response_received,
					       msecs_to_jiffies(SELFTEST_RESP_TIMEOUT_MS));
	if (!wait_ret)
		return -ETIMEDOUT;

	if (((payload_packet *)st_last_response.payload)->resp_field.processed_cmdid !=
	     APU_2_RPU_MB_CMD_MBOX_SELFTEST_END)
		return -EPROTO;

	return 0;
}

/* -----------------------------------------------------------------------
 * Called from the RX handler when msg_id == APU_2_RPU_MB_CMD_MBOX_SELFTEST
 * and st_active is set.  Copies the response and signals the waiter.
 * -----------------------------------------------------------------------
 */
void visp_mbox_selftest_rx_response(const mbox_post_msg *msg)
{
	if (!st_active)
		return;
	memcpy(&st_last_response, msg, sizeof(*msg));
	complete(&st_response_received);
}

/* -----------------------------------------------------------------------
 * Wait for a SELFTEST response from the RPU and log PASS/FAIL.
 * Returns 0 on PASS, -EIO on FAIL or timeout.
 * -----------------------------------------------------------------------
 */
static int selftest_wait_and_score(struct rpu_dev *rpu, mbox_st_case_e case_id,
				   mbox_st_verdict_e expected,
				   int *total, int *passed)
{
	payload_packet *pkt;
	u32 actual_verdict;
	u8 pass_byte;
	long ret;

	ret = wait_for_completion_timeout(&st_response_received,
					  msecs_to_jiffies(SELFTEST_RESP_TIMEOUT_MS));
	(*total)++;

	if (!ret) {
		dev_err(rpu->dev,
			"MBOX_SELFTEST FAIL case=%u: timeout (no response)\n",
			case_id);
		return -ETIMEDOUT;
	}

	pkt = (payload_packet *)st_last_response.payload;
	actual_verdict = pkt->resp_field.error_subcode_t;
	pass_byte = pkt->payload[0];

	/* A late reply for an earlier case must not be scored against this one. */
	if (pkt->resp_field.processed_cmdid != (u32)case_id) {
		dev_err(rpu->dev,
			"MBOX_SELFTEST FAIL case=%u: response is for case %u\n",
			case_id, pkt->resp_field.processed_cmdid);
		return -EIO;
	}

	if (pass_byte && actual_verdict == (u32)expected) {
		dev_info(rpu->dev,
			 "MBOX_SELFTEST PASS case=%u verdict=%u\n",
			 case_id, actual_verdict);
		(*passed)++;
		return 0;
	}

	dev_err(rpu->dev,
		"MBOX_SELFTEST FAIL case=%u expected=%u got=%u pass_byte=%u\n",
		case_id, expected, actual_verdict, pass_byte);
	return -EIO;
}

/* -----------------------------------------------------------------------
 * Wrapper: build + send + score one case.
 * -----------------------------------------------------------------------
 */
static void run_case(struct rpu_dev *rpu, mbox_core_id dest,
		     mbox_st_case_e case_id, mbox_st_verdict_e expected,
		     u32 pre_action, s64 seq_override, u32 crc_override,
		     u32 cookie_val, int *total, int *passed)
{
	int ret;

	ret = selftest_send_case(rpu, dest, case_id, expected, pre_action,
				 seq_override, crc_override, cookie_val);
	if (ret) {
		dev_err(rpu->dev, "MBOX_SELFTEST: send failed case=%u ret=%d\n",
			case_id, ret);
		(*total)++;
		return;
	}
	selftest_wait_and_score(rpu, case_id, expected, total, passed);
}

/* -----------------------------------------------------------------------
 * Main entry point.  Called from procfs write handler.
 * -----------------------------------------------------------------------
 */
int visp_mbox_selftest_run(struct rpu_dev *rpu, mbox_core_id dest)
{
	int total = 0, passed = 0;
	u32 fixed_cookie = 0x00001234; /* arbitrary non-sentinel cookie for dedup cases */
	int ret;

	if (!visp_mbox_integrity_active()) {
		dev_err(rpu->dev,
			"MBOX_SELFTEST: requires mbox_integrity=1\n");
		return -EINVAL;
	}

	st_active = true;

	ret = selftest_send_begin(rpu, dest);
	if (ret) {
		dev_err(rpu->dev, "MBOX_SELFTEST: BEGIN failed %d\n", ret);
		st_active = false;
		visp_mbox_mark_seq_resync(rpu);
		return ret;
	}

	/* --- G1: CRC cases ------------------------------------------------ */
	/* CRC_VALID: normal well-formed frame */
	run_case(rpu, dest, MBOX_ST_CASE_CRC_VALID, MBOX_ST_EXPECT_ACCEPT,
		 0, -1, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* CRC_CORRUPT_PAYLOAD: flip the checksum to a wrong value */
	run_case(rpu, dest, MBOX_ST_CASE_CRC_CORRUPT_PAYLOAD, MBOX_ST_EXPECT_DROP,
		 0, -1, 0xDEAD, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* CRC_HIGH_BITS: preserve the valid low-16 bits and add junk above it. */
	run_case(rpu, dest, MBOX_ST_CASE_CRC_HIGH_BITS, MBOX_ST_EXPECT_ACCEPT,
		 0, -1, 0xABCD0000, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* CRC_ZERO: force checksum to 0x0000 -> plain CRC fail -> DROP */
	run_case(rpu, dest, MBOX_ST_CASE_CRC_ZERO, MBOX_ST_EXPECT_DROP,
		 0, -1, 0x0000, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* --- G2: SEQ cases ------------------------------------------------ */
	/* SEQ_IN_ORDER: expected=0, send seq=0 */
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_IN_ORDER, MBOX_ST_EXPECT_ACCEPT,
		 MBOX_ST_PRE_RESET_SEQ, 0, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* SEQ_FORWARD_GAP: reset, send seq=5 (gap=5, <= 8 -> RESYNC) */
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_FORWARD_GAP, MBOX_ST_EXPECT_RESYNC,
		 MBOX_ST_PRE_RESET_SEQ, 5, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* SEQ_GAP_OVER_CAP: reset, send seq=9 -> both sides DROP. */
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_GAP_OVER_CAP, MBOX_ST_EXPECT_DROP,
		 MBOX_ST_PRE_RESET_SEQ, 9, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* SEQ_BACKWARD (2-frame recipe):
	 *   Frame 1 (primer): RESET_SEQ, seq=6 -> RPU RESYNC, expected becomes 7
	 *   Frame 2 (scored): NO reset, seq=5 -> RPU DROP (backward)
	 */
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_FORWARD_GAP, MBOX_ST_EXPECT_RESYNC,
		 MBOX_ST_PRE_RESET_SEQ, 6, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_BACKWARD, MBOX_ST_EXPECT_DROP,
		 0, 5, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* SEQ_OUT_OF_RANGE (D3): both sides reject high-bit sequence values. */
	run_case(rpu, dest, MBOX_ST_CASE_SEQ_OUT_OF_RANGE, MBOX_ST_EXPECT_DROP,
		 MBOX_ST_PRE_RESET_SEQ, 0x80000001, 0xFFFFFFFF,
		 SELFTEST_COOKIE_NOT_USED, &total, &passed);
	/* Rollover is covered by the RPU internal self-test, not this wire matrix. */

	/* --- G3: cookie de-dup cases -------------------------------------- */
	/* COOKIE_NEW: first send of a fresh cookie -> RPU caches it */
	run_case(rpu, dest, MBOX_ST_CASE_COOKIE_NEW, MBOX_ST_EXPECT_ACCEPT,
		 MBOX_ST_PRE_RESET_DEDUP, -1, 0xFFFFFFFF, fixed_cookie, &total, &passed);

	/* COOKIE_REPLAY: same cookie again -> RPU replays cached response */
	run_case(rpu, dest, MBOX_ST_CASE_COOKIE_REPLAY, MBOX_ST_EXPECT_REPLAY,
		 0, -1, 0xFFFFFFFF, fixed_cookie, &total, &passed);

	/* COOKIE_SUPPRESS: same cookie a 3rd time -> RPU suppresses */
	run_case(rpu, dest, MBOX_ST_CASE_COOKIE_SUPPRESS, MBOX_ST_EXPECT_SUPPRESS,
		 0, -1, 0xFFFFFFFF, fixed_cookie, &total, &passed);

	/* COOKIE_NOT_USED: sentinel 0xFFFFFFFF -> bypass dedup -> ACCEPT */
	run_case(rpu, dest, MBOX_ST_CASE_COOKIE_NOT_USED, MBOX_ST_EXPECT_ACCEPT,
		 0, -1, 0xFFFFFFFF, SELFTEST_COOKIE_NOT_USED, &total, &passed);

	/* COOKIE_DUP_BEFORE_CACHE: current RPU handler evaluates one frame at a
	 * time and caches immediately on NEW, so this first observation is ACCEPT.
	 */
	run_case(rpu, dest, MBOX_ST_CASE_COOKIE_DUP_BEFORE_CACHE, MBOX_ST_EXPECT_ACCEPT,
		 MBOX_ST_PRE_RESET_DEDUP, -1, 0xFFFFFFFF, fixed_cookie, &total, &passed);

	/* --- END: fresh-state reset --------------------------------------- */
	selftest_send_end(rpu, dest);

	st_active = false;

	/*
	 * Suite responses were consumed before visp_mbox_validate_seq(), so
	 * inbound_seq never followed the RPU's outbound counter.  Adopt the
	 * next production response's sequence instead of faulting on it.
	 */
	visp_mbox_mark_seq_resync(rpu);

	if (passed == total)
		dev_info(rpu->dev,
			 "MBOX_SELFTEST COMPLETE: %d/%d PASSED\n",
			 passed, total);
	else
		dev_err(rpu->dev,
			"MBOX_SELFTEST COMPLETE: %d/%d PASSED *** %d FAILED ***\n",
			passed, total, total - passed);

	return (passed == total) ? 0 : -EIO;
}

#endif /* MBOX_ENABLE_SELFTEST */

MODULE_AUTHOR("AMD ISP");
MODULE_DESCRIPTION("MBOX_SELFTEST APU driver");
MODULE_LICENSE("GPL");
