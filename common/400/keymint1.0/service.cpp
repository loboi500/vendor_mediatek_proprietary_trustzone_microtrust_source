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

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <BeanpodKeyMintDevice.h>
#include <BeanpodSecureClock.h>
#include <BeanpodSharedSecret.h>
#include <keymaster/soft_keymaster_logger.h>

#include "BeanpodRemotelyProvisionedComponent.h"

#include <BeanpodKeymaster.h>
#include <cutils/properties.h>

using aidl::android::hardware::security::keymint::BeanpodKeyMintDevice;
using aidl::android::hardware::security::keymint::BeanpodRemotelyProvisionedComponent;
using aidl::android::hardware::security::keymint::SecurityLevel;
using aidl::android::hardware::security::secureclock::BeanpodSecureClock;
using aidl::android::hardware::security::sharedsecret::BeanpodSharedSecret;

template <typename T, class... Args>
std::shared_ptr<T> addService(Args&&... args) {
	std::shared_ptr<T> ser = ndk::SharedRefBase::make<T>(std::forward<Args>(args)...);
	auto instanceName = std::string(T::descriptor) + "/default";
	LOG(INFO) << "adding keymint service instance: " << instanceName;
	binder_status_t status =
			AServiceManager_addService(ser->asBinder().get(), instanceName.c_str());
	CHECK(status == STATUS_OK);
	return ser;
}

int main() {
	// Zero threads seems like a useless pool, but below we'll join this thread to it, increasing
	// the pool size to 1.
	property_set("vendor.soter.teei.km.init", "start");
	ABinderProcess_setThreadPoolMaxThreadCount(0);

	auto bpKeymaster = new keymaster::BeanpodKeymaster();

	property_set("vendor.soter.teei.km.init", "add_service");
	// Add Keymint Service
	std::shared_ptr<BeanpodKeyMintDevice> keyMint =
			addService<BeanpodKeyMintDevice>(bpKeymaster);
	// Add Secure Clock Service
	addService<BeanpodSecureClock>(keyMint);
	// Add Shared Secret Service
	addService<BeanpodSharedSecret>(keyMint);
	// Add Remotely Provisioned Component Service
	addService<BeanpodRemotelyProvisionedComponent>(keyMint);
	property_set("vendor.soter.teei.km.init", "finish");
	ABinderProcess_joinThreadPool();
	return EXIT_FAILURE;  // should not reach
}
