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

$(shell mkdir -p out/target/product/dragon/root/vendor/firmware)

# Build a separate vendor.img
TARGET_COPY_OUT_VENDOR := vendor

TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := cortex-a53
TARGET_CPU_VARIANT_RUNTIME := cortex-a53

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a53
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a53

BOARD_KERNEL_BASE := 0x10000000
BOARD_KERNEL_PAGESIZE := 2048
TARGET_KERNEL_SOURCE := kernel/google/tegra
TARGET_KERNEL_CONFIG := lineageos_dragon_defconfig
TARGET_LINUX_KERNEL_VERSION := 3.18
TARGET_KERNEL_ARCH := arm64
TARGET_KERNEL_HEADER_ARCH := arm64
BOARD_KERNEL_IMAGE_NAME := Image.fit

# Disable emulator for "make dist" until there is a 64-bit qemu kernel
BUILD_EMULATOR := false

TARGET_BOARD_PLATFORM := tegra210_dragon
TARGET_BOARD_INFO_FILE ?= device/google/dragon/board-info.txt

TARGET_BOOTLOADER_BOARD_NAME := dragon
TARGET_RELEASETOOLS_EXTENSIONS := device/google/dragon

TARGET_SCREEN_DENSITY := 320
USE_OPENGL_RENDERER := true
BOARD_DISABLE_TRIPLE_BUFFERED_DISPLAY_SURFACES := true
BOARD_USES_DRM_HWCOMPOSER := true
BOARD_DRM_HWCOMPOSER_BUFFER_IMPORTER := nvidia-gralloc
BOARD_USES_LIBDRM := true
TARGET_RECOVERY_PIXEL_FORMAT := BGRA_8888
#TARGET_USES_HWC2 := true
TARGET_FORCES_DRM_HWC1 := true

TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USES_MKE2FS := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 3758096384
BOARD_CACHEIMAGE_PARTITION_SIZE := 419430400
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_VENDORIMAGE_PARTITION_SIZE := 268435456
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 4096

BOARD_BUILD_SYSTEM_ROOT_IMAGE := false

BUILD_BROKEN_DUP_RULES := true

BOARD_CHARGER_DISABLE_INIT_BLANK := true
BOARD_USES_GENERIC_INVENSENSE := false

BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO := true

BOARD_VENDOR_USE_SENSOR_HAL := sensor_hub

TARGET_FS_CONFIG_GEN += device/google/dragon/config.fs

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/google/dragon/bluetooth
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_CUSTOM_BT_CONFIG := device/google/dragon/bluetooth/vnd_dragon.txt

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
WPA_SUPPLICANT_VERSION      := VER_0_8_X
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_bcmdhd
BOARD_WLAN_DEVICE           := bcmdhd
WIFI_DRIVER_FW_PATH_PARAM   := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA     := "/vendor/firmware/fw_bcmdhd.bin"
WIFI_DRIVER_FW_PATH_AP      := "/vendor/firmware/fw_bcmdhd_apsta.bin"
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true

# Enable dex-preoptimization to speed up first boot sequence
ifeq ($(HOST_OS),linux)
  ifeq ($(TARGET_BUILD_VARIANT),userdebug)
    ifeq ($(WITH_DEXPREOPT),)
      WITH_DEXPREOPT := true
    endif
  endif
endif

ART_USE_HSPACE_COMPACT=true

# Security Patch Level
VENDOR_SECURITY_PATCH := 2019-06-05

BOARD_SEPOLICY_DIRS += device/google/dragon/sepolicy

# add firmware update to the updater binary
TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_dragon
TARGET_RECOVERY_UPDATER_EXTRA_LIBS +=
#TARGET_RECOVERY_UI_LIB := librecovery_ui_dragon

ifeq ($(SECURE_OS_BUILD),tlk)
  BOARD_SUPPORT_ROLLBACK_PROTECTION := true
endif

# Enable real time lockscreen charging current values
BOARD_GLOBAL_CFLAGS += -DBATTERY_REAL_INFO

#BOARD_HAL_STATIC_LIBRARIES := libhealthd.dragon

WITH_LINEAGE_CHARGER := false

# Shims
TARGET_LD_SHIM_LIBS += \
    /vendor/lib/hw/camera.dragon.so|libshim_camera.so

# Testing related defines
BOARD_PERFSETUP_SCRIPT := platform_testing/scripts/perf-setup/dragon-setup.sh

# Vendor Interface Manifest
DEVICE_MANIFEST_FILE := device/google/dragon/hidl/manifest.xml
DEVICE_MATRIX_FILE := device/google/dragon/hidl/compatibility_matrix.xml

TARGET_FLATTEN_APEX := true
