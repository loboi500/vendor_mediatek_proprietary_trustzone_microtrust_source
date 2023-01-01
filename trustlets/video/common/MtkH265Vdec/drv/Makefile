#
# Copyright (c) 2015 MediaTek Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mt6781))
USED_MTK_PLATFORM := MT6853
else
USED_MTK_PLATFORM := $(MTK_PLATFORM)
endif

ifeq (yes,$(strip $(MTK_SECURE_HEVC_VIDEO_SUPPORT_TABLET)))
USED_PLATFORM := $(filter $(USED_MTK_PLATFORM),MT6785)
else
USED_PLATFORM := MT8185
endif

ifeq ($(USED_MTK_PLATFORM),$(filter $(USED_MTK_PLATFORM),MT6885 MT6873 MT6853 $(USED_PLATFORM)))

TARGET = 4be4f7dc1f2c11e5b5f7727283247c7f.ta

#STRIPPED=disabled

SRC_C += manifest.c

EXTRA_LIBS += $(MSEE_FWK_DRV_OUT_DIR)/$(TEE_MODE)/msee_fwk_drv.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/HEVCSecureVdecDriver_intermediates/HEVCSecureVdecDriver.lib

MY_CFLAGS = $(CFLAGS_SYS_IRQ) $(CFLAGS_SYS_IO) $(CFLAGS_PF_UT_DRV)

MY_LDFLAGS = $(LDFLAGS_SYS_IRQ) $(LDFLAGS_SYS_IO) $(LDFLAGS_PF_UT_DRV)

MODE = shared

include $(UT_SDK_HOME)/build/prog.mk

endif
