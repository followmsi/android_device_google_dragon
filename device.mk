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

$(call inherit-product, device/google/dragon/hidl/hidl.mk)

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

ifeq ($(TARGET_PRODUCT), ryu_kasan)
LOCAL_FSTAB := $(LOCAL_PATH)/fstab.dragon.nocrypt
else
LOCAL_FSTAB := $(LOCAL_PATH)/fstab.dragon
endif

TARGET_RECOVERY_FSTAB = $(LOCAL_FSTAB)

PRODUCT_COPY_FILES := \
    $(LOCAL_PATH)/dump_bq25892.sh:system/bin/dump_bq25892.sh \
    $(LOCAL_PATH)/touchfwup.sh:system/bin/touchfwup.sh \
    $(LOCAL_PATH)/init.dragon.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.dragon.rc \
    $(LOCAL_PATH)/init.dragon.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.dragon.usb.rc \
    $(LOCAL_PATH)/init.recovery.dragon.rc:recovery/root/init.recovery.dragon.rc \
    $(LOCAL_PATH)/init_regions.sh:system/bin/init_regions.sh \
    $(LOCAL_PATH)/tune-thermal-gov.sh:system/bin/tune-thermal-gov.sh \
    $(LOCAL_PATH)/ueventd.dragon.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc \
    $(LOCAL_PATH)/speakerdsp.ini:system/etc/cras/speakerdsp.ini \
    $(LOCAL_PATH)/bcmdhd.cal:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/bcmdhd.cal \
    $(LOCAL_FSTAB):$(TARGET_COPY_OUT_RAMDISK)/fstab.dragon \
    $(LOCAL_FSTAB):$(TARGET_COPY_OUT_VENDOR)/etc/fstab.dragon

PRODUCT_PACKAGES += \
    libwpa_client \
    hostapd \
    wificond \
    wifilogd \
    wpa_supplicant \
    wpa_supplicant.conf \
    libwifikeystorehal \
    fs_config_files \
    fwtool

PRODUCT_COPY_FILES += \
    device/google/dragon/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    device/google/dragon/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf

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
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    $(LOCAL_PATH)/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio_policy_volumes_drc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes_drc.xml \
    $(LOCAL_PATH)/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(LOCAL_PATH)/mixer_paths_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_0.xml

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media_codecs.xml:system/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_performance.xml:system/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/media_profiles.xml:system/etc/media_profiles.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bluetooth/BCM4354_003.001.012.0443.0863.hcd:$(TARGET_COPY_OUT_VENDOR)/firmware/bcm4350c0.hcd \
    $(LOCAL_PATH)/bluetooth/bt_vendor.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/bluetooth/bt_vendor.conf

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

#TODO(dgreid) is this right?
PRODUCT_PROPERTY_OVERRIDES := \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p-dev-wlan0 \
    ro.hwui.texture_cache_size=86 \
    ro.hwui.layer_cache_size=56 \
    ro.hwui.r_buffer_cache_size=8 \
    ro.hwui.path_cache_size=40 \
    ro.hwui.gradient_cache_size=1 \
    ro.hwui.drop_shadow_cache_size=6 \
    ro.hwui.texture_cache_flushrate=0.4 \
    ro.hwui.text_small_cache_width=1024 \
    ro.hwui.text_small_cache_height=1024 \
    ro.hwui.text_large_cache_width=2048 \
    ro.hwui.text_large_cache_height=1024 \
    ro.hwui.disable_scissor_opt=true \
    ro.recents.grid=true

# mobile data provision prop
PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.android.prov_mobiledata=false

# facelock props
PRODUCT_PROPERTY_OVERRIDES += \
    ro.facelock.black_timeout=700 \
    ro.facelock.det_timeout=2500 \
    ro.facelock.rec_timeout=3500 \
    ro.facelock.est_max_time=500

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.media_vol_steps=25

# Cast
PRODUCT_PROPERTY_OVERRIDES += \
    persist.debug.wfd.enable=1

# The default locale should be determined from VPD, not from build.prop.
PRODUCT_SYSTEM_PROPERTY_BLACKLIST := \
    ro.product.locale

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

# set default USB configuration
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp,adb \
    ro.adb.secure=0 \
    ro.sf.lcd_density=320 \
    ro.opengles.version=196610

# Update the recovery image only if the option is enabled
# under Developer options
# by default, do not update the recovery image
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.recovery_update=false

# for audio
#TODO(dgreid) do we need libnvvisualizer?
PRODUCT_PACKAGES += \
    audio.primary.dragon \
    sound_trigger.primary.dragon \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default

PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true \
    ro.frp.pst=/dev/block/platform/700b0600.sdhci/by-name/PST

# ro.product.first_api_level indicates the first api level the device has commercially launched on.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.first_api_level=23

# for keyboard key mappings
PRODUCT_PACKAGES += \
    DragonKeyboard

# DRM Mappings
PRODUCT_PROPERTY_OVERRIDES += \
    camera.flash_off=0 \
    drm.service.enabled=true \
    ro.com.widevine.cachesize=16777216

# P2P0 Concurrency
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.direct.non-concurrent=true

# Face Unlock
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full

# Google Assistant
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opa.eligible_device=true

# Allows healthd to boot directly from charger mode rather than initiating a reboot.
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1

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

PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.vulkan=tegra

PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.ccodec=0

PRODUCT_PROPERTY_OVERRIDES += \
    ro.bionic.ld.warning=0

$(call inherit-product-if-exists, hardware/nvidia/tegra132/tegra132.mk)
$(call inherit-product-if-exists, vendor/google/dragon/device-vendor.mk)
$(call inherit-product-if-exists, vendor/google/dragon-common/device-vendor.mk)
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/config/config-bcm.mk)

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

# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    device/google/dragon/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    device/google/dragon/seccomp_policy/mediaextractor.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy

$(call inherit-product-if-exists, vendor/nvidia/dragon/dragon-vendor.mk)
