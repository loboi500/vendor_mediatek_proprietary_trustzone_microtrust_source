/*
 * Copyright (C) 2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#define IMSG_TAG "ut_kmsetkey"

#include <tee_client_api.h>
#include <string.h>
#include <imsg_log.h>
#include "ut_km_tac.h"
#include "ut_km_def.h"
#include <log/log.h>
#include <cutils/properties.h>


#define GOOGLE_KEY_STATUS "vendor.soter.teei.googlekey.status"
#define GOOGLE_KEY_MODEL "vendor.soter.teei.googlekey.model"
#define ATTESTKEY_FLAG 0xaf015838

static const char* host_name = "bta_loader";

static const TEEC_UUID KEYBOX_UUID =  { 0xd91f322a, 0xd5a4, 0x41d5, { 0x95, 0x51, 0x10, 0xed, 0xa3, 0x27, 0x2f, 0xc0 } };

int ut_km_import_google_key_model(unsigned char *data, unsigned int datalen) {
	if (data == NULL || datalen == 0)
		return -1;

	TEEC_Context context;
	TEEC_Session session ;
	TEEC_Operation operation ;
	TEEC_SharedMemory inputSM;
	TEEC_Result result;
	uint32_t returnOrigin = 0;

	memset((void *)&context, 0, sizeof(TEEC_Context));
	memset((void *)&session, 0, sizeof(TEEC_Session));
	memset((void *)&operation, 0, sizeof(TEEC_Operation));

	result = TEEC_InitializeContext(host_name, &context);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_InitializeContext FAILED, err:%x",result);
		goto cleanup_1;
	}

	result = TEEC_OpenSession(
	                 &context,
	                 &session,
	                 &KEYBOX_UUID,
	                 TEEC_LOGIN_PUBLIC,
	                 NULL,
	                 NULL,
	                 &returnOrigin);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_OpenSession FAILED, err:%x",result);
		goto cleanup_2;
	}

	inputSM.size = datalen;
	inputSM.flags = TEEC_MEM_INPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		ALOGE("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);
	memcpy(inputSM.buffer, data, datalen);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
	                                        TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
	operation.started = 1;

	result = TEEC_InvokeCommand(&session,CMD_WRITE_KEYBOX_MODEL_INFO,&operation,NULL);
	if (result != TEEC_SUCCESS) {
		ALOGE("TEEC_InvokeCommand FAILED, err:%x",result);
		goto cleanup_4;
	}

	result = operation.params[0].value.a;
	if (result != TEEC_SUCCESS) {
		ALOGE("ta busness error, %d", result);
	}

cleanup_4:
	TEEC_ReleaseSharedMemory(&inputSM);
cleanup_3:
	TEEC_CloseSession(&session);
cleanup_2:
	TEEC_FinalizeContext(&context);
cleanup_1:
	return result;
}

int ut_km_import_google_key(unsigned char *data, unsigned int datalen) {
	if (data == NULL || datalen == 0)
		return -1;

	TEEC_Context context;
	TEEC_Session session ;
	TEEC_Operation operation ;
	TEEC_SharedMemory inputSM;
	TEEC_Result result;
	uint32_t returnOrigin = 0;

	memset((void *)&context, 0, sizeof(TEEC_Context));
	memset((void *)&session, 0, sizeof(TEEC_Session));
	memset((void *)&operation, 0, sizeof(TEEC_Operation));

	result = TEEC_InitializeContext(host_name, &context);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_InitializeContext FAILED, err:%x",result);
		goto cleanup_1;
	}

	result = TEEC_OpenSession(
	                 &context,
	                 &session,
	                 &KEYBOX_UUID,
	                 TEEC_LOGIN_PUBLIC,
	                 NULL,
	                 NULL,
	                 &returnOrigin);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_OpenSession FAILED, err:%x",result);
		goto cleanup_2;
	}

	inputSM.size = datalen;
	inputSM.flags = TEEC_MEM_INPUT;
	result = TEEC_AllocateSharedMemory(&context, &inputSM);
	if (result != TEEC_SUCCESS || inputSM.buffer == 0) {
		ALOGE("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}


	memset(inputSM.buffer, 0, inputSM.size);
	memcpy(inputSM.buffer, data, datalen);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
	                                        TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &inputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = inputSM.size;
	operation.started = 1;

	result = TEEC_InvokeCommand(&session,0,&operation,NULL);
	if (result != TEEC_SUCCESS) {
		ALOGE("TEEC_InvokeCommand FAILED, err:%x",result);
		goto cleanup_4;
	}

	result = operation.params[0].value.a;
	if (result != TEEC_SUCCESS) {
		ALOGE("ta busness error, %d", result);
	}

cleanup_4:
	TEEC_ReleaseSharedMemory(&inputSM);
cleanup_3:
	TEEC_CloseSession(&session);
cleanup_2:
	TEEC_FinalizeContext(&context);
cleanup_1:
	return result;
}

int ut_km_check_google_key(void) {
	ALOGI("ut_km_check_google_key start.");

	TEEC_Context context;
	TEEC_Session session ;
	TEEC_Operation operation ;
	TEEC_SharedMemory outputSM;
	TEEC_Result result;
	uint32_t returnOrigin = 0;
	key_status_t key_status;
	uint32_t key_status_size = sizeof(key_status_t);

	memset((void *)&context, 0, sizeof(TEEC_Context));
	memset((void *)&session, 0, sizeof(TEEC_Session));
	memset((void *)&operation, 0, sizeof(TEEC_Operation));
	memset(&key_status, 0, key_status_size);

	property_set(GOOGLE_KEY_STATUS, "fail");
	property_set(GOOGLE_KEY_MODEL, "fail");

	result = TEEC_InitializeContext(host_name, &context);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_InitializeContext FAILED, err:%x",result);
		goto cleanup_1;
	}

	result = TEEC_OpenSession(
	                 &context,
	                 &session,
	                 &KEYBOX_UUID,
	                 TEEC_LOGIN_PUBLIC,
	                 NULL,
	                 NULL,
	                 &returnOrigin);
	if(result != TEEC_SUCCESS) {
		ALOGE("TEEC_OpenSession FAILED, err:%x",result);
		goto cleanup_2;
	}

	outputSM.size = key_status_size;
	outputSM.flags = TEEC_MEM_OUTPUT;
	result = TEEC_AllocateSharedMemory(&context, &outputSM);
	if (result != TEEC_SUCCESS || outputSM.buffer == NULL) {
		ALOGE("TEEC_AllocateSharedMemory FAILED, err:%x",result);
		goto cleanup_3;
	}

	memset(outputSM.buffer, 0, outputSM.size);

	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
	                                        TEEC_MEMREF_PARTIAL_OUTPUT, TEEC_NONE, TEEC_NONE);

	operation.params[1].memref.parent = &outputSM;
	operation.params[1].memref.offset = 0;
	operation.params[1].memref.size = outputSM.size;
	operation.started = 1;

	result = TEEC_InvokeCommand(&session, CMD_TA_GET_KEYBOX_STATUS, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		ALOGE("TEEC_InvokeCommand FAILED, err:%x",result);
		goto cleanup_4;
	}

	result = operation.params[0].value.a;
	if (result != 0) {
		ALOGE("check keybox failed, %d", result);
		result = -1;
		goto cleanup_4;
	}

	memcpy(&key_status, outputSM.buffer, key_status_size);

	if(key_status.google_key_staus == KM_MAGIC) {
		property_set(GOOGLE_KEY_STATUS, "ok");
		result = 0;
		if (TEE_PROP_VALUE_MAX-1 > key_status.google_key_model_info_len > 0) {
			property_set(GOOGLE_KEY_MODEL, (const char*)key_status.google_key_model_info);
		} else {
			ALOGE("google key model info len invalid, len:%d, %s.", key_status.google_key_model_info_len, key_status.google_key_model_info);
			property_set(GOOGLE_KEY_MODEL, "unknown");
		}
	} else {
		property_set(GOOGLE_KEY_STATUS, "fail");
		property_set(GOOGLE_KEY_MODEL, "fail");
		result = -1;
	}

	ALOGI("keybox already install, %08x, %08x", result, ATTESTKEY_FLAG);

cleanup_4:
	TEEC_ReleaseSharedMemory(&outputSM);
cleanup_3:
	TEEC_CloseSession(&session);
cleanup_2:
	TEEC_FinalizeContext(&context);
cleanup_1:
	return result;
}

int ut_km_import_google_key_and_model(unsigned char *data, unsigned int datalen) {
	if (data == NULL || datalen == 0)
		return -1;

	int ret = 0;
	unsigned int len = 0;
	char property_value[PROP_VALUE_MAX] = {0};

	ret = ut_km_import_google_key(data, datalen);
	if(ret != 0) {
		property_set(GOOGLE_KEY_STATUS, "fail");
		ALOGE("write google attest key failed, %d", ret);
		return ret;
	} else {
		if (property_get(ATTESTKEY_MODEL_INFO_PROPERTY, property_value, NULL) > 0) {
			len = strlen(property_value);
			if (len < TEE_PROP_VALUE_MAX) {
				ret = ut_km_import_google_key_model((uint8_t*)property_value, len);
				memset(property_value+len, 0, PROP_VALUE_MAX-len);
			} else {
				ret = ut_km_import_google_key_model((uint8_t*)property_value, TEE_PROP_VALUE_MAX-1);
				memset(property_value+TEE_PROP_VALUE_MAX-1, 0, PROP_VALUE_MAX-TEE_PROP_VALUE_MAX-1);
			}
			if (ret) {
				ALOGE("write google attest key model info failed, %d", ret);
				return ret;
			}
		} else {
			ALOGE("get google attest key model failed, %s.", property_value);
			return -2;
		}
	}
	property_set(GOOGLE_KEY_STATUS, "ok");
	property_set(GOOGLE_KEY_MODEL, property_value);

	return ret;
}
