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

# Use the non-open-source parts, if they're present
-include vendor/google_devices/dragon/BoardConfigVendor.mk
# Build a separate vendor.img
TARGET_COPY_OUT_VENDOR := vendor

TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := cortex-a53

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv7-a-neon
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a7

# Disable emulator for "make dist" until there is a 64-bit qemu kernel
BUILD_EMULATOR := false

TARGET_BOARD_PLATFORM := tegra210_dragon
TARGET_BOARD_INFO_FILE ?= device/google/dragon/board-info.txt

TARGET_BOOTLOADER_BOARD_NAME := dragon
TARGET_RELEASETOOLS_EXTENSIONS := device/google/dragon

USE_OPENGL_RENDERER := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 2
BOARD_DISABLE_TRIPLE_BUFFERED_DISPLAY_SURFACES := true
BOARD_USES_DRM_HWCOMPOSER := true
BOARD_DRM_HWCOMPOSER_BUFFER_IMPORTER := nvidia-gralloc
BOARD_USES_LIBDRM := true
TARGET_RECOVERY_PIXEL_FORMAT := BGRA_8888

PRESENT_TIME_OFFSET_FROM_VSYNC_NS := 0
VSYNC_EVENT_PHASE_OFFSET_NS := 7500000
SF_VSYNC_EVENT_PHASE_OFFSET_NS := 5000000

TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 3758096384
BOARD_CACHEIMAGE_PARTITION_SIZE := 419430400
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 4096

BOARD_CHARGER_DISABLE_INIT_BLANK := true
BOARD_USES_GENERIC_INVENSENSE := false

# RenderScript
OVERRIDE_RS_DRIVER := libnvRSDriver.so
BOARD_OVERRIDE_RS_CPU_VARIANT_32 := cortex-a15
BOARD_OVERRIDE_RS_CPU_VARIANT_64 := cortex-a57

BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_ALSA_AUDIO := true

BOARD_VENDOR_USE_SENSOR_HAL := sensor_hub

TARGET_USES_64_BIT_BCMDHD := true
TARGET_USES_64_BIT_BINDER := true

BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 1

#TARGET_SUPPORTS_64_BIT_APPS := true

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/google/dragon/bluetooth
BOARD_HAVE_BLUETOOTH_BCM := true

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

# Enable dex-preoptimization to speed up first boot sequence
ifeq ($(HOST_OS),linux)
  ifeq ($(TARGET_BUILD_VARIANT),user)
    ifeq ($(WITH_DEXPREOPT),)
      WITH_DEXPREOPT := true
    endif
  endif
endif

ART_USE_HSPACE_COMPACT=true

# let charger mode enter suspend
BOARD_CHARGER_ENABLE_SUSPEND := true

BOARD_SEPOLICY_DIRS += device/google/dragon/sepolicy
BOARD_SECCOMP_POLICY += device/google/dragon/seccomp

-include vendor/cm/sepolicy/sepolicy.mk

# add firmware update to the updater binary
TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_dragon
TARGET_RECOVERY_UPDATER_EXTRA_LIBS +=
TARGET_RECOVERY_UI_LIB := librecovery_ui_dragon

ifeq ($(SECURE_OS_BUILD),tlk)
  BOARD_SUPPORT_ROLLBACK_PROTECTION := true
endif

BOARD_HAL_STATIC_LIBRARIES := libdumpstate.dragon libhealthd.dragon

# TWRP settings
RECOVERY_VARIANT := twrp
TW_THEME := landscape_hdpi
TW_NO_SCREEN_TIMEOUT := true
TW_INCLUDE_MTP := true
BOARD_HAS_NO_REAL_SDCARD := true
RECOVERY_SDCARD_ON_DATA := true
TW_BRIGHTNESS_PATH := /sys/class/backlight/lpm102a188a-backlight/brightness
TW_CUSTOM_BATTERY_PATH := /sys/class/power_supply/bq27742-0
TW_INCLUDE_CRYPTO := true
TARGET_RECOVERY_DEVICE_MODULES := fwtool
TW_INCLUDE_NTFS_3G := true
TW_EXTRA_LANGUAGES := true

