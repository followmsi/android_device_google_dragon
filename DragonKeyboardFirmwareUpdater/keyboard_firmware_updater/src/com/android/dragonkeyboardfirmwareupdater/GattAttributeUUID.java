/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dragonkeyboardfirmwareupdater;

import java.util.UUID;

public class GattAttributeUUID {
    // Battery service
    public final static UUID UUID_BATTERY_SERVICE =
            UUID.fromString("0000180f-0000-1000-8000-00805f9b34fb");
    public final static UUID UUID_BATTERY_LEVEL_CHARACTERISTIC =
            UUID.fromString("00002A19-0000-1000-8000-00805f9b34fb");

    // Device information service
    public final static UUID UUID_DEVICE_INFORMATION_SERVICE =
            UUID.fromString("0000180a-0000-1000-8000-00805f9b34fb");
    public final static UUID UUID_DEVICE_INFORMATION_MANUFACTURER_CHARACTERISTIC =
            UUID.fromString("00002a29-0000-1000-8000-00805f9b34fb");
    public final static UUID UUID_DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC =
            UUID.fromString("00002a26-0000-1000-8000-00805f9b34fb");

    // DFU control point service
    public final static UUID UUID_DFU_SERVICE =
            UUID.fromString("00001530-1212-efde-1523-785feabcd123");
    public final static UUID UUID_DFU_CONTROL_POINT_CHARACTERISTIC =
            UUID.fromString("00001531-1212-efde-1523-785feabcd123");
    public final static UUID UUID_DFU_CONTROL_POINT_DESCRIPTOR =
            UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
}
