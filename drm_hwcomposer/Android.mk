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

__this_dir := $(call my-dir)

# =====================
# libdrmhwc_sync.a
# =====================
include $(CLEAR_VARS)

LOCAL_PATH := system/core/libsync

LOCAL_SRC_FILES := sync.c

LOCAL_CFLAGS := -Wno-unused-variable

LOCAL_MODULE := libdrmhwc_sync_dragon
LOCAL_VENDOR_MODULE := true

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH) $(LOCAL_PATH)/include

include $(BUILD_STATIC_LIBRARY)

# =====================
# libdrmhwc_utils.a
# =====================
include $(CLEAR_VARS)
 
LOCAL_PATH := $(__this_dir)

LOCAL_SRC_FILES := worker.cpp
 
LOCAL_MODULE := libdrmhwc_utils_dragon
LOCAL_VENDOR_MODULE := true

include $(BUILD_STATIC_LIBRARY)

# =====================
# hwcomposer.drm.so
# =====================
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdrm \
	libEGL \
	libGLESv2 \
	libhardware \
	liblog \
	libui \
	libutils

LOCAL_STATIC_LIBRARIES := libdrmhwc_utils_dragon libdrmhwc_sync_dragon

LOCAL_C_INCLUDES := \
        external/drm_gralloc \
	external/libdrm \
	external/libdrm/include/drm \
	system/core/include/utils \

LOCAL_SRC_FILES := \
	autolock.cpp \
	drmresources.cpp \
	drmcomposition.cpp \
	drmcompositor.cpp \
	drmcompositorworker.cpp \
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
	hwcomposer.cpp \
        hwcutils.cpp \
        platform.cpp \
        platformdrmgeneric.cpp \
        platformnv.cpp \
	separate_rects.cpp \
	virtualcompositorworker.cpp \
	vsyncworker.cpp

ifeq ($(strip $(BOARD_DRM_HWCOMPOSER_BUFFER_IMPORTER)),nvidia-gralloc)
LOCAL_CPPFLAGS += -DUSE_NVIDIA_IMPORTER
else
LOCAL_CPPFLAGS += -DUSE_DRM_GENERIC_IMPORTER
endif

LOCAL_MODULE := hwcomposer.dragon
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)
LOCAL_VENDOR_MODULE := true
LOCAL_HEADER_LIBRARIES += libhardware_headers

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
