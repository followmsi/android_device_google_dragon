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

LOCAL_PATH := device/google/dragon

$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_m.mk)

$(call inherit-product, $(LOCAL_PATH)/hidl/hidl.mk)

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

TARGET_RECOVERY_FSTAB := $(LOCAL_PATH)/recovery.fstab

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    device/google/dragon

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/dump_bq25892.sh:$(TARGET_COPY_OUT_VENDOR)/bin/dump_bq25892.sh \
    $(LOCAL_PATH)/touchfwup.sh:$(TARGET_COPY_OUT_VENDOR)/bin/touchfwup.sh \
    $(LOCAL_PATH)/init.dragon.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.dragon.rc \
    $(LOCAL_PATH)/init.dragon.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.dragon.usb.rc \
    $(LOCAL_PATH)/init.recovery.dragon.rc:recovery/root/init.recovery.dragon.rc \
    $(LOCAL_PATH)/init_regions.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init_regions.sh \
    $(LOCAL_PATH)/tune-thermal-gov.sh:$(TARGET_COPY_OUT_VENDOR)/bin/tune-thermal-gov.sh \
    $(LOCAL_PATH)/ueventd.dragon.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc \
    $(LOCAL_PATH)/speakerdsp.ini:$(TARGET_COPY_OUT_VENDOR)/etc/cras/speakerdsp.ini \
    $(LOCAL_PATH)/bcmdhd.cal:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/bcmdhd.cal

ifeq ($(TARGET_BUILD_SYSTEM_ROOT_IMAGE),true)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/fstab.dragon:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.dragon \

else
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/fstab.initdragon:$(TARGET_COPY_OUT_RAMDISK)/fstab.dragon \
    $(LOCAL_PATH)/fstab.initdragon:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.dragon

endif

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
    $(LOCAL_PATH)/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/dragon-keypad.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/dragon-keypad.kl

# ifeq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_PACKAGES += \
    tinyplay \
    tinycap \
    tinymix
#endif

PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/hearing_aid_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/hearing_aid_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    $(LOCAL_PATH)/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio_policy_configuration_bluetooth_legacy_hal.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration_bluetooth_legacy_hal.xml \
    $(LOCAL_PATH)/audio_policy_volumes_drc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes_drc.xml \
    $(LOCAL_PATH)/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(LOCAL_PATH)/mixer_paths_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_0.xml

# A2DP offload enabled for compilation
AUDIO_FEATURE_ENABLED_A2DP_OFFLOAD := false

# Enable A2DP offload (run-time switch for system components)
PRODUCT_PROPERTY_OVERRIDES += \
   persist.bluetooth.a2dp_offload.enable=false

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_performance.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/media_profiles_V1_0.xml

#PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bluetooth/BCM4354_003.001.012.0443.0863.hcd:$(TARGET_COPY_OUT_VENDOR)/firmware/bcm4350c0.hcd \
    $(LOCAL_PATH)/bluetooth/bt_vendor.conf:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth/bt_vendor.conf	

$(call inherit-product, $(LOCAL_PATH)/permissions.mk)

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/enctune.conf:$(TARGET_COPY_OUT_VENDOR)/etc/enctune.conf

PRODUCT_COPY_FILES += \
    device/google/dragon/configs/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml

PRODUCT_AAPT_CONFIG := normal large xlarge
PRODUCT_AAPT_PREF_CONFIG := xhdpi
PRODUCT_AAPT_PREBUILT_DPI := xhdpi

PRODUCT_CHARACTERISTICS := tablet,nosdcard

DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay

PRODUCT_ENFORCE_RRO_TARGETS := \
    framework-res

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory

#TODO(dgreid) is this right?
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p-dev-wlan0

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

# Smaug does not support ION needed for Codec 2.0
PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.ccodec=0

# The default locale should be determined from VPD, not from build.prop.
PRODUCT_SYSTEM_PROPERTY_BLACKLIST = \
    ro.product.locale

# OEM Unlock reporting
PRODUCT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

#OMX
#PRODUCT_PROPERTY_OVERRIDES += \
    persist.media.treble_omx=false \
    media.stagefright_legacyencoder=treu \
    media.stagefright.less-secure=true
PRODUCT_PROPERTY_OVERRIDES += \
    persist.media.treble_omx=false

# OMX
#PRODUCT_PACKAGES += \
    libc2dcolorconvert \
    libextmedia_jni \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxQcelp13Enc \
    libOmxVdec \
    libOmxVenc \
    libstagefrighthw

# OMX
#PRODUCT_PACKAGES += \
    libnvmm_audio \
    libnvmm_msaudio \
    libnvmm_parser \
    libnvmm_writer \
    libnvmmlite_audio \
    libnvmmlite_image \
    libnvmmlite_msaudio \
    libnvmmlite_video \
    libnvomx \
    libnvomxilclient \
    libstagefrighthw

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

# set default USB configuration
PRODUCT_PROPERTY_OVERRIDES += \
    ro.adb.secure=0

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320 \
    ro.opengles.version=196610

# for audio
#TODO(dgreid) do we need libnvvisualizer?
PRODUCT_PACKAGES += \
    audio.primary.dragon \
    sound_trigger.primary.dragon \
    audio.a2dp.default \
    audio.bluetooth.default \
    audio.hearing_aid.default \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler \
    vkinfo

PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true \
    ro.bt.bdaddr_path=/sys/devices/700b0200.sdhci/mmc_host/mmc0/mmc0:0001/mmc0:0001:2/net/wlan0/address \
    ro.frp.pst=/dev/block/platform/700b0600.sdhci/by-name/PST
	
#Bluetooth
PRODUCT_PACKAGES += \
    libbt-vendor

# ro.product.first_api_level indicates the first api level the device has commercially launched on.
PRODUCT_SHIPPING_API_LEVEL := 28

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.vndk.version=28 \
    ro.product.first_api_level=28

# Set current VNDK version for GSI
PRODUCT_PROPERTY_OVERRIDES += \
    ro.gsi.vndk.version=28

# for keyboard key mappings
PRODUCT_PACKAGES += \
    DragonKeyboard

#PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/prebuilts/libcutils.so:$(TARGET_COPY_OUT_VENDOR)/lib/vndk-sp/libcutils.so \
    $(LOCAL_PATH)/prebuilts/libprocessgroup.so:$(TARGET_COPY_OUT_VENDOR)/lib/libprocessgroup.so \
    $(LOCAL_PATH)/prebuilts/libcameraservice.so:$(TARGET_COPY_OUT_VENDOR)/lib/libcameraservice.so

# Camera configurations
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/camera/external_camera_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/external_camera_config.xml

# DRM Mappings
PRODUCT_PROPERTY_OVERRIDES += \
    camera.flash_off=0 \
    drm.service.enabled=true \
    ro.com.widevine.cachesize=16777216 \
    ro.com.google.clientidbase=android-google

#PRODUCT_PROPERTY_OVERRIDES += \
    ro.lmk.kill_timeout_ms=100 \
    ro.lmk.use_minfree_levels=true \
    ro.lmk.low=1001 \
    ro.lmk.medium=800 \
    ro.lmk.critical=0 \
    ro.lmk.critical_upgrade=false \
    ro.lmk.upgrade_pressure=100 \
    ro.lmk.downgrade_pressure=100 \
    ro.lmk.kill_heaviest_task=true

# Face Unlock
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full

# VNDK
PRODUCT_PACKAGES += \
    vndk_package

# VNDK-SP
PRODUCT_PACKAGES += \
    vndk-sp

# Google Assistant
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opa.eligible_device=true

# Allows healthd to boot directly from charger mode rather than initiating a reboot.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1

#PRODUCT_VENDOR_PROPERTY_BLACKLIST := \
    ro.product.vendor.device \
    ro.product.vendor.model \
    ro.product.vendor.name \
    ro.vendor.build.fingerprint

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

# GSI vndk config for start legacy
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/vndk/ld.config.vndk_lite.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.vndk_lite.txt \
    $(LOCAL_PATH)/vndk/ld.config.29.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.29.txt \
    $(LOCAL_PATH)/vndk/ld.config.28.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.28.txt \
    $(LOCAL_PATH)/vndk/ld.config.27.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.27.txt \
    $(LOCAL_PATH)/vndk/ld.config.26.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.26.txt

#   $(LOCAL_PATH)/ld.config.vndk_lite.Legacy.txt:$(TARGET_COPY_OUT_VENDOR)/etc/ld.config.vndk_lite.txt \

PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.vulkan=tegra

PRODUCT_PROPERTY_OVERRIDES += \
    ro.bionic.ld.warning=0

$(call inherit-product-if-exists, hardware/nvidia/tegra132/tegra132.mk)
$(call inherit-product-if-exists, vendor/google/dragon/device-vendor.mk)
$(call inherit-product-if-exists, vendor/google/dragon-common/device-vendor.mk)
#$(call inherit-product-if-exists, vendor/google/dragon-drm/device-vendor.mk)
#$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/config/config-bcm.mk)

#PRODUCT_PACKAGES += \
    libshim_camera

ENABLE_LIBDRM := true
BOARD_GPU_DRIVERS := tegra
USE_XML_AUDIO_POLICY_CONF := 1
PRODUCT_PACKAGES += \
    f54test \
    hwcomposer.dragon \
    libdrm.vendor \
    rmi4update \
    rmihidtool

# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/seccomp_policy/mediaextractor.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy

$(call inherit-product-if-exists, vendor/nvidia/dragon/dragon-vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/gsi_keys.mk)
