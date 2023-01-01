/*
 * Copyright 2018 The Android Open Source Project
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
 *
 * Modifications: Copyright (c) 2020 MICROTRUST Incorporated.
 */

#define LOG_TAG "beanpodkeymaster"

#include <cutils/log.h>
#include <keymaster/android_keymaster_messages.h>
#include <keymaster/keymaster_configuration.h>
#include <BeanpodKeymaster.h>
#include <beanpod_keymaster_ipc.h>
#include <unistd.h>
#include <android-base/properties.h>
#include <cutils/properties.h>


const char *km_debug_prop = "vendor.soter.teei.km.init";
// BeanpodKeymaster::Initialize() success flag.
static bool initialized = false;
#define INITIALIZE \
if (Initialize()) { \
	ALOGE("Initialize Beanpod Keymint failed\n"); \
	response->error = (keymaster_error_t)-1000; /*unknown error*/ \
	return; \
}

std::string wait_and_get_property(const char* prop) {
	std::string prop_value;
#ifndef KEYMASTER_UNIT_TEST_BUILD
	while (!android::base::WaitForPropertyCreation(prop)) {
		ALOGE("waited 15s for %s, still waiting...", prop);
	}
	prop_value = android::base::GetProperty(prop, "" /* default */);
#endif
	return prop_value;
}

namespace keymaster {
#define MAX_ANDROID_PROP_LEN 92
struct BPConfigureRequest : public ConfigureRequest {
	explicit BPConfigureRequest(int32_t ver) : ConfigureRequest(ver) {}
	size_t SerializedSize() const override {
		return sizeof(os_version) + sizeof(os_patchlevel)+MAX_ANDROID_PROP_LEN*5;
	}
	uint8_t* Serialize(uint8_t* buf, const uint8_t* end) const override {
		buf = append_uint32_to_buf(buf, end, os_version);
		buf = append_uint32_to_buf(buf, end, os_patchlevel);
		buf = append_to_buf(buf, end, brand, MAX_ANDROID_PROP_LEN);
		buf = append_to_buf(buf, end, manufacturer, MAX_ANDROID_PROP_LEN);
		buf = append_to_buf(buf, end, product, MAX_ANDROID_PROP_LEN);
		buf = append_to_buf(buf, end, model, MAX_ANDROID_PROP_LEN);
		return append_to_buf(buf, end, board, MAX_ANDROID_PROP_LEN);
	}

	char brand[MAX_ANDROID_PROP_LEN] = {0};
	char manufacturer[MAX_ANDROID_PROP_LEN] = {0};
	char product[MAX_ANDROID_PROP_LEN] = {0};
	char model[MAX_ANDROID_PROP_LEN] = {0};
	char board[MAX_ANDROID_PROP_LEN] = {0};
};

int BeanpodKeymaster::ConfigDeviceInfo(void)
{
	int32_t ver = 3;
	BPConfigureRequest req(ver);

	req.os_version = GetOsVersion();
	req.os_patchlevel = GetOsPatchlevel();

	std::string prop = wait_and_get_property("ro.product.brand");
	memcpy(req.brand, prop.c_str(), strlen(prop.c_str()));

	prop = wait_and_get_property("ro.product.manufacturer");
	memcpy(req.manufacturer, prop.c_str(), strlen(prop.c_str()));

	prop = wait_and_get_property("ro.product.name");
	memcpy(req.product, prop.c_str(), strlen(prop.c_str()));

	prop = wait_and_get_property("ro.product.model");
	memcpy(req.model, prop.c_str(), strlen(prop.c_str()));

	prop = wait_and_get_property("ro.product.board");
	memcpy(req.board, prop.c_str(), strlen(prop.c_str()));

	ConfigureResponse rsp(ver);
	(void)property_set(km_debug_prop, "configure");

	Configure(req, &rsp);

	if (rsp.error != KM_ERROR_OK) {
		(void)property_set(km_debug_prop, "configure_failed");
		ALOGE("Failed to configure keymaster %d", rsp.error);
		return -1;
	}
	(void)property_set(km_debug_prop, "configure_success");
	return 0;
}

int BeanpodKeymaster::ConfigPatchLevel(void)
{
	(void)property_set(km_debug_prop, "config_patch_level");
	int32_t ver = 3;

	ConfigureVendorPatchlevelRequest vendor_pl_req(ver);
	ConfigureVendorPatchlevelResponse vendor_pl_rsp(ver);
	vendor_pl_req.vendor_patchlevel = GetVendorPatchlevel();
	ConfigureVendorPatchLevel(vendor_pl_req, &vendor_pl_rsp);
	if (vendor_pl_rsp.error != KM_ERROR_OK) {
		(void)property_set(km_debug_prop, "configure_vendor_pl_failed");
		ALOGE("Failed to configure vendor patch level %d", vendor_pl_rsp.error);
		return -1;
	}

	ConfigureBootPatchlevelRequest  boot_pl_req(ver);
	ConfigureBootPatchlevelResponse boot_pl_rsp(ver);

	boot_pl_req.boot_patchlevel = (GetOsPatchlevel() * 100 + 1);

	ConfigureBootPatchLevel(boot_pl_req, &boot_pl_rsp);
	if (boot_pl_rsp.error != KM_ERROR_OK) {
		(void)property_set(km_debug_prop, "configure_boot_pl_failed");
		ALOGE("Failed to configure boot patch level %d", boot_pl_rsp.error);
		return -1;
	}
	(void)property_set(km_debug_prop, "config_patch_level_sucess");
	return 0;
}

int BeanpodKeymaster::Initialize() {
	if (initialized) {
		// already initialized.
		return 0;
	}
	(void)property_set(km_debug_prop, "load_ta");
	int err;
	char prop_value[PROPERTY_VALUE_MAX] = { 0 };

	ALOGI("keymaster connect start");
	err = property_get("vendor.soter.teei.init", prop_value, NULL);
	while (err < 1) { // not found vendor.soter.teei.init android prop.
		(void)property_set(km_debug_prop, "wait_teei_daemon");
		memset(prop_value, 0, PROPERTY_VALUE_MAX);
		usleep(200000); // 200 ms
		err = property_get("vendor.soter.teei.init", prop_value, NULL);
	}

	err = bp_keymaster_connect();
	if (err) {
		(void)property_set(km_debug_prop, "load_ta_failed");
		ALOGE("Failed to connect to beanpod keymaster %d", err);
		return err;
	}
	(void)property_set(km_debug_prop, "load_ta_success");

	ALOGI("keymaster connect end");

	if (ConfigDeviceInfo()) {
		ALOGE("config keymint failed");
		return -1;
	}

	if (ConfigPatchLevel()) {
		ALOGE("config patch level failed");
		return -1;
	}

	initialized = true;
	return 0;
}

BeanpodKeymaster::BeanpodKeymaster() {}

BeanpodKeymaster::~BeanpodKeymaster() {
	bp_keymaster_disconnect();
}

static void ForwardCommand(enum keymaster_command command,
				const Serializable& req,
				KeymasterResponse* rsp) {
	keymaster_error_t err;
	err = bp_keymaster_send(command, req, rsp);
	if (err != KM_ERROR_OK) {
		ALOGE("Failed to send cmd %d err: %d", command, err);
		rsp->error = err;
	}
}

static void ForwardCommandNoRequest(enum keymaster_command command, KeymasterResponse* rsp) {
	keymaster_error_t err;
	err = bp_keymaster_send_no_request(command, rsp);
	if (err != KM_ERROR_OK) {
		ALOGE("BeanpodKeymaster Failed to send cmd %d err: %d", command, err);
		rsp->error = err;
	}
}

void BeanpodKeymaster::GetVersion(const GetVersionRequest& request, GetVersionResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_VERSION, request, response);
}

void BeanpodKeymaster::SupportedAlgorithms(const SupportedAlgorithmsRequest& request,
					SupportedAlgorithmsResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_ALGORITHMS, request, response);
}

void BeanpodKeymaster::SupportedBlockModes(const SupportedBlockModesRequest& request,
					SupportedBlockModesResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_BLOCK_MODES, request, response);
}

void BeanpodKeymaster::SupportedPaddingModes(const SupportedPaddingModesRequest& request,
					SupportedPaddingModesResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_PADDING_MODES, request, response);
}

void BeanpodKeymaster::SupportedDigests(const SupportedDigestsRequest& request,
					SupportedDigestsResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_DIGESTS, request, response);
}

void BeanpodKeymaster::SupportedImportFormats(const SupportedImportFormatsRequest& request,
					SupportedImportFormatsResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_IMPORT_FORMATS, request, response);
}

void BeanpodKeymaster::SupportedExportFormats(const SupportedExportFormatsRequest& request,
					SupportedExportFormatsResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_SUPPORTED_EXPORT_FORMATS, request, response);
}

void BeanpodKeymaster::AddRngEntropy(const AddEntropyRequest& request,
				AddEntropyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_ADD_RNG_ENTROPY, request, response);
}

void BeanpodKeymaster::Configure(const ConfigureRequest& request, ConfigureResponse* response) {
	ForwardCommand(KM_CONFIGURE, request, response);
}

void BeanpodKeymaster::GenerateKey(const GenerateKeyRequest& request,
				GenerateKeyResponse* response) {
	ALOGI("BeanpodKeymaster begin generate key");
	INITIALIZE;
	GenerateKeyRequest datedRequest(request.message_version);
	datedRequest.key_description = request.key_description;

	if (request.key_description.Contains(TAG_CREATION_DATETIME)) {
		datedRequest.key_description.push_back(TAG_CREATION_DATETIME, java_time(time(NULL)));
	}
	datedRequest.attestation_signing_key_blob = request.attestation_signing_key_blob;
	datedRequest.attest_key_params = request.attest_key_params;
	datedRequest.issuer_subject  = request.issuer_subject;

	ForwardCommand(KM_GENERATE_KEY, datedRequest, response);
}

void BeanpodKeymaster::GetKeyCharacteristics(const GetKeyCharacteristicsRequest& request,
					GetKeyCharacteristicsResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GET_KEY_CHARACTERISTICS, request, response);
}

void BeanpodKeymaster::ImportKey(const ImportKeyRequest& request, ImportKeyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_IMPORT_KEY, request, response);
}

void BeanpodKeymaster::ImportWrappedKey(const ImportWrappedKeyRequest& request,
					ImportWrappedKeyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_IMPORT_WRAPPED_KEY, request, response);
}

void BeanpodKeymaster::ExportKey(const ExportKeyRequest& request, ExportKeyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_EXPORT_KEY, request, response);
}

void BeanpodKeymaster::AttestKey(const AttestKeyRequest& request, AttestKeyResponse* response) {
	ALOGI("Enter BeanpodKeymaster AttestKey");
	INITIALIZE;
	ForwardCommand(KM_ATTEST_KEY, request, response);
	ALOGI("Exit BeanpodKeymaster AttestKey, cerlen=%zd, error=%d", response->certificate_chain.entry_count, response->error);
#ifdef DEBUG
	unsigned int i = 0;
	int fd = -1;
	int len = 0;
	for (i = 0; i < response->certificate_chain.entry_count; i++) {
		if (i == 0)
			fd = open("/data/vendor/thh/0.pem", O_CREAT | O_RDWR, 0660);
		else if (i == 1)
			fd = open("/data/vendor/thh/1.pem", O_CREAT | O_RDWR, 0660);
		else if (i == 2)
			fd = open("/data/vendor/thh/2.pem", O_CREAT | O_RDWR, 0660);
		else if (i == 3)
			fd = open("/data/vendor/thh/3.pem", O_CREAT | O_RDWR, 0660);
		else
			fd = open("/data/vendor/thh/x.pem", O_CREAT | O_RDWR, 0660);
		len = write(fd, response->certificate_chain.entries[i].data, response->certificate_chain.entries[i].data_length);
		close(fd);
	}
#endif
}

void BeanpodKeymaster::UpgradeKey(const UpgradeKeyRequest& request, UpgradeKeyResponse* response) {
	ALOGI("Enter BeanpodKeymaster UpgradeKey");
	INITIALIZE;
	ForwardCommand(KM_UPGRADE_KEY, request, response);
	ALOGI("Exit BeanpodKeymaster UpgradeKey");
}

void BeanpodKeymaster::DeleteKey(const DeleteKeyRequest& request, DeleteKeyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_DELETE_KEY, request, response);
}

void BeanpodKeymaster::DeleteAllKeys(const DeleteAllKeysRequest& request,
				DeleteAllKeysResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_DELETE_ALL_KEYS, request, response);
}

void BeanpodKeymaster::BeginOperation(const BeginOperationRequest& request,
					BeginOperationResponse* response) {
	ALOGI("Enter BeanpodKeymaster BeginOperation");
	INITIALIZE;
	ForwardCommand(KM_BEGIN_OPERATION, request, response);
	ALOGI("Exit BeanpodKeymaster BeginOperation");
}

void BeanpodKeymaster::UpdateOperation(const UpdateOperationRequest& request,
					UpdateOperationResponse* response) {
	ALOGI("Enter BeanpodKeymaster UpdateOperation");
	ForwardCommand(KM_UPDATE_OPERATION, request, response);
	ALOGI("Exit BeanpodKeymaster UpdateOperation");
}

void BeanpodKeymaster::FinishOperation(const FinishOperationRequest& request,
					FinishOperationResponse* response) {
	ALOGI("Enter BeanpodKeymaster FinishOperation");
	ForwardCommand(KM_FINISH_OPERATION, request, response);
	ALOGI("Exit BeanpodKeymaster FinishOperation");
}

void BeanpodKeymaster::AbortOperation(const AbortOperationRequest& request,
					AbortOperationResponse* response) {
	ALOGI("Enter BeanpodKeymaster AbortOperation");
	ForwardCommand(KM_ABORT_OPERATION, request, response);
	ALOGI("Exit BeanpodKeymaster AbortOperation");
}

/* Methods for Keymaster 4.0 functionality -- not yet implemented */
void BeanpodKeymaster::GetHmacSharingParameters(GetHmacSharingParametersResponse* response) {
	INITIALIZE;
	ForwardCommandNoRequest(KM_GET_HMAC_SHARING_PARAMETERS, response);
}

void BeanpodKeymaster::ComputeSharedHmac(const ComputeSharedHmacRequest& request,
					ComputeSharedHmacResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_COMPUTE_SHARED_HMAC, request, response);
}

void BeanpodKeymaster::VerifyAuthorization(const VerifyAuthorizationRequest& request,
					VerifyAuthorizationResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_VERIFY_AUTHORIZATION, request, response);
}

void BeanpodKeymaster::DeviceLocked(const DeviceLockedRequest& request, DeviceLockedResponse* response) {
	ALOGI("Enter deviceLocked");
	INITIALIZE;
	ForwardCommand(KM_DEVICE_LOCKED, request, response);
}

void BeanpodKeymaster::EarlyBootEnded(EarlyBootEndedResponse* response) {
	ALOGI("Enter earlyBootEnded");
	INITIALIZE;
	ForwardCommandNoRequest(KM_EARLY_BOOT_ENDED, response);
	return ;
}

void BeanpodKeymaster::GenerateRkpKey(const GenerateRkpKeyRequest& request,
					GenerateRkpKeyResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GENERATE_RKP_KEY, request, response);
}

void BeanpodKeymaster::GenerateCsr(const GenerateCsrRequest& request,
					GenerateCsrResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GENERATE_CSR, request, response);
}

void BeanpodKeymaster::GenerateTimestampToken(const GenerateTimestampTokenRequest& request,
						GenerateTimestampTokenResponse* response) {
	INITIALIZE;
	ForwardCommand(KM_GENERATE_TIMESTAMPTOKEN, request, response);
}

void BeanpodKeymaster::ConfigureVendorPatchLevel(const ConfigureVendorPatchlevelRequest& request,
						ConfigureVendorPatchlevelResponse* response) {
	ForwardCommand(KM_CONFIGURE_VENDOR_PATCHLEVEL, request, response);
}

void BeanpodKeymaster::ConfigureBootPatchLevel(const ConfigureBootPatchlevelRequest& request,
						ConfigureBootPatchlevelResponse* response) {
	ForwardCommand(KM_CONFIGURE_BOOT_PATCHLEVEL, request, response);
}

}//namespace keymaster
