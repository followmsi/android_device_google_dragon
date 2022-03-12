# Copyright (C) 2020 The LineageOS Project
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

GCC_HOST_BINARIES:=$(wildcard prebuilts/gcc/linux-x86/host/x86_64-linux-*/x86_64-linux/bin)
LOCAL_SHARED_LIBRARIES += $(shell (basename -s .so `$(GCC_HOST_BINARIES)/objdump -x $(LOCAL_PATH)/$(LOCAL_SRC_FILES) 2>/dev/null |grep NEEDED` 2>/dev/null |grep -v ^NEEDED$ |tr '\n' ' '))
LOCAL_SHARED_LIBRARIES += $(shell (basename -s .so `$(GCC_HOST_BINARIES)/objdump -x $(LOCAL_PATH)/$(LOCAL_SRC_FILES_32) 2>/dev/null |grep NEEDED` 2>/dev/null |grep -v ^NEEDED$ |tr '\n' ' '))
LOCAL_SHARED_LIBRARIES += $(shell (basename -s .so `$(GCC_HOST_BINARIES)/objdump -x $(LOCAL_PATH)/$(LOCAL_SRC_FILES_64) 2>/dev/null |grep NEEDED` 2>/dev/null |grep -v ^NEEDED$ |tr '\n' ' '))

include $(BUILD_PREBUILT)
