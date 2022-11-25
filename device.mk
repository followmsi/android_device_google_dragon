#
# Copyright (C) 2015 The Android Open-Source Project
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
#

# There are three modes for TLK components, controlled by SECURE_OS_BUILD:
#
# "SECURE_OS_BUILD = tlk" will build everything from source.
# "SECURE_OS_BUILD = client_only" will build TLK client components from
#                    source but use a prebuilt version of the secure OS.
# "SECURE_OS_BUILD = false" will will use prebuilts for TLK and client
#                    components.

# Device was launched with M
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_m.mk)

$(call inherit-product, device/google/dragon/hidl/hidl.mk)

$(call inherit-product, device/google/dragon/permissions.mk)

$(call inherit-product, device/google/dragon/vendor/common-by-flags.mk) 

# Properties
$(call inherit-product, device/google/dragon/system_prop.mk)
$(call inherit-product, device/google/dragon/vendor_prop.mk)

# By default build TLK from source if it is available, otherwise use
# prebuilts.  To force using the prebuilt while having the source, set:
# SECURE_OS_BUILD=false
ifeq ($(wildcard vendor/nvidia/dragon-tlk/tlk),vendor/nvidia/dragon-tlk/tlk)
    SECURE_OS_BUILD ?= tlk
endif

# prebuilt kernel config
#
#ifeq ($(TARGET_PREBUILT_KERNEL),)
#LOCAL_KERNEL := device/google/dragon/Image.fit
#else
#LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
#endif
#
#PRODUCT_COPY_FILES := \
#    $(LOCAL_KERNEL):kernel \
#

LOCAL_FSTAB := $(LOCAL_PATH)/initfiles/fstab.dragon

TARGET_RECOVERY_FSTAB = $(LOCAL_FSTAB)

PRODUCT_BUILD_RECOVERY_IMAGE := true

PRODUCT_BUILD_BOOT_IMAGE := true

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/speakerdsp.ini:system/etc/cras/speakerdsp.ini \
    $(LOCAL_FSTAB):$(TARGET_COPY_OUT_RAMDISK)/fstab.dragon

# Ramdisk
PRODUCT_PACKAGES += \
    fstab.dragon \
    init.dragon.rc \
    init.dragon.usb.rc \
    init.comms.rc \
    init.recovery.dragon.rc \
    ueventd.dragon.rc \
    bt_loader \
    dump_bq25892 \
    init_regions \
    init_renderer \
    touchfwup \
    tune-thermal-gov \
    wifi_loader

PRODUCT_PACKAGES += \
    fs_config_files \
    fwtool

PRODUCT_COPY_FILES += \
    device/google/dragon/dragon-keypad.kl:system/usr/keylayout/dragon-keypad.kl

ifeq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_PACKAGES += \
    tinyplay \
    tinycap \
    tinymix
endif

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    $(LOCAL_PATH)/configs/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/configs/audio_policy_volumes_drc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes_drc.xml \
    $(LOCAL_PATH)/configs/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(LOCAL_PATH)/configs/mixer_paths_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_0.xml

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/configs/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/configs/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml

PRODUCT_COPY_FILES += \
    system/core/libprocessgroup/profiles/cgroups_28.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json \
    system/core/libprocessgroup/profiles/task_profiles_28.json:$(TARGET_COPY_OUT_VENDOR)/etc/task_profiles.json

# Bluetooth HAL
PRODUCT_PACKAGES += \
    libbt-vendor

# Copy dsp firmware to the vendor parition so it is available when hotwording
# starts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rt5677_elf_vad:vendor/firmware/rt5677_elf_vad

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/enctune.conf:system/etc/enctune.conf

PRODUCT_AAPT_CONFIG := normal large xlarge hdpi xhdpi xxhdpi
PRODUCT_AAPT_PREF_CONFIG := xhdpi

PRODUCT_CHARACTERISTICS := tablet,nosdcard

DEVICE_PACKAGE_OVERLAYS := \
    $(LOCAL_PATH)/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory

# Dexpreopt
PRODUCT_DEXPREOPT_SPEED_APPS += SystemUI

#TODO(dgreid) is this right?

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

# for audio
#TODO(dgreid) do we need libnvvisualizer?
PRODUCT_PACKAGES += \
    audio.primary.dragon \
    sound_trigger.primary.dragon \
    audio.bluetooth.default \
    audio.usb.default \
    audio.r_submix.default

# for keyboard key mappings
PRODUCT_PACKAGES += \
    DragonKeyboard

# Libshims
PRODUCT_PACKAGES += \
    camera.dragon_shim \
    libshim_sensors \
    libshims_postproc

# Wi-Fi country code setting
PRODUCT_PACKAGES += \
    DragonParts

# Face Unlock
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full

# HIDL
PRODUCT_ENFORCE_VINTF_MANIFEST_OVERRIDE := true

# TODO(dgreid) - Add back verity dependencies like flounder has.

$(call inherit-product, build/target/product/vboot.mk)

# only include verity on user builds
ifeq ($(TARGET_BUILD_VARIANT),user)
$(call inherit-product, build/target/product/verity.mk)
# including verity.mk automatically enabled boot signer which conflicts with
# vboot
PRODUCT_SUPPORTS_BOOT_SIGNER := false
PRODUCT_SUPPORTS_VERITY_FEC := false
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/platform/700b0600.sdhci/by-name/APP
PRODUCT_VENDOR_VERITY_PARTITION := /dev/block/platform/700b0600.sdhci/by-name/VNR
endif

# The following group is necessary to support building the NVIDIA vendor
# HALs and prebuilts.
BOARD_SUPPORT_NVOICE := true
BOARD_SUPPORT_NVAUDIOFX :=true
BOARD_USES_LIBDRM := true
NVRM_GPU_SUPPORT_NOUVEAU := 1
NV_GPU_USE_SYNC_FD := 1
USE_DRM_HWCOMPOSER := 1

ENABLE_LIBDRM := true
BOARD_GPU_DRIVERS := tegra
USE_XML_AUDIO_POLICY_CONF := 1
PRODUCT_PACKAGES += \
    f54test \
    libvulkan \
    hwcomposer.dragon \
    libdrm \
    rmi4update \
    rmihidtool

PRODUCT_COPY_FILES += \
    device/google/dragon/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt

PRODUCT_PACKAGES += \
    libprotobuf-cpp-lite-vendorcompat

# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    device/google/dragon/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    device/google/dragon/seccomp_policy/mediaextractor.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy

# Wifi
PRODUCT_PACKAGES += \
    hostapd \
    wificond \
    libwpa_client \
    wpa_supplicant \
    p2p_supplicant.conf \
    wpa_supplicant.conf \
    p2p_supplicant_overlay.conf \
    wpa_supplicant_overlay.conf

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH) \
    external/mesa3d

$(call inherit-product, vendor/google/dragon/dragon-vendor.mk)
