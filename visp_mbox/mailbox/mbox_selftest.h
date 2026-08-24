/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MBOX_SELFTEST_H_
#define _MBOX_SELFTEST_H_

#include "../visp_mbox_driver.h"
#include "mbox_fifo.h"

#ifdef MBOX_ENABLE_SELFTEST

int visp_mbox_selftest_run(struct rpu_dev *rpu, mbox_core_id dest);
void visp_mbox_selftest_rx_response(const mbox_post_msg *msg);
bool visp_mbox_selftest_active(void);

#else

static inline int visp_mbox_selftest_run(struct rpu_dev *rpu,
					 mbox_core_id dest)
{
	(void)rpu;
	(void)dest;
	return -ENODEV;
}

static inline void visp_mbox_selftest_rx_response(const mbox_post_msg *msg) {}
static inline bool visp_mbox_selftest_active(void) { return false; }

#endif /* MBOX_ENABLE_SELFTEST */

#endif /* _MBOX_SELFTEST_H_ */
