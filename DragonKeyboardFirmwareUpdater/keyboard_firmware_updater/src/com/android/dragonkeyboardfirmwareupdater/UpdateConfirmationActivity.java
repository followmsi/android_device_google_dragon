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

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class UpdateConfirmationActivity extends Activity {
    private final static String TAG = UpdateConfirmationActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");

        setContentView(R.layout.activity_update_confirmation);

        final Button installButton = (Button) findViewById(R.id.confirmation_install_button);
        installButton.setVisibility(View.VISIBLE);
        installButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                installButton.setEnabled(false);
                installButton.setVisibility(View.INVISIBLE);

                final Intent intent = getIntent();
                final Intent updateIntent = new Intent(
                        KeyboardFirmwareUpdateService.ACTION_KEYBOARD_UPDATE_CONFIRMED);
                updateIntent.putExtra(
                        KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_NAME,
                        intent.getStringExtra(KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_NAME));
                updateIntent.putExtra(
                        KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_ADDRESS,
                        intent.getStringExtra(KeyboardFirmwareUpdateService.EXTRA_KEYBOARD_ADDRESS));
                sendBroadcast(updateIntent);

                // Close update confirmation page.
                finish();
            }
        });
    }
}
