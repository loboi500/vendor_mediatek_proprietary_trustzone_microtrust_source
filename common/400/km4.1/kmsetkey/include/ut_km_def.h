/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KEYMASTER_DEF_H__
#define __UT_KEYMASTER_DEF_H__
/**
 * Command ID's
 */

#define CMD_ID_TEE_REE_IMPORT_KEYBOX 601
#define CMD_ID_TEE_REE_CHECK_KEYBOX 602
/*... add more curves when needed */

#define KM_MAGIC 0xaf015838

#define TEE_PROP_VALUE_MAX 84

typedef uint64_t secure_id_t;
typedef uint64_t salt_t;
typedef unsigned char uint8_t;


typedef enum {
	CMD_CA_IMPORT_KEYBOX = 0,
	CMD_CA_VERIFY_KEYBOX,

	CMD_TA_GET_KEYBOX = 100,
	CMD_TA_GET_KEYBOX_STATUS,
	CMD_WRITE_KEYBOX_MODEL_INFO,

	CMD_CA_TEST = 1000,

} kb_cmd_t;
/**
 * Command message.
 *
 * @param len Length of the data to process.
 * @param data Data to be processed
 */
typedef struct {
	uint32_t commandId;
} command_t;
/**
 * Response structure
 */
typedef struct {
	int32_t error;
} response_t;

typedef struct {
	uint32_t google_key_staus;
	uint32_t attk_status;
	uint32_t oemkey_status;
	uint32_t ifaa_key_status;
	uint32_t google_key_model_info_len;
	int8_t google_key_model_info[TEE_PROP_VALUE_MAX];
	uint32_t reserved0;
	uint32_t reserver1;
	uint32_t reserver2;
	uint32_t reserver3;
} key_status_t;


typedef struct {
	uint32_t key_material_size;
	uint32_t in_params_length;
	uint32_t out_entry_count;
	uint32_t datalen[4];
	uint32_t android_version;
	uint32_t km_version;
	uint32_t reserve[2];
	uint8_t name[12];
} attest_key_t;

typedef struct {
	uint32_t attest_keybox_len;
} import_attest_keybox_t;

/**
 * TCI message data.
 */
typedef struct {
	command_t command;
	response_t response;
	// if we get this param is 1, then we need to parse keymaster_key_param_t.block
	// also if we send a data, we need to check, if the keymaster_key_param_t data has a block type
	// param
	// if yes, we set this param 1, then add the buffer according to the rule you make,
	unsigned keymaster_key_param_t_has_block;
	union {
		import_attest_keybox_t import_attest_keybox;
	};
} ut_message_t;

#endif  // __UT_KEYMASTER_DEF_H__
