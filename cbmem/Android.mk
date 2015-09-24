ifneq (,$(findstring $(TARGET_DEVICE),dragon))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cbmem
LOCAL_MODULE_TAGS := debug
LOCAL_CLANG := false
LOCAL_SRC_FILES := cbmem.c
LOCAL_MULTILIB := 32
include $(BUILD_EXECUTABLE)

endif
