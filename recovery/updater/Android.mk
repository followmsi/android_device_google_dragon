ifneq (,$(findstring $(TARGET_DEVICE),dragon))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := librecovery_updater_dragon
LOCAL_SRC_FILES := \
    flash_ec.c \
    flash_mtd.c \
    flash_device.c \
    flash_file.cpp \
    vboot_interface.c \
    recovery_updater.cpp \
    update_fw.cpp
# For error_code.h (included by recovery_updater.cpp).
LOCAL_C_INCLUDES += bootable/recovery
# For vboot_struct.h
LOCAL_C_INCLUDES += external/vboot_reference/firmware/include
LOCAL_CFLAGS += -Werror
LOCAL_STATIC_LIBRARIES := \
    libedify

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := librecovery_ui_dragon
LOCAL_SRC_FILES := \
    flash_ec.c \
    flash_mtd.c \
    flash_device.c \
    flash_file.cpp \
    vboot_interface.c \
    recovery_ui.cpp
# For ui.h, common.h (included by recovery_ui.cpp).
LOCAL_C_INCLUDES += bootable/recovery
# For vboot_struct.h
LOCAL_C_INCLUDES += external/vboot_reference/firmware/include
LOCAL_CFLAGS += -Werror
LOCAL_STATIC_LIBRARIES := \
    libedify

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fwtool
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := flash_ec.c flash_mtd.c flash_device.c vboot_interface.c debug_ec.c flash_file.cpp fwtool.cpp update_fw.cpp
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CFLAGS += -Wno-unused-parameter -DUSE_LOGCAT
LOCAL_C_INCLUDES += bootable/recovery
# For vboot_struct.h
LOCAL_C_INCLUDES += external/vboot_reference/firmware/include
LOCAL_STATIC_LIBRARIES := \
    libedify

include $(BUILD_EXECUTABLE)

endif
