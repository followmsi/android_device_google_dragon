# Copyright (C) 2016 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Boot animation
TARGET_SCREEN_HEIGHT := 2560
TARGET_SCREEN_WIDTH := 1800

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_tablet_wifionly.mk)

# Inherit from hardware-specific part of the product configuration
$(call inherit-product, device/google/dragon/aosp_dragon.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := dragon
PRODUCT_NAME := lineage_dragon
PRODUCT_BRAND := Google
PRODUCT_MODEL := Pixel C
PRODUCT_MANUFACTURER := google

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=dragon \
    BUILD_FINGERPRINT=google/ryu/dragon:8.1.0/OPM1.171019.022.A1/4609966:user/release-keys \
    PRIVATE_BUILD_DESC="ryu-user 8.1.0 OPM1.171019.022.A1 4609966 release-keys"

PRODUCT_PROPERTY_OVERRIDES += \
    ro.recents.grid=true


