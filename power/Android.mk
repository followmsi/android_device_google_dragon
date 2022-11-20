# Copyright (C) 2019 The LineageOS Project
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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.nvidia.hardware.power@1.0-service
LOCAL_INIT_RC := vendor.nvidia.hardware.power@1.0-service.rc
LOCAL_VINTF_FRAGMENTS := vendor.nvidia.hardware.power@1.0-service.xml

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    libhidlbase \
    liblog \
    libcutils \
    libutils \
    libdl \
    libexpat \
    vendor.nvidia.hardware.power@1.0

LOCAL_SRC_FILES := \
    service.cpp \
    Power.cpp \
    nvpowerhal.cpp \
    timeoutpoker.cpp \
    powerhal_parser.cpp \
    powerhal_utils.cpp \
    tegra_sata_hal.cpp \
    power_floor_t210.cpp

# T124+ uses set interactive. Revist if <= T114 is brought back
LOCAL_CFLAGS += -DPOWER_MODE_SET_INTERACTIVE
LOCAL_CFLAGS += -DTARGET_TEGRA_VERSION=$(TARGET_TEGRA_VERSION:t=)

LOCAL_CFLAGS += -DUSE_NVPHS
LOCAL_SHARED_LIBRARIES += libnvphs

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := nvidia
include $(BUILD_EXECUTABLE)
