# Copyright (C) 2015 The Android Open Source Project
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

ifeq ($(strip $(BOARD_USES_DRM_HWCOMPOSER)),true)

LOCAL_PATH := $(call my-dir)

common_drm_hwcomposer_cflags := \
    -Wall \
    -Werror \
    -Wno-unused-function \
    -Wno-unused-label \
    -Wno-unused-parameter \
    -Wno-unused-private-field \
    -Wno-unused-variable \

# =====================
# hwcomposer.dragon.so
# =====================
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdrm \
	libEGL \
	libGLESv2 \
	libhardware \
	liblog \
	libsync \
	libui \
	libutils

LOCAL_STATIC_LIBRARIES := libdrmhwc_utils

LOCAL_C_INCLUDES := \
	external/drm_gralloc \
	external/libdrm \
	external/libdrm/include/drm \
	system/core/include/utils \
	system/core/libsync \
	system/core/libsync/include \

LOCAL_SRC_FILES := \
	drmresources.cpp \
	drmcomposition.cpp \
	drmcompositor.cpp \
	drmconnector.cpp \
	drmcrtc.cpp \
	drmdisplaycomposition.cpp \
	drmdisplaycompositor.cpp \
	drmencoder.cpp \
	drmeventlistener.cpp \
	drmmode.cpp \
	drmplane.cpp \
	drmproperty.cpp \
	glworker.cpp \
	hwcutils.cpp \
	platform.cpp \
	separate_rects.cpp \
	virtualcompositorworker.cpp \
	vsyncworker.cpp

ifeq ($(TARGET_FORCES_DRM_HWC1),true)
LOCAL_SRC_FILES += hwcomposer.cpp
else
LOCAL_SRC_FILES += drmhwctwo.cpp
endif

LOCAL_CFLAGS := $(common_drm_hwcomposer_cflags)

LOCAL_CPPFLAGS += \
	-DHWC2_USE_CPP11 \
	-DHWC2_INCLUDE_STRINGIFICATION

ifeq ($(strip $(BOARD_DRM_HWCOMPOSER_BUFFER_IMPORTER)),nvidia-gralloc)
LOCAL_CPPFLAGS += -DUSE_NVIDIA_IMPORTER
LOCAL_SRC_FILES += platformnv.cpp
else
LOCAL_CPPFLAGS += -DUSE_DRM_GENERIC_IMPORTER
LOCAL_SRC_FILES += platformdrmgeneric.cpp
endif

LOCAL_MODULE := hwcomposer.dragon
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
LOCAL_VENDOR_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
