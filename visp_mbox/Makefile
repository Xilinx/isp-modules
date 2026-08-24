TARGET := visp_mbox

obj-m := $(TARGET).o

$(TARGET)-objs += visp_mbox_driver.o
#$(TARGET)-objs += visp_event.o
#$(TARGET)-objs += visp_procfs.o
#$(TARGET)-objs += isp_ctrl/visp_ctrl.o
#$(TARGET)-objs += visp_app.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_cmd.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_crc.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_seq.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_api.o
$(TARGET)-objs += $(pwd)/mailbox/mbox_fifo.o
$(TARGET)-objs += visp_mbox_wdt.o

# Mailbox self-test: builds mbox_selftest.o and exposes the write-only node
# /proc/visp_mbox/selftest.  Off by default.  When enabled,
# visp_mbox_selftest_run() also runs during probe ahead of INIT_FIRMWARE and a
# failure aborts the bind, so the driver will not come up if the RPU does not
# answer the self-test frames.  Enable for a build with:
#   make MBOX_ENABLE_SELFTEST=1
MBOX_ENABLE_SELFTEST ?= 0

ifeq ($(MBOX_ENABLE_SELFTEST),1)
$(TARGET)-objs += $(pwd)/mailbox/mbox_selftest.o
endif

$(info PWD_PATH is $(PWD))

ccflags-y := -DISP_VERSION_MACRO=\"Version:M17\ Release:26.1.0\"
ccflags-y += -I$(PWD)
ccflags-y += -I$(PWD)/visp/
ccflags-y += -I$(PWD)/visp_common/include/
ccflags-y += -I$(PWD)/common/
ccflags-y += -I$(PWD)/visp_mbox/
ccflags-y += -I$(PWD)/visp_mbox/mailbox/
#ccflags-y += -I$(PWD)/../visp/mailbox/
#ccflags-y += -I$(PWD)/../vvcma_isp/isp_ctrl/ae/aev2/

ccflags-y += -I$(PWD)/visp_common/vsi_units/include/
ccflags-y += -I$(PWD)/visp_common/vsi_units/include/cam_device/
ccflags-y += -I$(PWD)/include/
#ccflags-y += -I$(PWD)/../visp/user_lib/source/vsi_units/include_priv/cam_device/
#ccflags-y += -I$(PWD)/../visp/user_lib/source/vsi_units/source/cam_device/

USER_INC:=$(PWD)/visp_common/isp_ctrl/visp_ctrl.h

ccflags-y += -DDEBUG -O2

ifeq ($(MBOX_ENABLE_SELFTEST),1)
ccflags-y += -DMBOX_ENABLE_SELFTEST
endif

# Define to enable full RPU WWDT self-heal (rebind + remoteproc restart) on
# expiry; otherwise the WWDT notifier only logs which RPU core expired.
#ccflags-y += -DVISP_WDT_SELFHEAL

PWD := $(shell pwd)
