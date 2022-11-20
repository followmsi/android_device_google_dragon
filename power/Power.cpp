/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2020 The LineageOS Project
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

#define LOG_TAG "vendor.nvidia.hardware.power@1.0-service"

// #define LOG_NDEBUG 0

#define FOSTER_E_HDD    "/dev/block/sda"
#define HDD_STANDBY_TIMEOUT     60

#include <android/log.h>
#include <utils/Log.h>
#include "Power.h"
#include "tegra_sata_hal.h"

namespace vendor {
namespace nvidia {
namespace hardware {
namespace power {
namespace V1_0 {
namespace implementation {

using ::android::hardware::power::V1_0::Feature;
using ::android::hardware::power::V1_0::PowerHint;
using ::android::hardware::power::V1_0::PowerStatePlatformSleepState;
using ::android::hardware::power::V1_0::Status;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::power::V1_0::Feature;
using ::vendor::nvidia::hardware::power::V1_0::ExtPowerHint;

Power::Power() {
    pInfo = new powerhal_info();

    pInfo->input_devs.push_back({-1, "touch\n"});
    pInfo->input_devs.push_back({-1, "raydium_ts\n"});

    pInfo->no_sclk_boost = true;

    common_power_init(pInfo);
}

// Methods from ::vendor::nvidia::hardware::power::V1_0::IPower follow.
Return<void> Power::powerHintExt(ExtPowerHint hint, const hidl_vec<int32_t>& data) {
    common_power_hint(pInfo, hint, static_cast<const void*>(data.data()));
    return Void();
}


// Methods from ::android::hardware::power::V1_0::IPower follow.
Return<void> Power::setInteractive(bool interactive)  {
    common_power_set_interactive(pInfo, interactive ? 1 : 0);

#if TARGET_TEGRA_VERSION == 210
    set_power_level_floor(interactive);
#endif

    if (!access(FOSTER_E_HDD, F_OK)) {
        /*
        * Turn-off Foster HDD at display off
        */
        ALOGI("HAL: Display is %s, set HDD to %s standby mode.", interactive?"on":"off", interactive?"disable":"enter");
        int error = 0;
        if (interactive) {
            error = hdd_disable_standby_timer();
            if (error)
                ALOGE("HAL: Failed to set standby timer, error: %d", error);
        }
        else {
            error = hdd_set_standby_timer(HDD_STANDBY_TIMEOUT);
            if (error)
                ALOGE("HAL: Failed to set standby timer, error: %d", error);
        }
    }

    return Void();
}

Return<void> Power::powerHint(PowerHint hint, int32_t data) {
    common_power_hint(pInfo, static_cast<ExtPowerHint>(hint), &data);
    return Void();
}

Return<void> Power::setFeature(Feature feature, __attribute__ ((unused)) bool activate)  {
    switch (static_cast<feature_t>(feature)) {
    case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
        ALOGW("Double tap to wake is not supported\n");
        break;
    default:
        ALOGW("Error setting the feature, it doesn't exist %d\n", feature);
        break;
    }

    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb) {
    hidl_vec<PowerStatePlatformSleepState> states;
    states.resize(0);
    _hidl_cb(states, Status::SUCCESS);
    return Void();
}

status_t Power::registerAsSystemService() {
    status_t ret = 0;

    ret = IPower::registerAsService();
    if (ret != 0) {
        ALOGE("Failed to register IPower (%d)", ret);
        goto fail;
    } else {
        ALOGI("Successfully registered IPower");
    }

fail:
    return ret;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace power
}  // namespace hardware
}  // namespace nvidia
}  // namespace vendor
