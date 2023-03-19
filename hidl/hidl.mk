# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio@7.0-impl \
    android.hardware.audio.service.dragon \
    android.hardware.audio.effect@7.0-impl \
    android.hardware.soundtrigger@2.2-impl

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.1-service \
    android.hardware.bluetooth.audio@2.1-impl \
    libbt-vendor

# Camera
PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.4-impl \
    android.hardware.camera.provider@2.4-service \
    camera.device@1.0-impl \
    camera.device@3.2-impl \
    libcameraservice

# Display
PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@2.0-impl

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm-service.clearkey

# Gatekeeper
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-service.software

# Health
PRODUCT_PACKAGES += \
    android.hardware.health@2.0-impl \
    android.hardware.health@2.0-service.dragon

# Keymaster
PRODUCT_PACKAGES += \
    android.hardware.keymaster@4.0-service

# Lights
PRODUCT_PACKAGES += \
    android.hardware.light@2.0-service-nvidia

# Memtrack
PRODUCT_PACKAGES += \
    android.hardware.memtrack@1.0-impl

# Power
PRODUCT_PACKAGES += \
    vendor.nvidia.hardware.power@1.0-service

# PHS
PRODUCT_PACKAGES += \
    nvphsd.conf \
    init.nvphsd_setup.rc \
    nvphsd.rc \
    nvphsd_common.conf \
    nvphsd_setup.sh

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors@1.0-impl \
    android.hardware.sensors@1.0-service \
    sensors.dragon

# USB
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service.basic

# Configstore
PRODUCT_PACKAGES += \
    android.hardware.configstore@1.1-service

# VNDK
PRODUCT_PACKAGES += \
    vndk-sp

# HIDL
PRODUCT_PACKAGES += \
    libhidltransport \
    libhwbinder

# WiFi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    android.hardware.wifi.supplicant-V1-ndk
