LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE        := fstab.dragon
LOCAL_MODULE_CLASS  := ETC
LOCAL_SRC_FILES     := fstab.dragon
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := init.comms.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := init.comms.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := init.dragon.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := init.dragon.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := init.dragon.usb.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := init.dragon.usb.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := init.recovery.dragon.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := init.recovery.dragon.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := ueventd.dragon.rc
LOCAL_MODULE_STEM   := ueventd.rc
LOCAL_MODULE_CLASS  := ETC
LOCAL_SRC_FILES     := ueventd.dragon.rc
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := bt_loader
LOCAL_SRC_FILES     := bt_loader.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_MODULE_OWNER  := nvidia
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := dump_bq25892
LOCAL_SRC_FILES     := dump_bq25892.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := init_regions
LOCAL_SRC_FILES     := init_regions.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_MODULE_OWNER  := nvidia
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := init_renderer
LOCAL_SRC_FILES     := init_renderer.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := touchfwup
LOCAL_SRC_FILES     := touchfwup.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := tune-thermal-gov
LOCAL_SRC_FILES     := tune-thermal-gov.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := wifi_loader
LOCAL_SRC_FILES     := wifi_loader.sh
LOCAL_MODULE_SUFFIX := .sh
LOCAL_MODULE_CLASS  := EXECUTABLES
LOCAL_MODULE_OWNER  := nvidia
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

