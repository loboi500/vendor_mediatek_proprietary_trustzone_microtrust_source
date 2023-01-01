############MTK_TEE_RELEASE_BASIC#############
ifeq (yes,$(strip $(MTK_TEE_RELEASE_BASIC)))
# eg: $(1)--> sec:common:drv
define check_tee_release_module_path
p0 := $(shell echo $(1) |awk -F ':' '{print $$1}')
p1 := $(shell echo $(1) |awk -F ':' '{print $$2}')
p2 := $(shell echo $(1) |awk -F ':' '{print $$3}')
endef

$(foreach p,$(sort $(MTK_TEE_RELEASE_BASIC_MODULES)),\
  $(eval $(call check_tee_release_module_path,$(p)))\
  $(eval MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,$(p0),$(p1),$(p2)))\
)

MTK_TEE_RELEASE_CHECK_MODULES :=
ifneq (,$(filter yes,$(MTK_TEE_RELEASE_SVP) $(MTK_SEC_VIDEO_PATH_SUPPORT)))
$(foreach m,$(MTK_TEE_RELEASE_SVP_MODULES),$(eval MTK_TEE_RELEASE_CHECK_MODULES += $(m)))
endif
ifneq (,$(filter yes,$(MTK_TEE_RELEASE_SCAM) $(MTK_CAM_SECURITY_SUPPORT)))
$(foreach m,$(MTK_TEE_RELEASE_SCAM_MODULES),$(eval MTK_TEE_RELEASE_CHECK_MODULES += $(m)))
endif

$(foreach p,$(sort $(MTK_TEE_RELEASE_CHECK_MODULES)),\
  $(eval $(call check_tee_release_module_path,$(p)))\
  $(eval MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,$(p0),$(p1),$(p2)))\
)
endif

###############################################################################
# GENIEZONE
###############################################################################
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,pmem,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,pmem,common,ta)
ifeq ($(strip $(MTK_ENABLE_GENIEZONE)),yes)
ifeq ($(strip $(MTK_GZ_SUPPORT_SDSP)),yes)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,fod_sample,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,fod_sample,common,ta)
### M4U
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,drv)
MICROTRUST_ALL_MODULE_MAKEFILE += $(call mtk_microtrust_find_module_makefile,m4u,common,ta)
endif
endif
