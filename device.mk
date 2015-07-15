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

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/google/dragon-kernel/Image.fit
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

LOCAL_FSTAB := $(LOCAL_PATH)/fstab.dragon

TARGET_RECOVERY_FSTAB = $(LOCAL_FSTAB)

PRODUCT_COPY_FILES := \
    $(LOCAL_KERNEL):kernel \
    $(LOCAL_PATH)/init.dragon.rc:root/init.dragon.rc \
    $(LOCAL_PATH)/init.dragon.usb.rc:root/init.dragon.usb.rc \
    $(LOCAL_PATH)/init.recovery.dragon.rc:root/init.recovery.dragon.rc \
    $(LOCAL_FSTAB):root/fstab.dragon \
    $(LOCAL_PATH)/ueventd.dragon.rc:root/ueventd.dragon.rc \
    $(LOCAL_PATH)/bcmdhd.cal:system/etc/wifi/bcmdhd.cal

PRODUCT_PACKAGES += \
    libwpa_client \
    hostapd \
    dhcpcd.conf \
    wpa_supplicant \
    wpa_supplicant.conf \
    fs_config_files \
    crash_collector

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/dragon_features.xml:system/etc/permissions/dragon_features.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:system/etc/permissions/android.hardware.opengles.aep.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio_policy.conf:system/etc/audio_policy.conf \
    $(LOCAL_PATH)/mixer_paths_0.xml:system/etc/mixer_paths_0.xml

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/media_codecs.xml:system/etc/media_codecs.xml \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bluetooth/BCM4354_003.001.012.0259.0512.hcd:$(TARGET_COPY_OUT_VENDOR)/firmware/bcm4350c0.hcd \
    $(LOCAL_PATH)/bluetooth/bt_vendor.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/bluetooth/bt_vendor.conf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/gps/gps.conf:system/etc/gps.conf \
    $(LOCAL_PATH)/gps/glgps:system/bin/glgps \
    $(LOCAL_PATH)/gps/gpsconfig.xml:system/etc/gpsconfig.xml \
    $(LOCAL_PATH)/gps/gps.dragon.so:system/lib64/hw/gps.dragon.so

PRODUCT_AAPT_CONFIG := normal large xlarge hdpi xhdpi xxhdpi
PRODUCT_AAPT_PREF_CONFIG := xhdpi

PRODUCT_CHARACTERISTICS := tablet,nosdcard

DEVICE_PACKAGE_OVERLAYS := \
    $(LOCAL_PATH)/overlay

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory

PRODUCT_PACKAGES += \
    power.dragon \
    lights.dragon

#TODO(dgreid) is this right?
PRODUCT_PROPERTY_OVERRIDES := \
    wifi.interface=wlan0

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

# set default USB configuration
# TODO(dgreid) - set secure back to 1
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp \
    ro.adb.secure=0

# for audio
#TODO(dgreid) do we need libnvvisualizer?
PRODUCT_PACKAGES += \
    audio.primary.dragon \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default

PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true

# Allows healthd to boot directly from charger mode rather than initiating a reboot.
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1

# TODO(dgreid) - Add back verity dependencies like flounder has.

$(call inherit-product, build/target/product/vboot.mk)
$(call inherit-product, build/target/product/verity.mk)
# including verity.mk automatically enabled boot signer which conflicts with
# vboot
PRODUCT_SUPPORTS_BOOT_SIGNER := false
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/platform/700b0600.sdhci/by-name/APP
PRODUCT_VENDOR_VERITY_PARTITION := /dev/block/platform/700b0600.sdhci/by-name/VNR

$(call inherit-product-if-exists, hardware/nvidia/tegra132/tegra132.mk)
$(call inherit-product-if-exists, vendor/google_devices/dragon/device-vendor.mk)
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4354/device-bcm.mk)

ENABLE_LIBDRM := true
BOARD_GPU_DRIVERS := tegra
PRODUCT_PACKAGES += \
	hwcomposer.drm \
	libdrm

$(call inherit-product-if-exists, vendor/nvidia/dragon/dragon-vendor.mk)
