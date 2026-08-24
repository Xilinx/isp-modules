/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MBOX_SELFTEST_TYPES_H_
#define _MBOX_SELFTEST_TYPES_H_

/* Shared by APU (Linux) and RPU (firmware) - do not renumber, values are on-wire */

#include <linux/types.h>

#define MBOX_ST_DESC_MAGIC		(0x53544D42u)	/* "STMB" */

typedef enum {
	MBOX_ST_EXPECT_ACCEPT	= 0,
	MBOX_ST_EXPECT_DROP	= 1,
	MBOX_ST_EXPECT_RESYNC	= 2,
	MBOX_ST_EXPECT_REPLAY	= 3,
	MBOX_ST_EXPECT_SUPPRESS	= 4,
} mbox_st_verdict_e;

#define MBOX_ST_PRE_RESET_SEQ		(1u << 0)
#define MBOX_ST_PRE_RESET_DEDUP		(1u << 1)

typedef enum {
	/* CRC-16 */
	MBOX_ST_CASE_CRC_VALID			= 0,
	MBOX_ST_CASE_CRC_CORRUPT_PAYLOAD	= 1,
	MBOX_ST_CASE_CRC_HIGH_BITS		= 2,	/* D1: RPU accepts, APU drops */
	MBOX_ST_CASE_CRC_ZERO			= 3,
	/* sequence counter */
	MBOX_ST_CASE_SEQ_IN_ORDER		= 10,
	MBOX_ST_CASE_SEQ_FORWARD_GAP		= 11,
	MBOX_ST_CASE_SEQ_GAP_OVER_CAP		= 12,	/* gap 9: both sides drop */
	MBOX_ST_CASE_SEQ_BACKWARD		= 13,
	MBOX_ST_CASE_SEQ_OUT_OF_RANGE		= 14,	/* D3: RPU casts signed, APU drops */
	MBOX_ST_CASE_SEQ_ROLLOVER		= 15,
	/* cookie de-duplication */
	MBOX_ST_CASE_COOKIE_NEW			= 20,
	MBOX_ST_CASE_COOKIE_REPLAY		= 21,
	MBOX_ST_CASE_COOKIE_SUPPRESS		= 22,
	MBOX_ST_CASE_COOKIE_NOT_USED		= 23,
	MBOX_ST_CASE_COOKIE_DUP_BEFORE_CACHE	= 24,
} mbox_st_case_e;

/* 16-byte descriptor at payload_data[0] (outer offset 48), always well-formed */
typedef struct {
	u32 magic;	/* MBOX_ST_DESC_MAGIC */
	u32 case_id;	/* mbox_st_case_e     */
	u32 expected;	/* mbox_st_verdict_e  */
	u32 pre_action;	/* MBOX_ST_PRE_* bits */
} mbox_selftest_desc_t;

#endif /* _MBOX_SELFTEST_TYPES_H_ */
