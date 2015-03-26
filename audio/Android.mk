ifneq ($(filter tegra132_dragon,$(TARGET_BOARD_PLATFORM)),)
ifneq ($(filter dragon,$(TARGET_DEVICE)),)

MY_LOCAL_PATH := $(call my-dir)

include $(MY_LOCAL_PATH)/hal/Android.mk

endif
endif
