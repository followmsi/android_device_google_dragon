#
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
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE               := nvphsd.conf
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := nvphsd.foster.conf
LOCAL_ODM_MODULE           := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := nvphsd
LOCAL_SRC_FILES            := nvphsd.c
LOCAL_SHARED_LIBRARIES     := libc libdl libnvphsd
LOCAL_MODULE_CLASS         := EXECUTABLES
LOCAL_VENDOR_MODULE        := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE               := init.nvphsd_setup.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := init.nvphsd_setup.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := nvphsd.rc
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := nvphsd.rc
LOCAL_VENDOR_MODULE        := true
LOCAL_MODULE_RELATIVE_PATH := init
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := nvphsd_common.conf
LOCAL_MODULE_CLASS         := ETC
LOCAL_SRC_FILES            := nvphsd_common.conf
LOCAL_ODM_MODULE           := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE               := nvphsd_setup.sh
LOCAL_MODULE_CLASS         := EXECUTABLES
LOCAL_SRC_FILES            := nvphsd_setup.sh
LOCAL_VENDOR_MODULE        := true
include $(BUILD_PREBUILT)
