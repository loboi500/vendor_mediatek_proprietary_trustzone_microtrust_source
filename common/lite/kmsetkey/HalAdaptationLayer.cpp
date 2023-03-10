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

#include "HalAdaptationLayer.h"
#include "module.h"
#include <memory>
#include <cutils/log.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <log/log.h>
#include <cutils/log.h>
#include <kmsetkey_ipc.h>

extern struct kmsetkey_module HAL_MODULE_INFO_SYM;

const uint8_t Input_Ekkb_pub[256] = {
	0x54, 0xCB, 0xCD, 0xB3, 0xFF, 0xCD, 0xCC, 0xDD, 0xA3, 0xFF, 0x5D, 0x30, 0x95, 0x03, 0x2D, 0x2A,
	0x2A, 0x12, 0xE6, 0x90, 0x0B, 0x3F, 0xF7, 0x85, 0xE5, 0xB3, 0xDD, 0x8E, 0x4B, 0x27, 0x9D, 0x58,
	0xC8, 0x24, 0x7A, 0xB0, 0x83, 0x8B, 0xB1, 0xD4, 0xA4, 0x92, 0x17, 0x0E, 0xF2, 0xCF, 0x19, 0x9A,
	0xAB, 0xCE, 0xFB, 0x68, 0xCB, 0x86, 0x94, 0x6E, 0x16, 0x8E, 0x3D, 0xCC, 0xF8, 0x0C, 0xA6, 0x30,
	0x1C, 0x47, 0xA6, 0xB6, 0x50, 0x2F, 0x68, 0x94, 0x23, 0x0C, 0x62, 0xAF, 0xE1, 0x44, 0xA4, 0x27,
	0xD8, 0x79, 0x05, 0x68, 0x51, 0x89, 0x04, 0x49, 0x61, 0x93, 0x7A, 0xEF, 0xB5, 0xB9, 0x17, 0x72,
	0x28, 0x87, 0xBA, 0x94, 0x4A, 0xB8, 0xF1, 0x46, 0xCF, 0xE7, 0x53, 0x0A, 0x02, 0x5A, 0xEE, 0x59,
	0x47, 0xBE, 0xC2, 0x41, 0x98, 0xD9, 0x5B, 0x17, 0xAF, 0x10, 0x0B, 0xE0, 0x92, 0xBA, 0x65, 0x30,
	0x63, 0x76, 0x94, 0x2A, 0x26, 0x7D, 0x3F, 0x94, 0x2E, 0x9F, 0x06, 0xB8, 0xD3, 0xB0, 0x76, 0xE9,
	0xBD, 0xBA, 0x07, 0x6E, 0xE1, 0x3D, 0x1F, 0xC6, 0xDB, 0x7F, 0x34, 0xC1, 0xB4, 0xED, 0x8B, 0x00,
	0x36, 0xAE, 0x1E, 0xBB, 0x65, 0x81, 0x38, 0x94, 0x77, 0xE2, 0x4E, 0x5C, 0xC1, 0x9F, 0x93, 0x2D,
	0x29, 0xA3, 0x30, 0x29, 0xF7, 0xEC, 0xFC, 0xCC, 0x87, 0x3F, 0xFA, 0x09, 0xAD, 0x1E, 0xE5, 0xAF,
	0x4E, 0xCF, 0x0E, 0x44, 0x8C, 0xE3, 0xBF, 0x8D, 0x5B, 0xEB, 0xD6, 0xA0, 0xEA, 0xC6, 0xBF, 0xB1,
	0x56, 0xD5, 0xC9, 0xE6, 0xB8, 0xE1, 0xB9, 0x94, 0x85, 0xAD, 0x38, 0x38, 0xDD, 0xE2, 0x57, 0xCC,
	0xFE, 0xED, 0xF0, 0x2A, 0x10, 0xB6, 0x8E, 0x3C, 0xA2, 0x4D, 0x97, 0x60, 0x3E, 0xEC, 0x92, 0xE2,
	0xC1, 0x72, 0xB6, 0x38, 0xE2, 0xC0, 0xA8, 0xCA, 0xD6, 0xEB, 0x0C, 0x35, 0xE9, 0x3E, 0x8D, 0x91,
};

const uint8_t InputPkb[129] = {
	0x00,
	0x05, 0x14, 0x24, 0x14, 0x2F, 0xE1, 0xFC, 0x61, 0xB1, 0x0B, 0x97, 0xAF, 0x5C, 0x66, 0xB0, 0xF6,
	0x15, 0x26, 0xF6, 0x1C, 0x96, 0xC8, 0xBA, 0x96, 0x77, 0xD5, 0x4E, 0xFB, 0xA4, 0x91, 0xFD, 0xFB,
	0x16, 0x33, 0xED, 0x6E, 0xCC, 0x41, 0x0F, 0xCF, 0xC2, 0x94, 0xC2, 0x64, 0x1C, 0xFA, 0x12, 0x66,
	0x04, 0xE3, 0x4C, 0xF0, 0xB4, 0x5F, 0x15, 0x4B, 0xDB, 0xF4, 0x29, 0x1F, 0x98, 0xD3, 0xF5, 0x4E,
	0xA8, 0xD2, 0xA1, 0x0E, 0x8B, 0x59, 0xBF, 0x17, 0xDE, 0xB7, 0xA7, 0x50, 0x02, 0x2F, 0x14, 0x9C,
	0x7A, 0xE5, 0x24, 0x6D, 0x0E, 0x9F, 0xDE, 0x45, 0x4D, 0x6A, 0x75, 0x06, 0xB3, 0xDA, 0x88, 0x86,
	0x8D, 0xA6, 0x11, 0x43, 0xA8, 0x17, 0xA9, 0x6F, 0x70, 0x27, 0x01, 0xDA, 0xFA, 0xAF, 0xF6, 0xA8,
	0x47, 0xEC, 0xEF, 0x29, 0x66, 0x4E, 0xA8, 0x7C, 0x99, 0xFA, 0x40, 0xB8, 0xD4, 0x8A, 0x2C, 0xB1,
};

extern "C" {

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_open( const struct hw_module_t* module, const char* id,
					struct hw_device_t** device)
{
	ALOGI("opening nv kmsetkey device.\n");

	if ( id == NULL )
		return -EINVAL;

	// Make sure we initialize only if module provided is known
	if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
			(module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
			(module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
			(0!=memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
					   sizeof(KMSETKEY_HARDWARE_MODULE_NAME)-1)) )
	{
		return -EINVAL;
	}

	std::unique_ptr<HalAdaptationLayer> kmsetkey_device(
			new HalAdaptationLayer(const_cast<hw_module_t*>(module)));

	if (!kmsetkey_device)
	{
		ALOGE("Heap exhuasted. Exiting...");
		return -ENOMEM;
	}

	*device = reinterpret_cast<hw_device_t*>(kmsetkey_device.release());
	ALOGI("Kmsetkey device created");
	return 0;
}

/******************************************************************************/
__attribute__((visibility("default")))
int nv_kmsetkey_close(hw_device_t *hw)
{
	if (hw == NULL)
		return 0; // Nothing to close closed

	HalAdaptationLayer* gk = reinterpret_cast<HalAdaptationLayer*>(hw);
	if (NULL == gk)
	{
		ALOGE("Kmsetkey not initialized.");
		return -ENODEV;
	}

	delete gk;
	return 0;
}
} // extern "C"


static int32_t decrypt_key_install(int handle)
{
	int32_t rc;
	uint32_t inlen = 0, out_size = 0;
	uint32_t offset = 0;
	uint8_t *buf = NULL;
	uint8_t out[MAX_MSG_SIZE];

	inlen = sizeof(Input_Ekkb_pub) + sizeof(InputPkb);

	buf = (uint8_t*)malloc(inlen);
	if (buf == NULL) return -1;

	memcpy(buf + offset, Input_Ekkb_pub, sizeof(Input_Ekkb_pub));
	offset += sizeof(Input_Ekkb_pub);
	memcpy(buf + offset, InputPkb, sizeof(InputPkb));

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(handle, SET_DECRYPT_KEY, true, buf, inlen, out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, SET_DECRYPT_KEY);
	}

	if (buf) free(buf);

	return rc;
}
/* -------------------------------------------------------------------------
   Implementation of HalAdaptationLayer methods
   -------------------------------------------------------------------------*/

int32_t HalAdaptationLayer::attest_key_install(const uint8_t *peakb,
		const uint32_t peakb_len)
{
	int32_t rc;
	uint32_t payload_offset, payload_len, out_size;
	uint8_t out[MAX_MSG_SIZE];
	struct kmsetkey_msg *msg;
	int handle;

	ALOGI("enter attest keybox HAL!\n");

	rc = kmsetkey_connect(&handle);
	if (rc < 0) {
		ALOGE("kmsetkey_connect failed: %d\n", rc);
		return rc;
	}

	rc = decrypt_key_install(handle);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, SET_DECRYPT_KEY);
		goto exit;
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(handle, KEY_LEN, true, (uint8_t *)&peakb_len, sizeof(uint32_t), out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_LEN);
		goto exit;
	}

	for (payload_offset = 0; payload_offset < peakb_len; payload_offset += payload_len) {
		payload_len = peakb_len - payload_offset < MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg) ? peakb_len - payload_offset : MAX_MSG_SIZE - IPC_MSG_SIZE - sizeof(struct kmsetkey_msg);
		out_size = MAX_MSG_SIZE;
		rc = kmsetkey_call(handle, KEY_BUF, payload_offset + payload_len < peakb_len ? false : true, peakb + payload_offset, payload_len, out, &out_size);
		if (rc < 0) {
			ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, KEY_BUF);
			goto exit;
		}
	}

	out_size = MAX_MSG_SIZE;
	rc = kmsetkey_call(handle, SET_KEY, true, NULL, 0, out, &out_size);
	if (rc < 0) {
		ALOGE("kmsetkey_call failed: %d for cmd %u\n", rc, SET_KEY);
	}

exit:
	kmsetkey_disconnect(&handle);
	return rc < 0 ? rc : 0;
}


/******************************************************************************/
HalAdaptationLayer::HalAdaptationLayer(hw_module_t* module)
{
	ALOGI("kmsetkey: HalAdaptationLayer constructor initialized.");
	/* -------------------------------------------------------------------------
	   Device description
	   -------------------------------------------------------------------------*/
	_device.common.tag = HARDWARE_MODULE_TAG;
	_device.common.version = 1;
	_device.common.module = module;
	_device.common.close = nv_kmsetkey_close;

	/* -------------------------------------------------------------------------
	   All function pointers used by the HAL module
	   -------------------------------------------------------------------------*/
	_device.attest_key_install = HalAdaptationLayer::attest_key_install;

}

/******************************************************************************/


