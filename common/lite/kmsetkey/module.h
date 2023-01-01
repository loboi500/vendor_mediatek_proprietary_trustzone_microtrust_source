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

#ifndef _module_H_
#define _module_H_

#define KMSETKEY_HARDWARE_MODULE_NAME "Hardware kmsetkey HAL"
#define KMSETKEY_HARDWARE_MODULE_ID "kmsetkey"
#define KMSETKEY_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)


#ifdef __cplusplus
extern "C" {
#endif
int nv_kmsetkey_open(const struct hw_module_t* module, const char* id,
		struct hw_device_t** device);

int nv_kmsetkey_close(hw_device_t *hw);

#ifdef __cplusplus
}
#endif

#endif
