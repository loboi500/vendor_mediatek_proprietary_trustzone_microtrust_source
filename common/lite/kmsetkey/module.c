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

#include <stdbool.h>
#include <hardware/hardware.h>
#include "kmsetkey.h"
#include "module.h"

/* -------------------------------------------------------------------------
   Module definitions needed for integrtion with Android HAL framework.
   -------------------------------------------------------------------------*/

static struct hw_module_methods_t kmsetkey_module_methods = {
	.open = nv_kmsetkey_open,
};

__attribute__((visibility("default")))
struct kmsetkey_module HAL_MODULE_INFO_SYM = {
	.common =
	{
		.tag = HARDWARE_MODULE_TAG,
		.module_api_version = KMSETKEY_MODULE_API_VERSION_0_1,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.id = KMSETKEY_HARDWARE_MODULE_ID,
		.name = KMSETKEY_HARDWARE_MODULE_NAME,
		.author = "MediaTek",
		.methods = &kmsetkey_module_methods,
		.dso = 0,
		.reserved = {},
	},
};
