/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HalAdaptationLayer_H_
#define _HalAdaptationLayer_H_

#include <memory>
#include <stdlib.h>
#include <hardware/hardware.h>
#include "kmsetkey.h"

#undef  LOG_TAG
#define LOG_TAG "kmsetkey"


struct HalAdaptationLayer
{
    kmsetkey_device_t          _device;

/* -----------------------------------------------------------------------------
 * @brief   An interface for key injection from HIDL to HAL.
 *
 * @param   peakb: data buffer pointer
 * @param   peakb_len: data buffer length
 *
 * @returns:
 *          ERROR_NONE: Success
 *          An error code < 0 on failure
 -------------------------------------------------------------------------------- */
	static int32_t attest_key_install(const uint8_t *peakb, const uint32_t peakb_len);

    HalAdaptationLayer(hw_module_t* module);

private:
	// Struct is non-copyable and not default constructible
	HalAdaptationLayer();
	HalAdaptationLayer(const HalAdaptationLayer&);
	HalAdaptationLayer& operator=(const HalAdaptationLayer&);
};

#endif /* _HalAdaptationLayer_H_ */
