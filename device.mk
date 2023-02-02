#
# Copyright (C) 2020 The LineageOS Project
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

TARGET_REFERENCE_DEVICE  := dragon

PRODUCT_CHARACTERISTICS  := tablet,nosdcard
PRODUCT_AAPT_CONFIG      := normal large xlarge
PRODUCT_AAPT_PREF_CONFIG := xhdpi

$(call inherit-product, $(LOCAL_PATH)/hidl/hidl.mk)
$(call inherit-product, $(LOCAL_PATH)/permissions/permissions.mk)
$(call inherit-product, $(LOCAL_PATH)/vendor/common-by-flags.mk) 

$(call inherit-product, build/target/product/product_launched_with_m.mk)
$(call inherit-product, build/target/product/vboot.mk)

$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

$(call inherit-product, vendor/google/dragon/dragon-vendor.mk)

# Init related
PRODUCT_PACKAGES += \
    fstab.dragon \
    ueventd.dragon.rc \
    init.dragon.rc \
    init.usb.rc \
    init.recovery.rc \
    init.comms.rc \
    init.tlk.rc \
    bt_loader \
    dump_bq25892 \
    init_regions \
    init_renderer \
    touchfwup \
    tune-thermal-gov \
    wifi_loader
	
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/fstab.dragon:$(TARGET_COPY_OUT_RAMDISK)/fstab.dragon

# Art
# Speed profile services and wifi-service to reduce RAM and storage.
PRODUCT_SYSTEM_SERVER_COMPILER_FILTER := speed-profile

# Audio
PRODUCT_PACKAGES += \
    audio.primary.dragon \
    sound_trigger.primary.dragon \
    audio.bluetooth.default \
    audio.usb.default \
    audio.r_submix.default \
    audio_effects.xml \
    audio_policy_configuration.xml \
    audio_policy_volumes_drc.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/speakerdsp.ini:system/etc/cras/speakerdsp.ini

# Bluetooth
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/comms/vnd_dragon.txt:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth/bt_vendor.conf

# Cgroup and task_profiles
PRODUCT_COPY_FILES += \
    system/core/libprocessgroup/profiles/cgroups_28.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json \
    system/core/libprocessgroup/profiles/task_profiles_28.json:$(TARGET_COPY_OUT_VENDOR)/etc/task_profiles.json

# Config.fs
PRODUCT_PACKAGES += \
    fs_config_files

# Dexpreopt
PRODUCT_DEXPREOPT_SPEED_APPS += \
    SystemUI

# Display Device Config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/display_id_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/displayconfig/display_id_0.xml

# Dragon
PRODUCT_PACKAGES += \
    DragonParts \
    DragonKeyboard

# Graphics
BOARD_GPU_DRIVERS        := tegra
BOARD_USES_LIBDRM        := true
ENABLE_LIBDRM            := true
NVRM_GPU_SUPPORT_NOUVEAU := 1
NV_GPU_USE_SYNC_FD       := 1
USE_DRM_HWCOMPOSER       := 1

PRODUCT_PACKAGES += \
    f54test \
    libvulkan \
    hwcomposer.dragon \
    libdrm \
    rmi4update \
    rmihidtool

# HIDL
PRODUCT_ENFORCE_VINTF_MANIFEST_OVERRIDE := true

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl

# Libshims
PRODUCT_PACKAGES += \
    camera.dragon_shim \
    libshim_sensors \
    libshims_postproc
    
# Libprotobuf
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full \
    libprotobuf-cpp-lite-vendorcompat

# Locale
# The default locale should be determined from VPD, not from build.prop.
PRODUCT_SYSTEM_PROPERTY_BLACKLIST += \
    ro.product.locale

# Media
PRODUCT_PACKAGES += \
    media_codecs.xml \
    media_codecs_performance.xml \
    media_profiles_V1_0.xml \
    mixer_paths_0.xml \
    enctune.conf

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_ODM)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_ODM)/etc/media_codecs_google_video.xml
    
PRODUCT_COPY_FILES += \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml
    
# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay

# Public libraries
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/public.libraries.txt:$(TARGET_COPY_OUT_VENDOR)/etc/public.libraries.txt    

# Recovery
PRODUCT_PACKAGES += \
    fwtool
    
# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    device/google/dragon \
    external/mesa3d \
    hardware/nvidia

# USB
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory
    
# Vendor seccomp policy files for media components
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy \
    $(LOCAL_PATH)/seccomp_policy/mediaextractor.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediaextractor.policy

# VNDK
PRODUCT_PACKAGES += \
    vndk_package
    
# VNDK prebuilts
PRODUCT_COPY_FILES += \
    prebuilts/vndk/v29/arm64/arch-arm64-armv8-a/shared/vndk-core/libprotobuf-cpp-lite.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libprotobuf-cpp-lite-v29.so \
    prebuilts/vndk/v29/arm64/arch-arm-armv8-a/shared/vndk-core/libprotobuf-cpp-lite.so:$(TARGET_COPY_OUT_VENDOR)/lib/libprotobuf-cpp-lite-v29.so \
    prebuilts/vndk/v29/arm64/arch-arm64-armv8-a/shared/vndk-core/libprotobuf-cpp-full.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libprotobuf-cpp-full-v29.so \
    prebuilts/vndk/v29/arm64/arch-arm-armv8-a/shared/vndk-core/libprotobuf-cpp-full.so:$(TARGET_COPY_OUT_VENDOR)/lib/libprotobuf-cpp-full-v29.so

# WiFi
PRODUCT_PACKAGES += \
    hostapd \
    wificond \
    libwpa_client \
    wpa_supplicant \
    wpa_supplicant.conf \
    p2p_supplicant.conf \
    p2p_supplicant_overlay.conf \
    wpa_supplicant_overlay.conf \
    wifi_scan_config.conf
    
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.direct.interface=p2p-dev-wlan0 \
    wifi.direct.non-concurrent=true
