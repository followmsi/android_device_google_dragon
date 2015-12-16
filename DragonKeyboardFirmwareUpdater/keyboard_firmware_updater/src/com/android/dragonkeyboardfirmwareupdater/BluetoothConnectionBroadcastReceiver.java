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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.UserManager;
import android.util.Log;

public class BluetoothConnectionBroadcastReceiver extends BroadcastReceiver {
    private final static String TAG = BluetoothConnectionBroadcastReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {

        if (!BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED.equals(intent.getAction())) return;

        Bundle extras = intent.getExtras();
        final BluetoothDevice device = extras.getParcelable(BluetoothDevice.EXTRA_DEVICE);
        final int deviceConnectionState = extras.getInt(BluetoothAdapter.EXTRA_CONNECTION_STATE);

        Log.d(TAG, "Receive action: " + intent.getAction() + " device: " + device.getName()
                + "[" + device.getAddress() + "], state: " + deviceConnectionState);

        // Only start the service when there is a Pixel C Keyboard connected.
        if (device == null || deviceConnectionState != BluetoothAdapter.STATE_CONNECTED ||
                !context.getString(R.string.target_keyboard_name).equals(device.getName())) {
            return;
        }

        Log.d(TAG, "Start Pixel C Keyboard Firmware Updater service");

        Intent serviceIntent = new Intent(context, KeyboardFirmwareUpdateService.class);
        serviceIntent.putExtra(KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_NAME, device.getName());
        serviceIntent.putExtra(KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_ADDRESS, device.getAddress());
        context.startService(serviceIntent);
    }
}
