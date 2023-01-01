/*
 **
 ** Copyright 2018, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 **
 ** Modifications: Copyright (c) 2020 MICROTRUST Incorporated.
 */

#define LOG_TAG "beanpodkeymaster4.1"

#include <authorization_set.h>
#include <cutils/log.h>
#include <keymaster/android_keymaster.h>
#include <keymaster/android_keymaster_messages.h>
#include <BeanpodKeymaster41Device.h>
#include <hardware/hw_auth_token.h>
#include <beanpod_keymaster_ipc.h>


using ::keymaster::V4_0::ng::hidlKeyParams2Km;

namespace keymaster {
namespace V4_1 {

namespace {
int ver = 3;

inline V41ErrorCode legacy_enum_conversion(const keymaster_error_t value) {
	return static_cast<V41ErrorCode>(value);
}

}  // namespace

BeanpodKeymaster41Device::~BeanpodKeymaster41Device() {}

Return<V41ErrorCode>
BeanpodKeymaster41Device::deviceLocked(bool passwordOnly,
                                       const VerificationToken& verificationToken) {
	DeviceLockedResponse response(ver);
	keymaster::VerificationToken serializableToken;
	serializableToken.challenge = verificationToken.challenge;
	serializableToken.timestamp = verificationToken.timestamp;
	serializableToken.parameters_verified.Reinitialize(
	        hidlKeyParams2Km(verificationToken.parametersVerified));
	serializableToken.security_level =
	        static_cast<keymaster_security_level_t>(verificationToken.securityLevel);
	serializableToken.mac =
	        KeymasterBlob(verificationToken.mac.data(), verificationToken.mac.size());

	impl_->deviceLocked(DeviceLockedRequest((ver), passwordOnly, std::move(serializableToken)), &response);
	return legacy_enum_conversion(response.error);
}

Return<V41ErrorCode> BeanpodKeymaster41Device::earlyBootEnded() {
	EarlyBootEndedResponse response(ver);
	impl_->earlyBootEnded(&response);
	return legacy_enum_conversion(response.error);
}

}  // namespace V4_1
}  // namespace keymaster
