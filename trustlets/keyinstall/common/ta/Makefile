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

TARGET = 08110000000000000000000000000000.ta

MTK_RPMB_SUPPORT := no

# Secure memory
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib

HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drDRMHDCPCommon_api_intermediates/drDRMHDCPCommon_api.lib

HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drKeyInstall_api_intermediates/drKeyInstall_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/taKeyInstall_core_intermediates/taKeyInstall_core.lib

SRC_C += manifest.c

# For Ekkb_pub and Pkb customization: project > platform > common
# vendor/mediatek/proprietary/trustzone/common/hal/source/trustlets/keyinstall/common/ta/drm
DRM_PATH = ../../../../../../common/hal/source/trustlets/keyinstall/common/ta/drm
SRC_C += $(firstword $(wildcard $(DRM_PATH)/$(MTK_PROJECT)/key.c $(DRM_PATH)/$(ARCH_MTK_PLATFORM)/key.c) $(DRM_PATH)/common/key.c)

MY_CFLAGS = $(CFLAGS_PF_UT_BTA) $(CFLAGS_PF_UT_DRV_CLIENT)

MY_LDFLAGS = $(LDFLAGS_PF_UT_BTA) $(LDFLAGS_PF_UT_DRV_CLIENT)

MODE = shared

include $(UT_SDK_HOME)/build/prog.mk
