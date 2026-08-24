// SPDX-License-Identifier: MIT
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include "mbox_cmd.h"
#include "mbox_crc.h"
#include "mbox_seq.h"
#include "mbox_api.h"
#include "mbox_error_code.h"
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stddef.h>

#include "sensor_cmd.h"
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/string.h>

#include "visp_driver.h"
#include "visp_mbox_driver.h"
#ifdef MBOX_ENABLE_SELFTEST
#include "mbox_selftest.h"
#endif

#define XPAR_PS_0_PSPMC_0_PSV_IPI_0_BIT_MASK 0x00000004U
#define XPAR_PS_0_PSPMC_0_PSV_IPI_1_BIT_MASK 0x00000008U

/* Default IPI SMC function IDs */
#define SMC_IPI_MAILBOX_OPEN 0x82001000U
#define SMC_IPI_MAILBOX_RELEASE 0x82001001U
#define SMC_IPI_MAILBOX_STATUS_ENQUIRY 0x82001002U
#define SMC_IPI_MAILBOX_NOTIFY 0x82001003U
#define SMC_IPI_MAILBOX_ACK 0x82001004U
#define SMC_IPI_MAILBOX_ENABLE_IRQ 0x82001005U
#define SMC_IPI_MAILBOX_DISABLE_IRQ 0x82001006U

#define DEBUG_ENABLE_LOG

struct response_packet {
	u32 cmdid;
	u32 cookie;
	u32 error_subcode;
};

static bool visp_mbox_integrity_enable = true;
module_param_named(mbox_integrity, visp_mbox_integrity_enable, bool, 0644);
MODULE_PARM_DESC(mbox_integrity,
		 "Enable mailbox CRC-16 and sequence-counter integrity checks (default: enabled)");

bool visp_mbox_integrity_active(void)
{
	return visp_mbox_integrity_enable;
}

/*
 * Checksum covers the header + seq_counter + payload, but everything before
 * seq_counter (media_server_flags/checksum/timestamps/msg_id/size/type/
 * cookie/payload_size) is already final at this point, so that portion can
 * be digested outside write_lock. visp_mbox_prepare_integrity() below only
 * has to stamp seq_counter and finish the CRC over the remainder. Note: the
 * remainder still includes the actual payload bytes (up to MAX_PAYLOAD_SIZE),
 * so this doesn't fully bound write_lock hold time for large commands - see
 * review notes; fixing that further would require a payload_packet
 * wire-format change coordinated with RPU firmware, which is out of scope
 * here since firmware currently expects this field order.
 */
static size_t visp_mbox_integrity_prefix_len(void)
{
	return offsetof(mbox_post_msg, payload) +
	       offsetof(payload_packet, seq_counter);
}

static u16 visp_mbox_prepare_integrity_prefix(const mbox_post_msg *msg)
{
	if (!visp_mbox_integrity_enable)
		return 0;

	return visp_mbox_crc16(msg, visp_mbox_integrity_prefix_len());
}

static void visp_mbox_prepare_integrity(struct rpu_dev *rpu, mbox_post_msg *msg,
					u16 prefix_crc)
{
	size_t prefix_len, total_len;
	payload_packet *pkt;

	if (!visp_mbox_integrity_enable)
		return;

	pkt = (payload_packet *)msg->payload;
	pkt->seq_counter = rpu->outbound_seq;

	prefix_len = visp_mbox_integrity_prefix_len();
	total_len = visp_mbox_checksum_size(msg);
	msg->checksum = visp_mbox_crc16_update((const u8 *)msg + prefix_len,
						total_len - prefix_len, prefix_crc);
}

static int visp_mbox_validate_integrity(struct rpu_dev *rpu, mbox_post_msg *msg)
{
	u32 received_checksum;
	u16 calculated_checksum;
	payload_packet *pkt;

	if (!visp_mbox_integrity_enable)
		return VPI_SUCCESS;

	if (!msg || msg->size > MAX_PAYLOAD_SIZE) {
		dev_err(rpu->dev,
			"Mailbox invalid payload size from RPU%d: msg_id=%u size=%u max=%u\n",
			rpu->rpu_id, msg ? msg->msg_id : 0, msg ? msg->size : 0,
			MAX_PAYLOAD_SIZE);
		return VPI_ERR_INVALID;
	}

	received_checksum = msg->checksum;
	calculated_checksum = visp_mbox_calculate_checksum(msg);
	if (received_checksum != calculated_checksum) {
		dev_err(rpu->dev,
			"Mailbox checksum mismatch from RPU%d: expected 0x%04x got 0x%04x for msg_id=%u crc_len=%u\n",
			rpu->rpu_id, calculated_checksum, received_checksum,
			msg->msg_id, (u32)visp_mbox_checksum_size(msg));
		return VPI_ERR_CHECKSUM;
	}

	pkt = (payload_packet *)msg->payload;
	if (msg->msg_id == MB_CMD_RES_SUCCESS &&
	    pkt->resp_field.processed_cmdid == APU_2_RPU_MB_CMB_INIT_FIRMWARE)
		visp_mbox_mark_seq_resync(rpu);

	return visp_mbox_validate_seq(rpu, msg);
}

uint32_t write_mboxcmd(uint32_t cmd_id, void *struct_msg, uint16_t size,
		       uint32_t flag, mbox_core_id receiver_id,
		       mbox_core_id core_id)
{
	int ret;
	mbox_post_msg *msg = NULL;
	struct rpu_dev *rpu;
	u16 integrity_prefix_crc = 0;

	rpu = visp_mbox_get_rpu_dev(receiver_id + VISP_MBOX_RPU6);
	if (!rpu)
		return -EINVAL;

	/* Get a buffer from the pre-allocated TX pool */
	msg = visp_get_tx_buffer(rpu);
	if (!msg)
		return -ENOMEM;

	memset(msg, 0, sizeof(*msg));
	msg->msg_id = cmd_id;
	msg->media_server_flags = flag;
	visp_mbox_set_message_size(msg, (payload_packet *)struct_msg);
	if (msg->size > MAX_PAYLOAD_SIZE) {
		dev_err(rpu->dev,
			"%s: Message size %u exceeds maximum payload size %u\n",
			__func__, msg->size, MAX_PAYLOAD_SIZE);
		visp_free_tx_buffer(rpu, msg);
		return -EINVAL;
	}
	memcpy(msg->payload, struct_msg, ALIGN(msg->size, 8));

	if (core_id != MBOX_CORE_APU)
		core_id = MBOX_CORE_APU;

	integrity_prefix_crc = visp_mbox_prepare_integrity_prefix(msg);

	/*
	 * LOCK SCOPE: stamp the sequence/checksum and enqueue atomically.
	 * The sequence number is only committed (advanced) on a successful
	 * enqueue so a dropped (FIFO-full) message does not leave a hole that
	 * permanently desyncs the RPU's strict in-order validation.
	 */
	mutex_lock(&rpu->write_lock);

	visp_mbox_prepare_integrity(rpu, msg, integrity_prefix_crc);

	ret = vpi_mbox_post(rpu->apu_tx_ctrl, msg, receiver_id, NULL);
	if (ret == VPI_SUCCESS && visp_mbox_integrity_enable)
		visp_mbox_commit_outbound_seq(rpu);
	mutex_unlock(&rpu->write_lock);

	/* Return buffer to pool after hardware copies it */
	visp_free_tx_buffer(rpu, msg);

	if (ret == VPI_ERR_FULL) {
		dev_err_ratelimited(rpu->dev,
			"%s: TX FIFO FULL (RPU%d) - RPU can't drain enqueue commands fast enough. "
			"This WILL cause frame drops. Check RPU processing load.\n",
			__func__, rpu->rpu_id);
		return -EAGAIN;
	}
	if (ret)
		return ret;

	return VPI_SUCCESS;
}
EXPORT_SYMBOL_GPL(write_mboxcmd);

int visp_mbox_apu_read(struct rpu_dev *rpu)
{
	struct visp_dev *isp_dev;
	uint32_t cmd, isp_id, instance_id, path, port;
	int ret = 0;
	mbox_post_msg *msg_copy;

	/* Get a buffer from the pre-allocated RX pool */
	msg_copy = visp_get_rx_buffer(rpu);
	if (!msg_copy) {
		pr_err("%s: Failed to get RX buffer\n", __func__);
		return -ENOMEM;
	}

	mutex_lock(&rpu->read_lock);
	ret = vpi_mbox_read(rpu->apu_rx_ctrl, msg_copy, rpu->core_id);
	mutex_unlock(&rpu->read_lock);
	/* Check if read was successful */
	if (ret != VPI_SUCCESS) {
		if (ret == VPI_ERR_EMPTY) {
			/* FIFO empty - race condition between check and read */
			visp_free_rx_buffer(rpu, msg_copy);
			return 0; /* Not an error, just empty */
		}
		/* Other read errors */
		pr_err("%s: vpi_mbox_read failed with error: %d\n", __func__,
		       ret);
		visp_free_rx_buffer(rpu, msg_copy);
		return ret;
	}

#ifdef MBOX_ENABLE_SELFTEST
	/*
	 * Self-test frames carry crafted seq/cookie and no ISP context, so hand
	 * them to the harness before production integrity/seq validation and
	 * ISP routing would reject them.
	 */
	if (visp_mbox_selftest_active()) {
		uint32_t st_id = msg_copy->msg_id;

		if (st_id == APU_2_RPU_MB_CMD_MBOX_SELFTEST_BEGIN ||
		    st_id == APU_2_RPU_MB_CMD_MBOX_SELFTEST ||
		    st_id == APU_2_RPU_MB_CMD_MBOX_SELFTEST_END ||
		    st_id == RPU_2_APU_MB_CMD_MBOX_SELFTEST_RESULT) {
			visp_mbox_selftest_rx_response(msg_copy);
			visp_free_rx_buffer(rpu, msg_copy);
			return 0;
		}
	}
#endif

	ret = visp_mbox_validate_integrity(rpu, msg_copy);
	if (ret != VPI_SUCCESS) {
		visp_free_rx_buffer(rpu, msg_copy);
		return ret;
	}

	/* Extract fields from the copy */
	cmd = msg_copy->msg_id;
	payload_packet *pkt = (payload_packet *)msg_copy->payload;
	uint8_t *payload_ptr = pkt->payload;
	mb_cmd_id_e resp_cmd = pkt->resp_field.processed_cmdid;

	/* INIT_FIRMWARE response has no ISP context; handle it here */
	if (cmd == MB_CMD_RES_SUCCESS &&
	    resp_cmd == APU_2_RPU_MB_CMB_INIT_FIRMWARE) {
		rpu->init_fw_status = pkt->resp_field.error_subcode_t;
		dev_info_ratelimited(rpu->dev,
			"INIT_FIRMWARE resp: cmd=%u status=%u\n",
			resp_cmd, rpu->init_fw_status);
		if (rpu->init_fw_status == 0)
			complete(&rpu->init_fw_done);
		visp_free_rx_buffer(rpu, msg_copy);
		return 0;
	}

	memcpy(&instance_id, payload_ptr, sizeof(uint32_t));
	payload_ptr += sizeof(uint32_t);
	memcpy(&path, payload_ptr, sizeof(uint32_t));

	if (instance_id >> 8)
		dev_warn_ratelimited(rpu->dev,
				     "%s: instance_id upper bits set 0x%08x\n",
			__func__, instance_id);

	/* Extract instance_id from bits 0-7 */
	instance_id = instance_id & 0xFF;

	/*
	 * Calculate ISP device ID from instance_id:
	 * INTENDED design (16 instances per ISP):
	 *   ISP_DEV 0: instance_id  0-15  (using isp_id = instance_id / 16)
	 *   ISP_DEV 1: instance_id 16-31
	 *   ISP_DEV 2: instance_id 32-47
	 *   ISP_DEV 3: instance_id 48-63
	 *   ISP_DEV 4: instance_id 64-79
	 *   ISP_DEV 5: instance_id 80-95
	 */
	isp_id = visp_mbox_isp_slot(instance_id);

	/* Validate ISP instance ID range */
	if (isp_id < 0 || isp_id >= MAX_ISP_INSTANCES) {
		dev_err(rpu->dev,
			"%s: FATAL - Invalid isp_id %u out of range [0-%d] "
			"(instance_id=%u, rpu_id=%d). "
			"Firmware or memory corruption.\n",
			__func__, isp_id, MAX_ISP_INSTANCES - 1,
			instance_id, rpu->rpu_id);
		visp_free_rx_buffer(rpu, msg_copy);
		return -EINVAL;
	}

	isp_dev = rpu->isp_dev[isp_id];
	if (!isp_dev) {
		dev_err(rpu->dev,
			"%s: FATAL - ISP%d device not registered "
			"(instance_id=%u, cmd=%d, rpu_id=%d)\n"
			"Ensure all ISP modules loaded before "
			"streaming. Rejecting message.\n",
			__func__, isp_id, instance_id, cmd, rpu->rpu_id);
		visp_free_rx_buffer(rpu, msg_copy);
		/* Return immediately - no further processing */
		return -ENODEV;
	}

	/*
	 * Map instance_id to port (0-3) for array indexing.
	 * Each ISP has 4 ports represnt 4 instances per port
	 */
	port = isp_dev->instanceid_port_map[instance_id % INSTANCES_PER_ISP];

	/* Guard against ports beyond active streams for this ISP */
	if (port >= isp_dev->num_streams) {
		dev_err(rpu->dev,
			"ACK for inactive port %u (instance_id=%u, num_streams=%u)\n",
			port, instance_id, isp_dev->num_streams);
		visp_free_rx_buffer(rpu, msg_copy);
		return -EINVAL;
	}

	if (msg_copy->media_server_flags == 1) {
		if (kfifo_is_full(&rpu->app_fifo)) {
			dev_err(rpu->dev,
				"app_fifo is FULL (%u/%u), dropping message"
				" cmd=%u (RPU%d)! Userspace not reading fast"
				" enough!\n",
				kfifo_len(&rpu->app_fifo), RPU_CMD_KFIFO_SIZE,
				cmd, rpu->rpu_id);
			ret = -ENOSPC;
			visp_free_rx_buffer(rpu, msg_copy);
			goto ERROR;
		}
		mutex_lock(&rpu->app_fifo_lock);
		if (!kfifo_in(&rpu->app_fifo, &msg_copy, 1)) {
			mutex_unlock(&rpu->app_fifo_lock);
			dev_err(rpu->dev,
				"Failed to queue into app_fifo\n");
			ret = -ENOMEM;
			visp_free_rx_buffer(rpu, msg_copy);
			goto ERROR;
		}
		mutex_unlock(&rpu->app_fifo_lock);
		/* Wake up any blocking readers (read/poll/select/epoll) */
		wake_up_interruptible(&rpu->read_wait);
		goto DONE;
	} else {
		/* Handle command responses and display buffer messages */
		if (cmd == MB_CMD_RES_SUCCESS) {
			if (port >= 4) {
				dev_err(rpu->dev,
					"FATAL - Invalid port %u (max 3) "
					"instance_id=%u. Memory corruption\n",
					port, instance_id);
				visp_free_rx_buffer(rpu, msg_copy);
				return -EINVAL;
			}
			/* Extract command type for routing decisions */
			/* Debug logging and validation for ENQUE_BUFFER commands */
			if (resp_cmd == APU_2_RPU_MB_CMD_ENQUE_BUFFER) {
				uint32_t buffer_index = 0;
				uint8_t *payload_ptr;

				/* Extract buffer_index from payload */
				payload_ptr = ((payload_packet *)
					msg_copy->payload)->payload;

				payload_ptr += sizeof(uint32_t);
				payload_ptr += sizeof(uint32_t);
				memcpy(&buffer_index, payload_ptr,
				       sizeof(uint8_t));
				/* Validate buffer_index */
				if (buffer_index >= MEDIA_ISP_BUF_FRAME_MAX) {
					dev_err(rpu->dev,
						"FATAL-ENQUE_BUFFER ACK:invalid"
						" buffer_index=%u "
						"for port=%u path=%u (max 31)."
						" Array bounds violation!\n",
						buffer_index, port, path);
					visp_free_rx_buffer(rpu, msg_copy);
					return -EINVAL;
				}
				/* MIMO mode: Firmware reports path=6, but
				 * arrays only have 4 paths (0,1,2,3)
				 */
				if (path == 6 && isp_dev->ss_mode_i0 &&
				    strcmp(isp_dev->ss_mode_i0, "mimo") == 0) {
					path = 3; /* Map path 6 to 3 for array indexing */
				}
				/*
				 * Validate path only after the MIMO path=6
				 * remap above, so a legitimate MIMO ACK still
				 * passes.  Anything left out of range would
				 * index the [MEDIA_ISP_CHN_MAX] arrays below
				 * out of bounds -- including a write via
				 * enq_ack_pending[] and enq_ack_error[] -- so
				 * reject it the same way buffer_index is.
				 */
				if (path >= MEDIA_ISP_CHN_MAX) {
					dev_err(rpu->dev,
						"FATAL-ENQUE_BUFFER ACK:invalid"
						" path=%u for port=%u (max %d)."
						" Array bounds violation!\n",
						path, port,
						MEDIA_ISP_CHN_MAX - 1);
					visp_free_rx_buffer(rpu, msg_copy);
					return -EINVAL;
				}
				/*
				 * Correlate the ENQ ACK to the queued buffer by
				 * echoed cookie and active-waiter flag (contract
				 * §6): a missing waiter or mismatched cookie is a
				 * stale/duplicate ACK, drop it without signaling.
				 */
				if (visp_mbox_integrity_active()) {
					u32 enq_exp = READ_ONCE(rpu->enq_ack_cookie
						[isp_id][port][path][buffer_index]);

					if (!READ_ONCE(rpu->enq_ack_pending[isp_id]
					    [port][path][buffer_index]) ||
					    pkt->cookie != enq_exp) {
						dev_warn_ratelimited(rpu->dev,
								     "Dropping stale/duplicate ENQ ACK (isp=%u port=%u path=%u buf=%u exp=0x%08x got=0x%08x)\n",
								     isp_id, port,
								     path, buffer_index,
								     enq_exp, pkt->cookie);
						visp_free_rx_buffer(rpu, msg_copy);
						goto DONE;
					}
					WRITE_ONCE(rpu->enq_ack_pending[isp_id]
						   [port][path][buffer_index], false);
				}
				/* Extract error code from ACK and store before signaling */
				int error_code = pkt->resp_field.error_subcode_t;
				/*
				 * If stream-off already set -ESHUTDOWN and fired
				 * complete_all(), this is a late ACK from the
				 * previous session.  Do not overwrite the shutdown
				 * marker and do not call complete() again — it
				 * would spuriously unblock the next session's ENQ
				 * wait for the same buffer slot.
				 */
				if (isp_dev->enq_ack_error[port][path][buffer_index]
				    != -ESHUTDOWN) {
					/* Active path: store error and signal completion */
					isp_dev->enq_ack_error[port][path]
						[buffer_index] = error_code;
					complete(&isp_dev->apu_wait_for_enq_ack
						[port][path][buffer_index]);
				} else {
					/* Stream closed: late ACK dropped silently */
					dev_dbg(rpu->dev,
						"Late ENQ ACK dropped (stream closed): "
						"instance_id=%u port=%u path=%u buf=%u\n",
						instance_id, port, path, buffer_index);
				}
				/* Free message immediately - no FIFO needed */
				visp_free_rx_buffer(rpu, msg_copy);
			} else {
				/*
				 * Correlate the waited ACK to the active waiter by
				 * echoed cookie (contract §6). A missing waiter or
				 * mismatched cookie means a stale/duplicate/orphan
				 * response: log and drop, never wake a waiter.
				 */
				if (visp_mbox_integrity_active()) {
					u32 exp = READ_ONCE(rpu->cmd_ack_cookie
							    [isp_id][port]);

					if (!READ_ONCE(rpu->cmd_ack_pending
					    [isp_id][port]) ||
					    pkt->cookie != exp) {
						dev_warn_ratelimited(rpu->dev,
								     "Dropping stale/duplicate cmd ACK (isp=%u port=%u exp=0x%08x got=0x%08x cmd=%u)\n",
								     isp_id, port, exp,
								     pkt->cookie, resp_cmd);
						visp_free_rx_buffer(rpu, msg_copy);
						goto DONE;
					}
					WRITE_ONCE(rpu->cmd_ack_pending[isp_id]
						   [port], false);
				}
				if (kfifo_is_full(&isp_dev->cmd_ack_fifo[port])) {
					dev_err_ratelimited(rpu->dev,
						"cmd_ack_fifo[%u] FULL,"
						" dropping ACK for "
						"instance_id=%u - command"
						" responses not consumed\n",
						port, instance_id);
					ret = -ENOSPC;
				visp_free_rx_buffer(rpu, msg_copy);
					goto ERROR;
				}
				mutex_lock(&isp_dev->cmd_ack_fifo_lock[port]);
				if (!kfifo_in(&isp_dev->cmd_ack_fifo[port],
					      &msg_copy, 1)) {
					mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);
					dev_err(rpu->dev,
						"Failed to queue into cmd_ack_fifo"
						"[%u] (race condition)\n", port);
					ret = -ENOMEM;
					visp_free_rx_buffer(rpu, msg_copy);
					goto ERROR;
				}
				mutex_unlock(&isp_dev->cmd_ack_fifo_lock[port]);
				complete(&isp_dev->apu_wait_for_cmd_ack[port]);
			}
			goto DONE;
		}

		/* Handle data response - route to per-port FIFO/completion */
		if (cmd == MB_CMD_GET_SUCCESS) {
			/*
			 * Correlate the data response to the active waiter by
			 * echoed cookie (contract §6); drop stale/orphan ones.
			 */
			if (visp_mbox_integrity_active()) {
				u32 exp = READ_ONCE(rpu->data_cookie[isp_id][port]);

				if (!READ_ONCE(rpu->data_pending[isp_id][port]) ||
				    pkt->cookie != exp) {
					dev_warn_ratelimited(rpu->dev,
							     "Dropping stale/duplicate data resp (isp=%u port=%u exp=0x%08x got=0x%08x)\n",
							     isp_id, port, exp, pkt->cookie);
					visp_free_rx_buffer(rpu, msg_copy);
					goto DONE;
				}
				WRITE_ONCE(rpu->data_pending[isp_id][port], false);
			}
			mutex_lock(&isp_dev->data_fifo_lock[port]);
			if (!kfifo_in(&isp_dev->data_fifo[port], &msg_copy, 1)) {
				mutex_unlock(&isp_dev->data_fifo_lock[port]);
				dev_err(rpu->dev,
					"Failed to queue into data_fifo[%u]\n",
					port);
				ret = -ENOMEM;
				visp_free_rx_buffer(rpu, msg_copy);
				goto ERROR;
			}
			mutex_unlock(&isp_dev->data_fifo_lock[port]);
			complete(&isp_dev->apu_wait_for_data[port]);
			goto DONE;
		}

		/* Handle display buffer notification from RPU */
		if (cmd == RPU_2_APU_CMD_DISPLAY_BUFFER) {
			if (!isp_dev->frameout_cb) {
				dev_err(rpu->dev, "CALLBACK IS NULL\n");
				ret = -EINVAL;
				visp_free_rx_buffer(rpu, msg_copy);
				goto ERROR;
			}

			/*
			 * Direct callback - no FIFO overhead.
			 * Frameout processing happens synchronously in mailbox context.
			 * Message ownership transferred to callback, which must free it.
			 */
			ret = isp_dev->frameout_cb(isp_dev, port, msg_copy);
			if (ret != 0) {
				dev_err(rpu->dev,
					"Frameout callback failed for port %u: %d\n",
					port, ret);
				/* Callback should have freed message on error */
			}
			goto DONE;
		}

		/* Unexpected command (media_server_flags != 1) */
		dev_err(rpu->dev,
			"%s: FATAL - Unexpected/corrupted command id %d "
			"received (instance_id=%u, isp_id=%d, flags=%u). "
			"Firmware mismatch or memory corruption.\n",
			__func__, cmd, instance_id, isp_id,
			msg_copy->media_server_flags);
		visp_free_rx_buffer(rpu, msg_copy);
		/* Return immediately - corrupted or unsupported command */
		return -EINVAL;
	}

ERROR:
DONE:
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_apu_read);

int visp_mbox_mailbox_init(struct rpu_dev *rpu, u32 cpu,
			   uint64_t MBOX_FIFO_START_ADDR,
			   uint64_t mbox_fifo_start_addr_phy)
{
	if (!rpu) {
		pr_err("%s: Invalid RPU device (NULL pointer).\n", __func__);
		return -EINVAL;
	}

	if (MBOX_FIFO_START_ADDR == 0) {
		pr_err("%s: Invalid virtual address (NULL).\n", __func__);
		return -EINVAL;
	}

	if (mbox_fifo_start_addr_phy == 0) {
		pr_err("%s: Invalid physical address (NULL).\n", __func__);
		return -EINVAL;
	}

	rpu->apu_rx_ctrl = visp_mbox_init(rpu->core_id,
					  MBOX_CORE_APU,
					  MBOX_FIFO_START_ADDR,
					  mbox_fifo_start_addr_phy,
					  MBOX_FIFO_BLOCK_SIZE);
	if (!rpu->apu_rx_ctrl) {
		pr_err("%s: Failed to initialize APU RX control.\n", __func__);
		return -ENOMEM;
	}

	rpu->apu_tx_ctrl = visp_mbox_init(MBOX_CORE_APU, rpu->core_id,
					  MBOX_FIFO_START_ADDR,
					  mbox_fifo_start_addr_phy,
					  MBOX_FIFO_BLOCK_SIZE);
	if (!rpu->apu_tx_ctrl) {
		pr_err("%s: Failed to initialize APU TX control.\n", __func__);
		kfree(rpu->apu_rx_ctrl);
		rpu->apu_rx_ctrl = NULL;
		return -ENOMEM;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(visp_mbox_mailbox_init);

void mailbox_close(struct rpu_dev *rpu)
{
	kfree(rpu->apu_tx_ctrl);
	kfree(rpu->apu_rx_ctrl);
}

int visp_mbox_send_command(mb_cmd_id_e cmd, void *data, uint32_t size,
			   uint32_t flag, uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;

	ret = write_mboxcmd(cmd, data, size, flag, dest_cpu, src_cpu);
	return ret;
}
EXPORT_SYMBOL_GPL(visp_mbox_send_command);

MODULE_AUTHOR("anandam");
MODULE_DESCRIPTION("MBOX_CMD");
MODULE_LICENSE("GPL");
