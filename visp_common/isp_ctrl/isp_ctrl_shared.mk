# Shared compiler-flag fragment for visp_common/isp_ctrl.
#
# Included identically by every module that compiles these sources directly
# (visp, visp_mimo, visp_mimo_video) so the compile command kbuild records
# for each shared object is byte-for-byte the same regardless of which
# module's build processes it first - letting all of them point straight at
# these canonical paths without per-module wrapper copies. Deliberately
# holds only include paths, no -D macros and no optimization/debug flags:
# those stay module-specific (see each consumer's own Makefile) and must
# never leak in here, or the shared objects stop being safely shareable.
VISP_COMMON_ISP_CTRL_INC := $(shell find $(VISP_PATH)/visp_common/isp_ctrl -type d 2>/dev/null)
ccflags-y += -I$(VISP_PATH)/visp_common/isp_ctrl/
ccflags-y += $(addprefix -I,$(VISP_COMMON_ISP_CTRL_INC))
