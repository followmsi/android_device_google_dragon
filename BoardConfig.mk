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

BUILD_BROKEN_DUP_RULES := true
SELINUX_IGNORE_NEVERALLOWS := true
BUILD_BROKEN_MISSING_REQUIRED_MODULES := true
BUILD_BROKEN_ELF_PREBUILT_PRODUCT_COPY_FILES := true

# Architecture
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_SMP := true
TARGET_CPU_VARIANT := cortex-a53
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_VARIANT := cortex-a53.a57
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi

# Board
TARGET_TEGRA_VERSION               := t210
TARGET_BOARD_PLATFORM              := tegra
TARGET_NO_BOOTLOADER               := true

BOARD_FLASH_BLOCK_SIZE             := 4096
BOARD_BOOTIMAGE_PARTITION_SIZE     := 33554432
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 26767360
BOARD_CACHEIMAGE_PARTITION_SIZE    := 419430400
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 3758096384
BOARD_VENDORIMAGE_PARTITION_SIZE   := 318767104
TARGET_USERIMAGES_USE_EXT4         := true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE  := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_COPY_OUT_VENDOR             := vendor

# Bootloader versions
TARGET_BOARD_INFO_FILE := device/google/dragon/board-info.txt

# Manifest
DEVICE_MANIFEST_FILE := device/google/dragon/hidl/manifest.xml
DEVICE_MATRIX_FILE   := device/google/dragon/hidl/compatibility_matrix.xml

# Kernel
BOARD_KERNEL_BASE           := 0x10000000
BOARD_KERNEL_PAGESIZE       := 2048
TARGET_KERNEL_SOURCE        := kernel/google/tegra
TARGET_KERNEL_CONFIG        := lineageos_dragon_defconfig
TARGET_LINUX_KERNEL_VERSION := 3.18
BOARD_KERNEL_IMAGE_NAME     := Image.fit
TARGET_KERNEL_CLANG_COMPILE := false

# Audio
TARGET_EXCLUDES_AUDIOFX  := true

# Bluetooth
BOARD_CUSTOM_BT_CONFIG := device/google/dragon/comms/vnd_dragon.txt
BOARD_HAVE_BLUETOOTH_BCM := true

# Camera
USE_DEVICE_SPECIFIC_CAMERA := true

# Display
TARGET_SCREEN_DENSITY := 320

# Graphics
USE_OPENGL_RENDERER                            := true
BOARD_DISABLE_TRIPLE_BUFFERED_DISPLAY_SURFACES := true
BOARD_USES_DRM_HWCOMPOSER                      := true
BOARD_DRM_HWCOMPOSER_BUFFER_IMPORTER           := nvidia-gralloc
BOARD_USES_LIBDRM                              := true
TARGET_RECOVERY_PIXEL_FORMAT                   := BGRA_8888
TARGET_USES_HWC1 := true                       := true

# Permissions
TARGET_FS_CONFIG_GEN += device/google/dragon/config.fs

# Properties
TARGET_VENDOR_PROP := device/google/dragon/vendor.prop

# Recovery
TARGET_RECOVERY_FSTAB        := device/google/dragon/rootdir/fstab.dragon
TARGET_RECOVERY_PIXEL_FORMAT := RGBX_8888

# Security Patch Level
VENDOR_SECURITY_PATCH := 2019-06-05

# Sepolicy
BOARD_VENDOR_SEPOLICY_DIRS += device/google/dragon/sepolicy

# Shims
TARGET_LD_SHIM_LIBS := \
    /vendor/lib/hw/camera.dragon.so|camera.dragon_shim.so \
    /vendor/lib/hw/camera.dragon.so|libshims_postproc.so \
    /vendor/lib/libnvmm_camera_v3.so|libshim_sensors.so

# Wifi
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_bcmdhd
BOARD_WLAN_DEVICE                := bcmdhd
WIFI_DRIVER_FW_PATH_PARAM        := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA          := "/vendor/firmware/fw_bcmdhd.bin"
WIFI_DRIVER_FW_PATH_AP           := "/vendor/firmware/fw_bcmdhd_apsta.bin"
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true
