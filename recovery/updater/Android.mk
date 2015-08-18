ifneq (,$(findstring $(TARGET_DEVICE),dragon))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := librecovery_updater_dragon
LOCAL_SRC_FILES := flash_ec.cpp flash_mtd.cpp flash_file.cpp flash_device.cpp vboot_interface.cpp update_fw.cpp recovery_updater.cpp
LOCAL_C_INCLUDES += bootable/recovery
# For vboot_struct.h
LOCAL_C_INCLUDES += external/vboot_reference/firmware/include

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := fwtool
LOCAL_MODULE_TAGS := debug
LOCAL_CLANG := true
LOCAL_SRC_FILES := flash_ec.cpp flash_mtd.cpp flash_file.cpp flash_device.cpp vboot_interface.cpp update_fw.cpp debug_ec.cpp fwtool.cpp
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_C_INCLUDES += bootable/recovery
# For vboot_struct.h
LOCAL_C_INCLUDES += external/vboot_reference/firmware/include

include $(BUILD_EXECUTABLE)

endif
