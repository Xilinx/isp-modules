/* SPDX-License-Identifier: MIT */
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

#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/device/driver.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/workqueue.h>

#include <linux/firmware/amd-versal2-error-events.h>
#include <linux/firmware/xlnx-event-manager.h>
#include <linux/firmware/xlnx-zynqmp.h>

#include "mbox_cmd.h"
#include "visp_mbox_wdt.h"

/*
 * Runtime toggle controlled by visp_mbox sysfs.
 * 0: notifier unregistered; 1: notifier registered and active.
 */
static bool wdt_selfheal_enable;
static bool wdt_notifier_registered;
static DEFINE_MUTEX(wdt_ctrl_lock);

/*
 * node = VERSAL2_EVENT_ERROR_LPDSLCR_ERR3 (0x28114000)
 * event = LPX_WWDT0..WWDT3 (BIT(24)..BIT(27)), wired to RPU cores 6..9 on
 * this platform (WWDT4 is not used for RPU core watchdog notification).
 * Registering the whole group lets the callback identify which RPU
 * core's watchdog actually expired instead of assuming a single core.
 * Matches the node/event pair already proven on bare-metal APU
 * (XilinxProcessorIPLib/drivers/visp_ss/src/wdt_notifier.c).
 */
#define VISP_MBOX_WDT_NODE_ID  VERSAL2_EVENT_ERROR_LPDSLCR_ERR3
#define VISP_MBOX_WDT_EVENT_MASK \
	(XPM_VERSAL2_EVENT_ERROR_MASK_LPX_WWDT0 | \
	 XPM_VERSAL2_EVENT_ERROR_MASK_LPX_WWDT1 | \
	 XPM_VERSAL2_EVENT_ERROR_MASK_LPX_WWDT2 | \
	 XPM_VERSAL2_EVENT_ERROR_MASK_LPX_WWDT3)

/* Ordinal WWDTn -> RPU core index, per platform's fixed lpd_wwdtN wiring. */
static const int visp_mbox_wdt_rpu_core[] = { 6, 7, 8, 9 };

/* Guards against duplicate PM_NOTIFY_CB deliveries for the same event,
 * one flag per RPU core so one core's in-flight recovery never blocks a
 * genuine expiry on a different core.
 */
static atomic_t wdt_event_handled[ARRAY_SIZE(visp_mbox_wdt_rpu_core)];

struct visp_mbox_wdt_expiry_work {
	struct work_struct work;
	int rpu_id;
	/* Detects a core whose WWDT keeps re-expiring right after recovery
	 * (RPU firmware's own hal_wdt_init self-test failing, not a one-off
	 * fault), so auto-recovery can be given up on instead of rebooting
	 * forever.
	 */
	ktime_t last_expiry;
	unsigned int rapid_count;
	bool given_up;
};

static struct visp_mbox_wdt_expiry_work
	visp_mbox_wdt_expiry_works[ARRAY_SIZE(visp_mbox_wdt_rpu_core)];

/* Re-expiring this soon after the previous recovery means the core is stuck
 * re-triggering on its own, not hitting a fresh one-off fault.
 */
#define VISP_MBOX_WDT_LOOP_WINDOW_MS 5000
#define VISP_MBOX_WDT_LOOP_LIMIT 1

/*
 * Runs in process context (unlike the PM_NOTIFY_CB callback), so it is safe
 * to walk isp_dev state and call into isp_dev->wdt_expiry_cb, which may
 * block while posting the event to userspace.
 *
 * Full recovery sequence for the RPU core that hung:
 *   1. For every isp_dev this RPU owns: force-kill its pipelines locally
 *      (no RPU/mailbox command - the RPU is dead) and notify the daemon,
 *      while the isp_dev pointer is still valid.
 *   2. device_release_driver() each of those isp_dev's platform devices -
 *      this runs visp_remove() synchronously and frees every devm-allocated
 *      resource of the driver instance (isp_dev included), so rpu->isp_dev[]
 *      is nulled right after. The underlying struct device is untouched
 *      (only unbound), so the pointer stays valid for the reattach below.
 *   3. Resync the RPU firmware handshake over mailbox.
 *   4. device_attach() each platform device, which reprobes and re-links
 *      rpu->isp_dev[] with a freshly initialized isp_dev.
 */
static void visp_mbox_wdt_notify_cb(const u32 *payload, void *data);
static void visp_mbox_wdt_expiry_worker(struct work_struct *work);

static int visp_mbox_wdt_register_locked(void)
{
	int i;
	int ret;

	if (wdt_notifier_registered)
		return 0;

	for (i = 0; i < ARRAY_SIZE(visp_mbox_wdt_expiry_works); i++) {
		atomic_set(&wdt_event_handled[i], 0);
		visp_mbox_wdt_expiry_works[i].rpu_id = visp_mbox_wdt_rpu_core[i];
		visp_mbox_wdt_expiry_works[i].last_expiry = 0;
		visp_mbox_wdt_expiry_works[i].rapid_count = 0;
		visp_mbox_wdt_expiry_works[i].given_up = false;
		INIT_WORK(&visp_mbox_wdt_expiry_works[i].work,
			  visp_mbox_wdt_expiry_worker);
	}

	ret = xlnx_register_event(PM_NOTIFY_CB, VISP_MBOX_WDT_NODE_ID,
				  VISP_MBOX_WDT_EVENT_MASK, false,
				  visp_mbox_wdt_notify_cb, NULL);
	if (ret) {
		pr_err("visp_mbox_wdt: xlnx_register_event failed for node 0x%x event 0x%x: %d\n",
		       (unsigned int)VISP_MBOX_WDT_NODE_ID,
		       (unsigned int)VISP_MBOX_WDT_EVENT_MASK, ret);
		return ret;
	}

	wdt_notifier_registered = true;
	pr_info("visp_mbox_wdt: registered for LPX_WWDT0..3 (node=0x%x event=0x%x)\n",
		(unsigned int)VISP_MBOX_WDT_NODE_ID,
		(unsigned int)VISP_MBOX_WDT_EVENT_MASK);

#ifndef VISP_WDT_SELFHEAL
	pr_info("visp_mbox_wdt: logging expiries only (build with VISP_WDT_SELFHEAL defined for full recovery)\n");
#endif

	return 0;
}

static void visp_mbox_wdt_unregister_locked(void)
{
	int ret;
	int i;

	if (!wdt_notifier_registered)
		return;

	for (i = 0; i < ARRAY_SIZE(visp_mbox_wdt_expiry_works); i++)
		cancel_work_sync(&visp_mbox_wdt_expiry_works[i].work);

	ret = xlnx_unregister_event(PM_NOTIFY_CB, VISP_MBOX_WDT_NODE_ID,
				    VISP_MBOX_WDT_EVENT_MASK,
				    visp_mbox_wdt_notify_cb, NULL);
	if (ret)
		pr_err("visp_mbox_wdt: xlnx_unregister_event failed: %d\n", ret);
	else
		pr_info("visp_mbox_wdt: unregistered LPX_WWDT0..3 notifier\n");

	wdt_notifier_registered = false;
}

bool visp_mbox_wdt_selfheal_get(void)
{
	bool enabled;

	mutex_lock(&wdt_ctrl_lock);
	enabled = wdt_selfheal_enable;
	mutex_unlock(&wdt_ctrl_lock);

	return enabled;
}

int visp_mbox_wdt_selfheal_set(bool enable)
{
	bool changed = false;
	int ret = 0;

	mutex_lock(&wdt_ctrl_lock);

	if (wdt_selfheal_enable == enable)
		goto out;

	if (enable) {
		wdt_selfheal_enable = true;
		ret = visp_mbox_wdt_register_locked();
		if (ret) {
			wdt_selfheal_enable = false;
			goto out;
		}
	} else {
		wdt_selfheal_enable = false;
		visp_mbox_wdt_unregister_locked();
	}
	changed = true;

out:
	mutex_unlock(&wdt_ctrl_lock);
	if (changed)
		pr_info("visp_mbox_wdt: wdt_selfheal %s via sysfs\n",
			enable ? "enabled" : "disabled");
	return ret;
}

static void visp_mbox_wdt_expiry_worker(struct work_struct *work)
{
	struct visp_mbox_wdt_expiry_work *ew =
		container_of(work, struct visp_mbox_wdt_expiry_work, work);
	struct rpu_dev *rpu;
	/* Only read in the VISP_WDT_SELFHEAL re-arm path below. */
	int __maybe_unused wwdt_n = ew - visp_mbox_wdt_expiry_works;
	ktime_t now = ktime_get();

	if (ew->last_expiry &&
	    ktime_ms_delta(now, ew->last_expiry) < VISP_MBOX_WDT_LOOP_WINDOW_MS)
		ew->rapid_count++;
	else
		ew->rapid_count = 0;
	ew->last_expiry = now;

	if (ew->rapid_count >= VISP_MBOX_WDT_LOOP_LIMIT) {
		if (!ew->given_up) {
			ew->given_up = true;
			pr_err("visp_mbox_wdt: RPU core %d re-expired %u times within %dms of the previous recovery -- likely the RPU firmware's own hal_wdt_init self-test failing, not a one-off fault. Giving up auto-recovery for this core; it is left powered down.\n",
			       ew->rpu_id, ew->rapid_count + 1,
			       VISP_MBOX_WDT_LOOP_WINDOW_MS);
		}
		/*
		 * Keep this core software-disarmed (wdt_event_handled[wwdt_n] stays 1).
		 * Do not churn xlnx_(un)register_event() from this worker: unregistering
		 * while the event-manager callback path is active can race and crash in
		 * xlnx_call_notify_cb_handler().
		 */
		return;
	}

	rpu = visp_mbox_get_rpu_dev(ew->rpu_id);
	if (!rpu) {
		pr_warn("visp_mbox_wdt: no rpu_dev for expired RPU core %d\n",
			ew->rpu_id);
		return;
	}

	/*
	 * Disarm now so the INIT_FIRMWARE resync below (VISP_WDT_SELFHEAL
	 * builds only) doesn't immediately re-send WDT_TEST and loop forever
	 * on this expiry.
	 */
	rpu->wdt_test_enabled = false;

	/*
	 * Local teardown + userspace notify is pure APU-side bookkeeping with
	 * no dependency on RPU/firmware capability, so it must NOT be gated
	 * by VISP_WDT_SELFHEAL - only the firmware-dependent unbind/rproc-
	 * restart/reprobe sequence below stays behind that flag.
	 */
	{
#ifdef VISP_WDT_SELFHEAL
		struct visp_dev *isps[MAX_NO_ISP] = { NULL };
		struct device *devs[MAX_NO_ISP] = { NULL };
		bool recover[MAX_NO_ISP] = { false };
#endif
		int i, pad, active;

		for (i = 0; i < MAX_NO_ISP; i++) {
			struct visp_dev *isp_dev = rpu->isp_dev[i];

			if (!isp_dev)
				continue;

			active = 0;
			for (pad = 0; pad < VISP_PORT_PAD_NR * MAX_PORTS; pad++)
				if (isp_dev->streamon[pad])
					active++;

			pr_info("visp_mbox_wdt: isp_dev %d on RPU core %d: %d/%d streams active, forcing cleanup\n",
				isp_dev->id, ew->rpu_id, active,
				VISP_PORT_PAD_NR * MAX_PORTS);

			/*
			 * Close admission and release mailbox waiters before invoking
			 * any mode-specific callback. The callback may take a port lock;
			 * doing this afterward would force it to race or deadlock.
			 */
			visp_wdt_begin_teardown(isp_dev);

			if (isp_dev->wdt_expiry_cb)
				isp_dev->wdt_expiry_cb(isp_dev, active);

#ifdef VISP_WDT_SELFHEAL
			isps[i] = isp_dev;
			devs[i] = isp_dev->dev;
			recover[i] = true;
#endif
		}

#ifdef VISP_WDT_SELFHEAL
		/*
		 * Full recovery: unbind, restart the RPU, resync the mailbox
		 * handshake, and reprobe. Only meaningful once the local
		 * teardown above has already made every isp_dev's driver-side
		 * state safe to unbind.
		 */
		for (i = 0; i < MAX_NO_ISP; i++) {
			int port;

			if (!recover[i])
				continue;

			for (port = 0; port < MAX_PORTS; port++) {
				mutex_lock(&isps[i]->port_lock[port]);
				mutex_unlock(&isps[i]->port_lock[port]);
			}

			device_release_driver(devs[i]);

			/* isp_dev was devm-freed by .remove() above; drop the stale pointer. */
			rpu->isp_dev[i] = NULL;
		}

		if (visp_mbox_rpu_restart(rpu, isps, MAX_NO_ISP))
			pr_err("visp_mbox_wdt: remoteproc restart failed for RPU core %d\n",
			       ew->rpu_id);

		visp_mbox_clear_init_firmware_success(rpu);
		if (visp_mbox_send_init_firmware(rpu)) {
			pr_err("visp_mbox_wdt: INIT_FIRMWARE resync failed for RPU core %d - leaving devices unbound rather than exposing a non-functional device\n",
			       ew->rpu_id);
			goto rearm;
		}

		for (i = 0; i < MAX_NO_ISP; i++) {
			if (!recover[i])
				continue;

			if (device_attach(devs[i]) <= 0)
				pr_err("visp_mbox_wdt: reprobe failed for %s\n",
				       dev_name(devs[i]));
		}

		/*
		 * Self-heal actually reset the RPU (rproc restart + INIT_FIRMWARE),
		 * which physically clears the WWDT-expired condition, so it is now
		 * legitimate to re-arm and detect a fresh expiry for this core.
		 */
rearm:
		atomic_set(&wdt_event_handled[wwdt_n], 0);
#else
		/*
		 * Logging-only build: the RPU firmware does not implement a WWDT
		 * handler/reset of its own - a WWDT expiry is turned into a
		 * subsystem reset driven by the PLM. This build only logs the expiry
		 * and does not drive that reset, so from the driver's side nothing
		 * clears the physical WWDT-expired condition. Re-arming here would
		 * NOT add detection - PLM keeps re-delivering the same callback every
		 * ~10ms and the worker would re-run force-teardown + re-post
		 * VISP_EVENT_WDT_EXPIRY for an already-known-dead core (event storm,
		 * not new information). So re-arm is deliberately gated to
		 * VISP_WDT_SELFHEAL (the only path that actually resets the core):
		 * keep the per-core software gate closed (wdt_event_handled stays 1),
		 * report once, and stay disarmed. One-shot-per-core is intentional.
		 */
		/* ISP-REVIEW BLOCKER: default build only ever detects the FIRST
		 * WWDT expiry per RPU core, forever, until a manual wdt_selfheal
		 * sysfs toggle cycle.
		 * Why: this is unchanged from the immediately-prior reviewed
		 * round (still carried forward, not newly regressed here) - the
		 * reentrancy/crash-risk concern this round's comment raises
		 * against calling xlnx_(un)register_event() from this worker is
		 * a real improvement (removed that churn entirely), but it does
		 * not restore any form of repeat detection. A production core
		 * that keeps hanging after its first expiry is invisible to this
		 * driver from then on in the default (non-SELFHEAL) build.
		 * Suggested fix: reuse the existing rapid_count/given_up debounce
		 * already in this function to safely gate a *rate-limited* re-arm
		 * (e.g. re-register but immediately give up again if redelivered
		 * within the debounce window) instead of an unconditional
		 * one-shot-forever gate, or add an explicit software "clear latch"
		 * call to the PM/event-manager API (if one exists) that does not
		 * require a full RPU reset.
		 */
		pr_info("visp_mbox_wdt: RPU core %d WWDT expiry handled locally (pipelines torn down, daemon notified where wdt_expiry_cb is wired up); core is software-disarmed until self-heal is enabled/re-toggled\n",
			ew->rpu_id);
#endif /* VISP_WDT_SELFHEAL */
	}
}

static void visp_mbox_wdt_notify_cb(const u32 *payload, void *data)
{
	u32 cb_type = payload[0];
	u32 node_id = payload[1];
	u32 event = payload[2];
	u32 wdt_bits;
	int wwdt_n;
	bool any_new = false;

	if (cb_type != PM_NOTIFY_CB) {
		pr_warn("visp_mbox_wdt: unexpected callback type 0x%x\n", cb_type);
		return;
	}

	wdt_bits = event & VISP_MBOX_WDT_EVENT_MASK;
	if (node_id != VISP_MBOX_WDT_NODE_ID || !wdt_bits) {
		pr_warn("visp_mbox_wdt: unexpected node/event 0x%x/0x%x\n",
			node_id, event);
		return;
	}

	/*
	 * One-shot per core: the PLM keeps re-delivering this callback for
	 * the same WWDT condition every ~10ms until that RPU core is
	 * actually reset, so only log/process the first occurrence per core.
	 * The payload is logged inside the accepted-event branch (not here)
	 * so a still-asserted condition can't flood dmesg. Scoped per core
	 * (not global) so a different core's genuine expiry is never dropped
	 * just because another core's recovery is still in flight.
	 */
	for (wwdt_n = 0; wwdt_n < ARRAY_SIZE(visp_mbox_wdt_rpu_core); wwdt_n++) {
		if (!(wdt_bits & BIT(24 + wwdt_n)))
			continue;

		if (atomic_cmpxchg(&wdt_event_handled[wwdt_n], 0, 1) != 0)
			continue;

		pr_info("visp_mbox_wdt: PM_NOTIFY_CB payload [0]=0x%08x [1]=0x%08x [2]=0x%08x [3]=0x%08x\n",
			payload[0], payload[1], payload[2], payload[3]);
		pr_warn("visp_mbox_wdt: WDT expired! LPX_WWDT%d -> RPU core %d\n",
			wwdt_n, visp_mbox_wdt_rpu_core[wwdt_n]);

		schedule_work(&visp_mbox_wdt_expiry_works[wwdt_n].work);
		any_new = true;
	}

	if (any_new)
		pr_warn("visp_mbox_wdt: WDT expired! recovery worker queued\n");
}

int visp_mbox_wdt_init(void)
{
	pr_info("visp_mbox_wdt: WWDT notifier disabled by default; use sysfs wdt_selfheal=1 to enable\n");
	return 0;
}

void visp_mbox_wdt_exit(void)
{
	mutex_lock(&wdt_ctrl_lock);
	wdt_selfheal_enable = false;
	visp_mbox_wdt_unregister_locked();
	mutex_unlock(&wdt_ctrl_lock);
}

/* Seconds the RPU should keep petting its WWDT for after receiving WDT_TEST. */
#define VISP_MBOX_WDT_TEST_PET_SECONDS 60

int visp_mbox_wdt_send_test(struct rpu_dev *rpu)
{
	payload_packet *packet;
	uint32_t instance_id = 0;
	uint32_t pet_seconds = VISP_MBOX_WDT_TEST_PET_SECONDS;
	int ret;

	if (!rpu || !rpu->tx_chan)
		return -ENODEV;

	/* payload_packet is ~16KB - too large for the kernel stack. */
	packet = kzalloc(sizeof(*packet), GFP_KERNEL);
	if (!packet)
		return -ENOMEM;
	packet->type = CMD;
	/* Fire-and-forget: bypass RPU cookie dedup so it isn't replayed. */
	packet->cookie = MBOX_COOKIE_NOT_USED;

	/* Payload layout mirrors INIT_FIRMWARE: instance_id first, then the
	 * command-specific data (here, how many seconds to pet the WWDT for).
	 */
	memcpy(packet->payload, &instance_id, sizeof(instance_id));
	memcpy(packet->payload + sizeof(instance_id), &pet_seconds, sizeof(pet_seconds));
	packet->payload_size = sizeof(instance_id) + sizeof(pet_seconds);

	ret = visp_mbox_send_command(APU_2_RPU_MB_CMD_WDT_TEST, packet, payload_extra_size,
				     1, rpu->core_id, MBOX_CORE_APU);
	if (ret < 0) {
		pr_err("visp_mbox_wdt: failed to queue WDT test command for RPU core %d: %d\n",
		       rpu->core_id, ret);
		kfree(packet);
		return ret;
	}

	/* IPI trigger - mailbox framework handles synchronization. */
	ret = mbox_send_message(rpu->tx_chan, NULL);
	if (ret < 0) {
		pr_err("visp_mbox_wdt: failed to trigger IPI for WDT test command (RPU core %d): %d\n",
		       rpu->core_id, ret);
		kfree(packet);
		return ret;
	}

	pr_info("visp_mbox_wdt: WDT test command sent to RPU core %d (pet=%us)\n",
		rpu->rpu_id, pet_seconds);
	kfree(packet);
	return 0;
}
