/*
 * Copyright 2020, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "beanpodkeymaster"
#include <log/log.h>

#include "BeanpodKeyMintOperation.h"

#include <aidl/android/hardware/security/keymint/ErrorCode.h>
#include <aidl/android/hardware/security/secureclock/ISecureClock.h>

#include <keymaster/android_keymaster.h>

#include "KeyMintUtils.h"

#include <BeanpodKeymaster.h>
#include <beanpod_keymaster_ipc.h>

using keymaster::BeanpodKeymaster;

namespace aidl::android::hardware::security::keymint {

using ::keymaster::AbortOperationRequest;
using ::keymaster::AbortOperationResponse;
using ::keymaster::Buffer;
using ::keymaster::FinishOperationRequest;
using ::keymaster::FinishOperationResponse;
using ::keymaster::TAG_ASSOCIATED_DATA;
using ::keymaster::UpdateOperationRequest;
using ::keymaster::UpdateOperationResponse;
using secureclock::TimeStampToken;
using namespace km_utils;

static void SwapBytes(uint8_t* bytes, uint32_t size) {
	uint8_t* first = NULL;
	uint8_t* last = NULL;
	for (first = bytes, last = bytes + size - 1; first < last; ++first, --last) {
		uint8_t temp = *first;
		*first = *last;
		*last = temp;
	}
	return;
}

BeanpodKeyMintOperation::BeanpodKeyMintOperation(
	const shared_ptr<BeanpodKeymaster> implementation,
	keymaster_operation_handle_t opHandle)
	: impl_(std::move(implementation)), opHandle_(opHandle) {}

BeanpodKeyMintOperation::~BeanpodKeyMintOperation() {
	if (opHandle_ != 0) {
		abort();
	}
}

ScopedAStatus
BeanpodKeyMintOperation::updateAad(const vector<uint8_t>& input,
				const optional<HardwareAuthToken>& /* authToken */,
				const optional<TimeStampToken>& /* timestampToken */) {
	UpdateOperationRequest request(impl_->message_version());
	request.op_handle = opHandle_;
	request.additional_params.push_back(TAG_ASSOCIATED_DATA, input.data(), input.size());

	UpdateOperationResponse response(impl_->message_version());
	impl_->UpdateOperation(request, &response);
	if (response.error != KM_ERROR_OK) {
		if (response.error == BEANPOD_KM_INVALID_INPUT_LENGTH) {
			response.error = KM_ERROR_INVALID_INPUT_LENGTH;
			abort();
		}
	}

	return kmError2ScopedAStatus(response.error);
}

ScopedAStatus BeanpodKeyMintOperation::update(const vector<uint8_t>& input,
					const optional<HardwareAuthToken>& authToken,
					const optional<TimeStampToken>&
					/* timestampToken */,
					vector<uint8_t>* output) {
	if (!output) return kmError2ScopedAStatus(KM_ERROR_OUTPUT_PARAMETER_NULL);

	UpdateOperationRequest request(impl_->message_version());
	request.op_handle = opHandle_;
	request.input.Reinitialize(input.data(), input.size());

	hw_auth_token_t auth_token;
	memset(&auth_token, 0, sizeof(hw_auth_token_t));
	if (authToken) {
		ALOGI("contain authtoken\n");
		auth_token.version = 0;
		auth_token.challenge = authToken->challenge;
		auth_token.user_id = authToken->userId;
		auth_token.authenticator_id = authToken->authenticatorId;
		auth_token.authenticator_type = static_cast<uint32_t>(authToken->authenticatorType);
		auth_token.timestamp = authToken->timestamp.milliSeconds;
		memcpy(auth_token.hmac, authToken->mac.data(), authToken->mac.size());
		SwapBytes((uint8_t*)&(auth_token.authenticator_type), sizeof(auth_token.authenticator_type));
		SwapBytes((uint8_t*)&(auth_token.timestamp), sizeof(auth_token.timestamp));
		request.additional_params.push_back(::keymaster::TAG_AUTH_TOKEN, &auth_token, sizeof(hw_auth_token_t));
	}

	UpdateOperationResponse response(impl_->message_version());
	impl_->UpdateOperation(request, &response);

	if (response.error != KM_ERROR_OK) {
		if (response.error == BEANPOD_KM_INVALID_INPUT_LENGTH) {
			response.error = KM_ERROR_INVALID_INPUT_LENGTH;
			abort();
		}

		return kmError2ScopedAStatus(response.error);
	}
	if (response.input_consumed != request.input.buffer_size()) {
		return kmError2ScopedAStatus(KM_ERROR_UNKNOWN_ERROR);
	}

	*output = kmBuffer2vector(response.output);
	return ScopedAStatus::ok();
}

ScopedAStatus
BeanpodKeyMintOperation::finish(const optional<vector<uint8_t>>& input,
				const optional<vector<uint8_t>>& signature,
				const optional<HardwareAuthToken>& authToken,
				const optional<TimeStampToken>& /* timestampToken */,
				const optional<vector<uint8_t>>& /* confirmationToken */,
				vector<uint8_t>* output) {

	if (!output) {
		return ScopedAStatus(AStatus_fromServiceSpecificError(
			static_cast<int32_t>(ErrorCode::OUTPUT_PARAMETER_NULL)));
	}

	FinishOperationRequest request(impl_->message_version());
	request.op_handle = opHandle_;
	if (input) request.input.Reinitialize(input->data(), input->size());
	if (signature) request.signature.Reinitialize(signature->data(), signature->size());

	hw_auth_token_t auth_token;
	memset(&auth_token, 0, sizeof(hw_auth_token_t));
	if (authToken) {
		ALOGI("contain authtoken\n");
		auth_token.version = 0;
		auth_token.challenge = authToken->challenge;
		auth_token.user_id = authToken->userId;
		auth_token.authenticator_id = authToken->authenticatorId;
		auth_token.authenticator_type = static_cast<uint32_t>(authToken->authenticatorType);
		auth_token.timestamp = authToken->timestamp.milliSeconds;
		memcpy(auth_token.hmac, authToken->mac.data(), authToken->mac.size());
		SwapBytes((uint8_t*)&(auth_token.authenticator_type), sizeof(auth_token.authenticator_type));
		SwapBytes((uint8_t*)&(auth_token.timestamp), sizeof(auth_token.timestamp));
		request.additional_params.push_back(::keymaster::TAG_AUTH_TOKEN, &auth_token, sizeof(hw_auth_token_t));
	}

	FinishOperationResponse response(impl_->message_version());
	impl_->FinishOperation(request, &response);

	if (response.error != KM_ERROR_OK) {
		if (response.error == BEANPOD_KM_INVALID_INPUT_LENGTH) {
			response.error = KM_ERROR_INVALID_INPUT_LENGTH;
			abort();
		}

		opHandle_ = 0;
		return kmError2ScopedAStatus(response.error);
	}

	opHandle_ = 0;
	*output = kmBuffer2vector(response.output);
	return ScopedAStatus::ok();
}

ScopedAStatus BeanpodKeyMintOperation::abort() {
	AbortOperationRequest request(impl_->message_version());
	request.op_handle = opHandle_;

	AbortOperationResponse response(impl_->message_version());
	impl_->AbortOperation(request, &response);
	opHandle_ = 0;

	return kmError2ScopedAStatus(response.error);
}

}//namespace aidl::android::hardware::security::keymint
