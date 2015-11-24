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

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.PowerManager;
import android.support.v4.app.NotificationCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import java.util.List;
import java.util.UUID;

import no.nordicsemi.android.dfu.DfuProgressListener;
import no.nordicsemi.android.dfu.DfuServiceListenerHelper;


public class KeyboardFirmwareUpdateService extends Service {
    private static final String TAG = KeyboardFirmwareUpdateService.class.getSimpleName();

    /* Actions for update status changes. */
    public static final String ACTION_KEYBOARD_UPDATE_CONFIRMED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_CONFIRMED";
    public static final String ACTION_KEYBOARD_UPDATE_POSTPONED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_POSTPONED";
    public static final String ACTION_KEYBOARD_UPDATE_STARTED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_STARTED";
    public static final String ACTION_KEYBOARD_UPDATE_PROGRESS_CHANGED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_PROGRESS_CHANGED";
    public static final String ACTION_KEYBOARD_UPDATE_ABORTED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_ABORTED";
    public static final String ACTION_KEYBOARD_UPDATE_COMPLETED =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_COMPLETED";

    /* Actions for update notification flag. */
    public static final String ACTION_KEYBOARD_UPDATE_NOTIFICATION_OFF =
            "com.android.dragonkeyboardfirmwareupdater.action.KEYBOARD_UPDATE_NOTIFICATION_OFF";

    /* Extra information for UpdaterConfirmationActivity. */
    public static final String EXTRA_KEYBOARD_NAME =
            "com.android.dragonkeyboardfirmwareupdater.EXTRA_KEYBOARD_NAME";
    public static final String EXTRA_KEYBOARD_ADDRESS =
            "com.android.dragonkeyboardfirmwareupdater.EXTRA_KEYBOARD_ADDRESS";
    public static final String EXTRA_KEYBOARD_FIRMWARE_VERSION =
            "com.android.dragonkeyboardfirmwareupdater.EXTRA_KEYBOARD_FIRMWARE_VERSION";

    /**
     * Bluetooth connectivity. The Bluetooth LE connection maintained in this service is for
     * retrieving keyboard information, such as device manufacture and so on, and switching the
     * keyboard to Device Update Mode (DFU). Once the DFU service is started, the connection
     * maintained here and its corresponding variable should be cleaned up.
     */
    // Bluetooth Gatt connection state of the keyboard.
    private static final int GATT_STATE_DISCONNECTED = 0;
    private static final int GATT_STATE_CONNECTING = 1;
    private static final int GATT_STATE_CONNECTED = 2;
    private static final int GATT_STATE_DISCOVERING_SERVICES = 3;
    private static final int GATT_STATE_DISCONNECTING = 4;
    // Bluetooth system services.
    private static final long SCAN_PERIOD = 7000;  // 7 seconds
    private final IBinder mBinder = new LocalBinder();
    private int mGattConnectionState = GATT_STATE_DISCONNECTED;
    private int mGattOperationStatus = -1;
    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothLeScanner mBluetoothLeScanner;
    // Bluetooth Gatt connection bond with the keyboard.
    private BluetoothGatt mBluetoothGattClient;
    private String mKeyboardName;
    private String mKeyboardAddress;
    private String mKeyboardAddressInAppMode;
    private String mKeyboardFirmwareVersion;
    // Bluetooth Gatt services provided by the keyboard.
    private BluetoothGattService mBatteryService;
    private BluetoothGattService mDeviceInfoService;
    private BluetoothGattService mDfuService;
    private BluetoothGattCharacteristic mDfuChar;
    // Bluetooth LE scan retry flag.
    private boolean mLeScanRetried = false;
    private int mDfuStatus = DFU_STATE_NOT_STARTED;

    /* Handler for posting delayed tasks. */
    private Handler mHandler;

    /* Wake lock for DFU. */
    private PowerManager.WakeLock mWakeLock;

    /* Update notification. */
    private static final int UPDATE_NOTIFICATION_ID = 1248;
    private static final int BATTERY_WARNING_NOTIFICATION_ID = 8421;
    private Notification mUpdateNotification;

    /**
     * Keeps track of the status of DFU process.
     * DFU_STATE_NOT_STARTED: DFU not started
     * DFU_STATE_OBTAINING_INFO: DFU not started, obtaining manufacturer, firmware version and battery level
     * DFU_STATE_INFO_READY: DFU not started, manufacturer, firmware version and battery level obtained
     * DFU_STATE_SWITCHING_TO_DFU_MODE: DFU not started, waiting for the keyboard to reboot into DFU mode
     * DFU_STATE_MODE_SWITCHED: DFU not started, switched to DFU mode
     * DFU_STATE_UPDATING: DFU started, pushing the new firmware to the keyboard
     * DFU_STATE_UPDATE_COMPLETE: DFU finished correctly
     * DFU_STATE_INFO_NOT_SUITABLE: DFU not started, the keyboard is not suitable for update
     * DFU_STATE_OBTAIN_INFO_ERROR: DFU not started, error(s) occurred during obtaining information
     * DFU_STATE_SWITCH_TO_DFU_MODE_ERROR: DFU not started, failed to switch to DFU mode
     * DFU_STATE_UPDATE_ABORTED: DFU started but aborted by either users or errors during update
     */
    private static final int DFU_STATE_NOT_STARTED = 5;
    private static final int DFU_STATE_OBTAINING_INFO = 6;
    private static final int DFU_STATE_INFO_READY = 7;
    private static final int DFU_STATE_SWITCHING_TO_DFU_MODE = 8;
    private static final int DFU_STATE_MODE_SWITCHED = 9;
    private static final int DFU_STATE_UPDATING = 10;
    private static final int DFU_STATE_UPDATE_COMPLETE = 11;
    private static final int DFU_STATE_INFO_NOT_SUITABLE = 12;
    private static final int DFU_STATE_OBTAIN_INFO_ERROR = 13;
    private static final int DFU_STATE_SWITCH_TO_DFU_MODE_ERROR = 14;
    private static final int DFU_STATE_UPDATE_ABORTED = 15;

    /* Handles Bluetooth LE scan results. Bluetooth LE scan occurs after DFU preparation is ready. */
    private ScanCallback mBluetoothLeScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            if (device == null || device.getName() == null) return;

            // Find the keyboard in DFU mode and start DFU process. The name of keyboard in DFU mode
            // is composed of the last three groups (G3:G4:G5) of its application mode address
            // (G0:G1:G2:G3:G4:G5).
            if (mKeyboardAddressInAppMode.endsWith(device.getName().toUpperCase()) &&
                    mDfuStatus != DFU_STATE_UPDATING) {
                Log.d(TAG, "onScanResult: Found target keyboard in DFU mode");

                scanLeDevice(false);

                // Start pushing new firmware to the keyboard.
                startDfuService(device.getName(), device.getAddress());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.w(TAG, "onScanFailed: Error Code: " + errorCode);
            if (!mLeScanRetried) {
                // Retry the scan once.
                mLeScanRetried = true;
                scanLeDevice(true);
            }
        }
    };

    /**
     * Handles Bluetooth Gatt client callback. Read/write operations should finish in a certain
     * order after DFU preparation starts.
     */
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        private boolean retryFlag = true;

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            assert gatt == mBluetoothGattClient;

            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    if (!checkOperationStatus(status)) {
                        Log.w(TAG, "BluetoothGattCallback: Transferring to new state: " + newState +
                                " failed with code: " + status);
                        return;
                    }

                    changeGattState(GATT_STATE_CONNECTED);
                    Log.i(TAG, "BluetoothGattCallback: Connected to Bluetooth Gatt server on " +
                            getKeyboardString());
                    // Start to discover services right after connection.
                    mBluetoothGattClient.discoverServices();
                    changeGattState(GATT_STATE_DISCOVERING_SERVICES);
                    Log.d(TAG, "BluetoothGattCallback: Start to discover services on " +
                            getKeyboardString());
                    break;

                case BluetoothProfile.STATE_DISCONNECTED:
                    Log.i(TAG, "BluetoothGattCallback: Disconnected from Bluetooth Gatt server on "
                            + getKeyboardString() + ", status: " + status);
                    if (mGattConnectionState != GATT_STATE_DISCONNECTED) {
                        changeGattState(GATT_STATE_DISCONNECTED);
                    }
                    cleanUpGattConnection();
                    break;
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            assert gatt == mBluetoothGattClient;

            if (!checkOperationStatus(status)) {
                changeDfuStatus(DFU_STATE_NOT_STARTED);
                return;
            }

            if (!getGattServices()) return;

            changeGattState(GATT_STATE_CONNECTED);
            readBatteryLevel();
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            assert gatt == mBluetoothGattClient;

            if (!checkOperationStatus(status)) {
                changeDfuStatus(DFU_STATE_OBTAIN_INFO_ERROR);
                return;
            }

            UUID uuid = characteristic.getUuid();
            if (GattAttributeUUID.UUID_BATTERY_LEVEL_CHARACTERISTIC.equals(uuid)) {

                int batteryLevel = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                if (batteryLevel < Integer.parseInt(getString(R.string.target_battery_level))) {
                    Log.w(TAG, "onCharacteristicRead BATTERY_LEVEL_CHARACTERISTIC: " +getKeyboardString()
                            + " battery level(" + batteryLevel + "%) is too low:");
                    changeDfuStatus(DFU_STATE_INFO_NOT_SUITABLE);

                    showBatteryWarningNotification();

                    return;
                }

                readDeviceManufacturer();
            } else if (GattAttributeUUID.UUID_DEVICE_INFORMATION_MANUFACTURER_CHARACTERISTIC.equals(uuid)) {

                String manufacturer = new String(characteristic.getValue());
                if (!manufacturer.equals(getString(R.string.target_manufacturer))) {
                    Log.d(TAG, "onCharacteristicRead DEVICE_INFORMATION_MANUFACTURER_CHARACTERISTIC: Invalid manufacturer: "
                            + manufacturer);
                    changeDfuStatus(DFU_STATE_INFO_NOT_SUITABLE);
                    return;
                }

                readDeviceFirmwareVersion();
            } else if (GattAttributeUUID.UUID_DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC.equals(uuid)) {

                mKeyboardFirmwareVersion = new String(characteristic.getValue());
                Log.d(TAG, "onCharacteristicRead DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC: current: "
                        + mKeyboardFirmwareVersion + " new: " + getString(R.string.target_firmware_version));

                Float versionNumber = 0.0f;
                // Parse the firmware version into Float number for the following checks.
                try {
                    versionNumber = Float.parseFloat(mKeyboardFirmwareVersion);
                } catch(NumberFormatException e) {
                    Log.w(TAG, "onCharacteristicRead DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC: " +
                          "firmware version parsing error");
                    changeDfuStatus(DFU_STATE_INFO_NOT_SUITABLE);
                    return;
                }

                // Check if the current firmware is updatable.
                if (versionNumber < Float.parseFloat(getString(R.string.target_min_updatable_firmware_version))) {
                    Log.d(TAG, "onCharacteristicRead DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC: " +
                          "current firmware(" + mKeyboardFirmwareVersion + ") is not updatable");
                    changeDfuStatus(DFU_STATE_INFO_NOT_SUITABLE);
                    return;
                }

                // Check if the current firmware is up to date.
                if (versionNumber >= Float.parseFloat(getString(R.string.target_firmware_version))) {
                    Log.d(TAG, "onCharacteristicRead DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC: " +
                            getKeyboardString() + " firmware(" + mKeyboardFirmwareVersion + ") is up to date");
                    changeDfuStatus(DFU_STATE_INFO_NOT_SUITABLE);
                    return;
                }

                changeDfuStatus(DFU_STATE_INFO_READY);
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            assert gatt == mBluetoothGattClient;

            if (GattAttributeUUID.UUID_DFU_CONTROL_POINT_CHARACTERISTIC.equals(characteristic.getUuid())) {
                Log.d(TAG, "onCharacteristicWrite DFU_CONTROL_POINT_CHARACTERISTIC: status: " + status);
                changeDfuStatus(DFU_STATE_MODE_SWITCHED);
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            assert gatt == mBluetoothGattClient;

            if (GattAttributeUUID.UUID_DFU_CONTROL_POINT_DESCRIPTOR.equals(descriptor.getUuid())) {
                Log.d(TAG, "onDescriptorWrite: DFU_CONTROL_POINT_DESCRIPTOR, status: " + status);
                enableDfuMode();
            }
        }
    };

    /* Handles DfuService callback. DFU service starts after the keyboard is found in LE scan.*/
    private DfuProgressListener mDfuProgressListener = new DfuProgressListener() {
        private void cancelDfuServiceNotification() {
            // Wait a bit before cancelling notification.
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    // If this activity is still open and upload process was completed, cancel the notification.
                    final NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
                    manager.cancel(DfuService.NOTIFICATION_ID);
                }
            }, 200);
        }

        @Override
        public void onDeviceConnecting(String deviceAddress) {
        }

        @Override
        public void onDeviceConnected(String deviceAddress) {
        }

        @Override
        public void onDfuProcessStarting(String deviceAddress) {
        }

        @Override
        public void onDfuProcessStarted(String deviceAddress) {
        }

        @Override
        public void onEnablingDfuMode(String deviceAddress) {
        }

        @Override
        public void onProgressChanged(
                String deviceAddress, int percent, float speed, float avgSpeed, int currentPart, int partsTotal) {
            if ((percent % 5) == 0) {
                Log.i(TAG, "DfuProgressListener: onProgressChanged: part" + currentPart + "/" +
                        partsTotal + ", " + percent + "%");
            }
        }

        @Override
        public void onFirmwareValidating(String deviceAddress) {
        }

        @Override
        public void onDeviceDisconnecting(String deviceAddress) {
        }

        @Override
        public void onDeviceDisconnected(String deviceAddress) {
        }

        @Override
        public void onDfuCompleted(String deviceAddress) {
            Log.d(TAG, "DfuProgressListener: onDfuCompleted");
            cancelDfuServiceNotification();
            changeDfuStatus(DFU_STATE_UPDATE_COMPLETE);
            mWakeLock.release();
        }

        @Override
        public void onDfuAborted(String deviceAddress) {
            Log.e(TAG, "DfuProgressListener: onDfuAborted");
            cancelDfuServiceNotification();
            changeDfuStatus(DFU_STATE_UPDATE_ABORTED);
            mWakeLock.release();
        }

        @Override
        public void onError(String deviceAddress, int error, int errorType, String message) {
            Log.e(TAG, "DfuProgressListener: onError: " + message);
            cancelDfuServiceNotification();
            changeDfuStatus(DFU_STATE_UPDATE_ABORTED);
            mWakeLock.release();
        }
    };
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            onHandleIntent(context, intent);
        }
    };

    /* Dynamically creates intent filter for BroadcastReceiver. */
    private static IntentFilter makeIntentFilter() {
        IntentFilter intentFilter = new IntentFilter();

        intentFilter.addAction(BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED);
        intentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        intentFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        intentFilter.addAction(ACTION_KEYBOARD_UPDATE_CONFIRMED);
        intentFilter.addAction(ACTION_KEYBOARD_UPDATE_POSTPONED);

        return intentFilter;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate: " + getString(R.string.app_name));
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand: " + getString(R.string.app_name));

        enableBluetoothConnectivity();
        DfuServiceListenerHelper.registerProgressListener(this, mDfuProgressListener);
        registerReceiver(mBroadcastReceiver, makeIntentFilter());
        mHandler = new Handler();

        // TODO(mcchou): Return proper flag.
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy" + getString(R.string.app_name));

        disableBluetoothConnectivity();
        DfuServiceListenerHelper.unregisterProgressListener(this, mDfuProgressListener);
        unregisterReceiver(mBroadcastReceiver);
    }

    /**
     * Handles intents ACTION_CONNECTION_STATE_CHANGED, ACTION_STATE_CHANGED,
     * ACTION_BOND_STATE_CHANGED, ACTION_KEYBOARD_UPDATE_CONFIRMED.
     * <p/>
     * [ACTION_STATE_CHANGED]
     * This action is used to keep track of ON/OFF state change on the system Bluetooth adapter.
     * The
     * purpose is to synchronize the local Bluetooth connectivity with system Bluetooth state.
     * <p/>
     * [ACTION_CONNECTION_STATE_CHANGED]
     * This action is used to keep track of the connection change on the target device. The purpose
     * is to synchronize the connection cycles of the local GATT connection and the system
     * Bluetooth
     * connection.
     * <p/>
     * [ACTION_BOND_STATE_CHANGED]
     * This action is used to keep track of the bond state change on the target device. The purpose
     * is to the connection cycles of the local GATT connection and the system Bluetooth
     * connection.
     * <p/>
     * [ACTION_KEYBOARD_UPDATE_CONFIRMED]
     * This action is used to receive the update confirmation from the user. The purpose is to
     * trigger DFU process.
     */
    private void onHandleIntent(Context context, Intent intent) {
        final String action = intent.getAction();
        Log.d(TAG, "onHandleIntent: Received action: " + action);

        if (BluetoothAdapter.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {

            if (!isBluetoothEnabled()) {
                Log.w(TAG, "onHandleIntent: Bluetooth connectivity not enabled");
                return;
            }

            // Match the connected device with the default keyboard name.
            Bundle extras = intent.getExtras();
            if (extras == null) return;
            final BluetoothDevice device = extras.getParcelable(BluetoothDevice.EXTRA_DEVICE);
            final int deviceConnectionState = extras.getInt(BluetoothAdapter.EXTRA_CONNECTION_STATE);

            Log.d(TAG, "onHandleIntent: " + device.getName() + " [" + device.getAddress() +
                    "] change to state: " + deviceConnectionState);

            // Match the name of the target keyboard.
            if (!isTargetKeyboard(device)) return;

            if (deviceConnectionState == BluetoothAdapter.STATE_CONNECTED) {
                // Prevent the second keyboard from using the service.
                if (isUpdateServiceInUse()) return;

                obtainKeyboardInfo(device.getName(), device.getAddress());

                if (mDfuStatus != DFU_STATE_INFO_READY) {
                    Log.w(TAG, "onHandleIntent: DFU preparation failed");
                    changeDfuStatus(DFU_STATE_OBTAIN_INFO_ERROR);
                    return;
                }

                showUpdateNotification();
            } else if (deviceConnectionState == BluetoothAdapter.STATE_DISCONNECTING) {
                handleGattDisconnection();
            }

        } else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            final int adapterState = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
            if (adapterState == BluetoothAdapter.STATE_ON) {
                if (!isBluetoothEnabled()) enableBluetoothConnectivity();
            } else if (adapterState == BluetoothAdapter.STATE_TURNING_OFF) {
                // Terminate update process and disable Bluetooth connectivity.
                disableBluetoothConnectivity();

                // Since BluetoothAdapter has been disabled, the callback of disconnection would not
                // be called. Therefore a separate clean-up of GATT connection is need.
                cleanUpGattConnection();
            }

        } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
            Bundle extras = intent.getExtras();
            if (extras == null) return;
            final BluetoothDevice device = extras.getParcelable(BluetoothDevice.EXTRA_DEVICE);
            final int deviceBondState = extras.getInt(BluetoothDevice.EXTRA_BOND_STATE);

            Log.d(TAG, "onHandleIntent: state change on device " + device.getName() + " [" +
                    device.getAddress() + "], bond state: " + deviceBondState);

            if (!isTargetKeyboard(device)) return;

            if (deviceBondState == BluetoothDevice.BOND_NONE) {
                handleGattDisconnection();
            }

        } else if (ACTION_KEYBOARD_UPDATE_CONFIRMED.equals(action)) {
            dismissUpdateNotification();

            if (mDfuStatus != DFU_STATE_INFO_READY || mDfuStatus == DFU_STATE_UPDATING) {
                Log.w(TAG, "onHandleIntent: DFP preparation not ready or DFU is in progress. ");
                changeDfuStatus(DFU_STATE_UPDATE_ABORTED);
                return;
            }

            String keyboardName = intent.getStringExtra(EXTRA_KEYBOARD_NAME);
            String keyboardAddress = intent.getStringExtra(EXTRA_KEYBOARD_ADDRESS);
            if (!mKeyboardName.equals(keyboardName) || !mKeyboardAddress.equals(keyboardAddress)) {
                Log.w(TAG, "onHandleIntent: No DFU service associated with " + keyboardName + " [" +
                        keyboardAddress + "]");
                return;
            }

            Log.d(TAG, "onHandleIntent: Start update process on " + keyboardName + " [" +
                    keyboardAddress + "]");
            changeDfuStatus(DFU_STATE_SWITCHING_TO_DFU_MODE);

        } else if (ACTION_KEYBOARD_UPDATE_POSTPONED.equals(action)) {
            dismissUpdateNotification();
            // TODO(mcchou): Update the preference when the Settings keyboard entry is available.
        }
    }

    /* Checks if Bluetooth connectivity is enabled. */
    private boolean isBluetoothEnabled() {
        return (mBluetoothManager != null && mBluetoothAdapter != null && mBluetoothLeScanner != null);
    }

    /* Checks if there is already a keyboard associated with the update service. */
    private boolean isUpdateServiceInUse() {
        return (mKeyboardName != null && mKeyboardAddress != null);
    }

    /* Returns a string including the keyboard name and address. */
    private String getKeyboardString() {
        return mKeyboardName + " [" + mKeyboardAddress + "]";
    }

    private boolean isTargetKeyboard(BluetoothDevice device) {
        return (device != null && getString(R.string.target_keyboard_name).equals(device.getName()));
    }

    /* Retrieves Bluetooth manager, adapter and scanner. */
    private boolean enableBluetoothConnectivity() {
        Log.d(TAG, "EnableBluetoothConnectivity");
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                Log.w(TAG, "EnableBluetoothConnectivity: Failed to obtain BluetoothManager");
                return false;
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Log.w(TAG, "EnableBluetoothConnectivity: Failed to obtain BluetoothAdapter");
            return false;
        }

        mBluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
        if (mBluetoothLeScanner == null) {
            Log.w(TAG, "EnableBluetoothConnectivity: Failed to obtain BluetoothLeScanner");
            return false;
        }

        // The first auto-connection after boot might be missed due to starting time of the updater service.
        List<BluetoothDevice> connectedDevices = mBluetoothManager.getConnectedDevices(BluetoothProfile.GATT);
        for (BluetoothDevice device : connectedDevices) {
            if (isTargetKeyboard(device) && !isUpdateServiceInUse()) {
                Log.d(TAG, "enableBluetoothConnectivity: Found keyboard " + device.getName() + " [" +
                        device.getAddress() + "] connected");
                obtainKeyboardInfo(device.getName(), device.getAddress());
                break;
            }
        }

        return true;
    }

    /* Disables Bluetooth connectivity if exists. */
    private void disableBluetoothConnectivity() {
        Log.d(TAG, "disableBluetoothConnectivity");
        handleGattDisconnection();
        try {
            Thread.sleep(3000);  // 3 seconds
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        mBluetoothManager = null;
        mBluetoothAdapter = null;
        mBroadcastReceiver = null;
    }

    /* Shows the update notification. */
    private void showUpdateNotification() {
        Log.d(TAG, "showUpdateNotification: " + getKeyboardString());

        // Intent for triggering the update confirmation page.
        Intent updateConfirmation = new Intent(this, UpdateConfirmationActivity.class);
        updateConfirmation.putExtra(EXTRA_KEYBOARD_NAME, mKeyboardName);
        updateConfirmation.putExtra(EXTRA_KEYBOARD_ADDRESS, mKeyboardAddress);
        updateConfirmation.putExtra(EXTRA_KEYBOARD_FIRMWARE_VERSION, mKeyboardFirmwareVersion);
        updateConfirmation.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        // Intent for postponing update.
        Intent postponeUpdate = new Intent(ACTION_KEYBOARD_UPDATE_POSTPONED);

        // Wrap intents into pending intents for notification use.
        PendingIntent laterIntent = PendingIntent.getBroadcast(
                this, 0, postponeUpdate, PendingIntent.FLAG_UPDATE_CURRENT);
        PendingIntent installIntent = PendingIntent.getActivity(
                this, 0, updateConfirmation, PendingIntent.FLAG_CANCEL_CURRENT);

        // Create a notification object with two buttons (actions)
        mUpdateNotification = new NotificationCompat.Builder(this)
                .setCategory(Notification.CATEGORY_SYSTEM)
                .setContentTitle(getString(R.string.notification_update_title))
                .setContentText(getString(R.string.notification_update_text))
                .setSmallIcon(R.drawable.ic_keyboard)
                .addAction(new NotificationCompat.Action.Builder(
                        R.drawable.ic_later, getString(R.string.notification_update_later),
                        laterIntent).build())
                .addAction(new NotificationCompat.Action.Builder(
                        R.drawable.ic_install, getString(R.string.notification_update_install),
                        installIntent).build())
                .setAutoCancel(true)
                .setOnlyAlertOnce(true)
                .build();

        // Show the notification via notification manager
        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(UPDATE_NOTIFICATION_ID, mUpdateNotification);
    }

    /* Dismisses the udpate notification. */
    private void dismissUpdateNotification() {
        if (mUpdateNotification == null) return;
        Log.d(TAG, "dismissUpdateNotification");
        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(UPDATE_NOTIFICATION_ID);
        mUpdateNotification = null;
    }

    /* Shows the keyboard battery warning notification. */
    private void showBatteryWarningNotification() {
        Log.d(TAG, "showBatteryWarningNotification: " + getKeyboardString());

        Notification batteryWarningNotification = new NotificationCompat.Builder(this)
                .setContentTitle(getString(R.string.notification_battery_warning_title))
                .setContentText(getString(R.string.notification_battery_warning_text))
                .setSmallIcon(R.drawable.ic_battery_warning)
                .setAutoCancel(true)
                .setOnlyAlertOnce(true)
                .setPriority(Notification.PRIORITY_HIGH)
                .setColor(Color.RED)
                .setStyle(new NotificationCompat.BigTextStyle().bigText(
                        getString(R.string.notification_battery_warning_text)))
                .build();

        // Show the notification via notification manager
        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(BATTERY_WARNING_NOTIFICATION_ID, batteryWarningNotification);
    }

    /* Connects to the GATT server hosted on the given Bluetooth LE device. */
    private boolean connectToKeyboard() {
        if (!isBluetoothEnabled() || !isUpdateServiceInUse()) {
            Log.w(TAG, "connectToKeyboard: Bluetooth connectivity not enabled or associated keyboard not found.");
            return false;
        }

        final BluetoothDevice keyboard = mBluetoothAdapter.getRemoteDevice(mKeyboardAddress);
        if (keyboard == null) {
            Log.w(TAG, "connectToKeyboard: " + getKeyboardString() + " not found. Unable to connect.");
            return false;
        }

        Log.d(TAG, "connectToKeyboard: Trying to create a new connection to " + getKeyboardString());
        mBluetoothGattClient = keyboard.connectGatt(this, false, mGattCallback);
        changeGattState(GATT_STATE_CONNECTING);
        mGattOperationStatus = BluetoothGatt.GATT_SUCCESS;

        return true;
    }

    /* Disconnects from the GATT server hosted on the given Bluetooth LE device. */
    private void disconnectFromKeyboard() {
        if (mGattConnectionState == GATT_STATE_DISCONNECTED) return;
        if (!isUpdateServiceInUse() || !isBluetoothEnabled() || mDfuStatus == DFU_STATE_NOT_STARTED) {
            Log.i(TAG, "disconnectFromKeyboard: Bluetooth connectivity not enabled");
            return;
        }

        Log.d(TAG, "disconnectFromKeyboard: " + getKeyboardString());

        mBluetoothGattClient.disconnect();
        changeGattState(GATT_STATE_DISCONNECTING);
        mGattOperationStatus = BluetoothGatt.GATT_SUCCESS;
    }

    /**
     * Cleans up Bluetooth GATT connection and the keyboard. This should be done before starting
     * DFU process.
     */
    private void cleanUpGattConnection() {
        Log.d(TAG, "cleanUpGattConnection");
        mKeyboardName = null;
        mKeyboardAddress = null;
        mKeyboardFirmwareVersion = null;
        mBluetoothGattClient = null;
        mBatteryService = null;
        mDeviceInfoService = null;
        mDfuService = null;
        mDfuChar = null;
        mLeScanRetried = false;
    }

    /* Starts to collect the information of the keyboard. */
    private void obtainKeyboardInfo(String keyboardName, String keyboardAddress) {
        Log.d(TAG, "obtainKeyboardInfo: Obtain the information of " + keyboardName + " [" +
                keyboardAddress + "]");

        // Connect to the keyboard and start to obtain its information.
        mKeyboardName = keyboardName;
        mKeyboardAddress = keyboardAddress.toUpperCase();
        mKeyboardAddressInAppMode = mKeyboardAddress;
        Log.d(TAG, "obtainKeyboardInfo: Associate DFU service with " + getKeyboardString());

        if (mGattConnectionState == GATT_STATE_CONNECTED) {
            Log.i(TAG, "obtainKeyboardInfo: Reuse previous GATT connection");
            readBatteryLevel();
        } else if (mGattConnectionState == GATT_STATE_DISCONNECTED) {
            changeDfuStatus(DFU_STATE_OBTAINING_INFO);
        } else {
            Log.w(TAG, "obtainKeyboardInfo: Failed to obtain keyboard information");
        }

        // Wait at most 10 seconds for the queries to GATT attributes to finish.
        int waitTimes = 5;
        while (mDfuStatus == DFU_STATE_OBTAINING_INFO && waitTimes > 0) {
            try {
                Thread.sleep(2000);  // 2 seconds
                waitTimes--;
                Log.d(TAG, "obtainKeyboardInfo: Wait for preparation completion");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    /* Starts/stops Bluetooth LE scan. */
    private void scanLeDevice(final boolean enable) {
        if (!isBluetoothEnabled()) {
            Log.w(TAG, "scanLeDevice: Bluetooth connectivity not enabled");
        }
        if (enable) {
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "scanLeDevice: Stop scanning");
                    mBluetoothLeScanner.stopScan(mBluetoothLeScanCallback);
                }
            }, SCAN_PERIOD);
            Log.d(TAG, "scanLeDevice: Start scanning");
            mBluetoothLeScanner.startScan(mBluetoothLeScanCallback);
        } else {
            Log.d(TAG, "scanLeDevice: Stop scanning");
            mBluetoothLeScanner.stopScan(mBluetoothLeScanCallback);
        }
    }

    /**
     * Retrieves Battery Service, Device Information Service, Device Firmware Update(DFU) service
     * and DFU Control Point Characteristic.
     */
    private boolean getGattServices() {
        Log.d(TAG, "getDfuServiceAndChar");

        if (mBluetoothGattClient == null) {
            Log.w(TAG, "getDfuServiceAndChar: Bluetooth GATT connection not initiated");
            return false;
        }

        mBatteryService = mBluetoothGattClient.getService(GattAttributeUUID.UUID_BATTERY_SERVICE);
        if (mBatteryService == null) {
            Log.e(TAG, "getBatteryService: Failed to get Battery Service");
            return false;
        }
        mDeviceInfoService = mBluetoothGattClient.getService(GattAttributeUUID.UUID_DEVICE_INFORMATION_SERVICE);
        if (mDeviceInfoService == null) {
            Log.e(TAG, "getDeviceInfoService: Failed to get Device Information Service");
            return false;
        }

        mDfuService = mBluetoothGattClient.getService(GattAttributeUUID.UUID_DFU_SERVICE);
        if (mDfuService == null) {
            Log.e(TAG, "getDfuServiceAndChar: Failed to get Device Firmware Update Service");
            return false;
        }

        mDfuChar = mDfuService.getCharacteristic(GattAttributeUUID.UUID_DFU_CONTROL_POINT_CHARACTERISTIC);
        if (mDfuChar == null) {
            Log.e(TAG, "getDfuServiceAndChar: Failed to get DFU Control Point characteristic");
            return false;
        }

        return true;
    }

    /* Retrieves battery level of the connected keyboard. */
    private boolean readBatteryLevel() {
        Log.d(TAG, "readBatteryLevel");

        BluetoothGattCharacteristic batteryLevelChar = mBatteryService.getCharacteristic(
                GattAttributeUUID.UUID_BATTERY_LEVEL_CHARACTERISTIC);
        if (batteryLevelChar == null || !mBluetoothGattClient.readCharacteristic(batteryLevelChar)) {
            Log.e(TAG, "readBatteryLevel: Failed to init batter level read operation");
            return false;
        }
        return true;
    }

    /* Retrieves device manufacturer of the connected keyboard. */
    private boolean readDeviceManufacturer() {
        Log.d(TAG, "readDeviceManufacturer");

        BluetoothGattCharacteristic deviceManufacturerChar = mDeviceInfoService.getCharacteristic(
                GattAttributeUUID.UUID_DEVICE_INFORMATION_MANUFACTURER_CHARACTERISTIC);
        if (deviceManufacturerChar == null ||
                !mBluetoothGattClient.readCharacteristic(deviceManufacturerChar)) {
            Log.e(TAG, "readDeviceInfo: Failed to init device manufacturer characteristic read operation");
            return false;
        }
        return true;
    }

    /* Retrieves device firmware version of the connected keyboard. */
    private boolean readDeviceFirmwareVersion() {
        Log.d(TAG, "readDeviceFirmwareVersion");

        BluetoothGattCharacteristic deviceFirmwareVersionChar = mDeviceInfoService.getCharacteristic(
                GattAttributeUUID.UUID_DEVICE_INFORMATION_FIRMWARE_VERSION_CHARACTERISTIC);
        if (deviceFirmwareVersionChar == null ||
                !mBluetoothGattClient.readCharacteristic(deviceFirmwareVersionChar)) {
            Log.e(TAG, "readDeviceInfo: Failed to get device firmware revision characteristic");
            return false;
        }
        return true;
    }

    /* Enables device firmware update notification of the connected keyboard. */
    private boolean enableDfuNotification() {
        Log.d(TAG, "enableDfuNotification");

        if (mDfuChar == null) {
            Log.w(TAG, "enableDfuNotification: DFU control point characteristic not initiated");
            return false;
        }

        BluetoothGattDescriptor dfuDesc = mDfuChar.getDescriptor(
                GattAttributeUUID.UUID_DFU_CONTROL_POINT_DESCRIPTOR);
        if (dfuDesc == null ||
                !dfuDesc.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE) ||
                !mBluetoothGattClient.writeDescriptor(dfuDesc)) {
            Log.e(TAG, "enableDfuNotification: Failed to init DFU descriptor write operation");
            return false;
        }
        return true;
    }

    /* Switches the connected keyboard to DFU mode. */
    private boolean enableDfuMode() {
        Log.d(TAG, "enableDfuMode");

        if (mDfuChar == null) {
            Log.w(TAG, "enableDfuMode: DFU control point characteristic not initiated");
            return false;
        }

        // Opcode: 0x01 -> Start DFU mode.
        //         0x04 -> DFU type: application
        final byte dfuOpcodeWithTypeApplication[] = new byte[]{0x01, 0x04};
        if (!mDfuChar.setValue(dfuOpcodeWithTypeApplication) ||
                !mBluetoothGattClient.writeCharacteristic(mDfuChar)) {
            Log.e(TAG, "enableDfuMode: Failed to init DFU mode switch");
            return false;
        }
        return true;
    }

    /* Checks if a Bluetooth Gatt operation is finished correctly. */
    private boolean checkOperationStatus(int status) {
        mGattOperationStatus = status;
        if (mGattOperationStatus != BluetoothGatt.GATT_SUCCESS) {
            Log.d(TAG, "BluetoothGattCallback: GATT operation failure: " + mGattOperationStatus);
            return false;
        }
        return true;
    }

    /* Starts DFU process. */
    private void startDfuService(String keyboardName, String keyboardAddress) {
        Log.d(TAG, "startDfuService");

        changeDfuStatus(DFU_STATE_UPDATING);

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.acquire();

        String packageName = getApplicationContext().getPackageName();
        int initResourceId = getResources().getIdentifier(
            getString(R.string.target_firmware_init_file_name), "raw", packageName);
        int imageResourceId = getResources().getIdentifier(
            getString(R.string.target_firmware_image_file_name), "raw", packageName);
        boolean keepBond = true;

        Log.d(TAG, "Name: " + keyboardName + "\n" +
                "Address: " + keyboardAddress + "\n" +
                "Init file: " + getString(R.string.target_firmware_init_file_name) + "\n" +
                "Image file: " + getString(R.string.target_firmware_image_file_name) + "\n" +
                "Image type: Application(" + DfuService.TYPE_APPLICATION + ")\n" +
                "Keep bond: " + keepBond);

        final Intent service = new Intent(this, DfuService.class);
        service.putExtra(DfuService.EXTRA_DEVICE_NAME, keyboardName);
        service.putExtra(DfuService.EXTRA_DEVICE_ADDRESS, keyboardAddress);
        service.putExtra(DfuService.EXTRA_INIT_FILE_RES_ID, initResourceId);
        service.putExtra(DfuService.EXTRA_FILE_RES_ID, imageResourceId);
        service.putExtra(DfuService.EXTRA_FILE_TYPE, DfuService.TYPE_APPLICATION);
        service.putExtra(DfuService.EXTRA_KEEP_BOND, true);

        startService(service);
    }

    /* Aborts DFU service if it is in progress. */
    public void abortDfu() {
        if (mDfuStatus != DFU_STATE_UPDATING) return;
        final Intent pauseAction = new Intent(DfuService.BROADCAST_ACTION);
        pauseAction.putExtra(DfuService.EXTRA_ACTION, DfuService.ACTION_ABORT);
        LocalBroadcastManager.getInstance(this).sendBroadcast(pauseAction);
    }

    /* State setter of GATT connection. */
    private void changeGattState(int newStatus) {
        mGattConnectionState = newStatus;
        Log.i(TAG, "-- changeGattState: " + getGattStateString(mGattConnectionState));
    }

    /* Helper function for logging GATT state change. */
    private String getGattStateString(int state) {
        switch (state) {
            case GATT_STATE_DISCONNECTED:
                return "GATT_STATE_DISCONNECTED";
            case GATT_STATE_CONNECTING:
                return "GATT_STATE_CONNECTING";
            case GATT_STATE_CONNECTED:
                return "GATT_STATE_CONNECTED";
            case GATT_STATE_DISCOVERING_SERVICES:
                return "GATT_STATE_DISCOVERING_SERVICES";
            case GATT_STATE_DISCONNECTING:
                return "GATT_STATE_DISCONNECTING";
            default:
                return "Unknown state (" + state + ")";
        }
    }

    /* State flow for the updater service. */
    private void changeDfuStatus(int newStatus) {
        int nextStatus = newStatus;
        switch (newStatus) {
            case DFU_STATE_NOT_STARTED:
                break;
            case DFU_STATE_OBTAINING_INFO:
                connectToKeyboard();
                break;
            case DFU_STATE_INFO_READY:
                // TODO(mcchou): Send info intent to Settings.
                break;
            case DFU_STATE_SWITCHING_TO_DFU_MODE:
                // TODO(mcchou): Send update in progress to Settings.
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if (mDfuStatus == DFU_STATE_SWITCHING_TO_DFU_MODE)
                            changeDfuStatus(DFU_STATE_SWITCH_TO_DFU_MODE_ERROR);
                    }
                }, SCAN_PERIOD * 2);
                enableDfuNotification();
                break;
            case DFU_STATE_MODE_SWITCHED:
                scanLeDevice(true);
                break;
            case DFU_STATE_UPDATING:
                // TODO(mcchou): Send progress intent to Settings.
                break;
            case DFU_STATE_UPDATE_COMPLETE:
                // TODO(mcchou): Send update complete to Settings.
                nextStatus = DFU_STATE_NOT_STARTED;
                break;
            case DFU_STATE_INFO_NOT_SUITABLE:
                // TODO(mcchou): Send fail intent to Settings.
                disconnectFromKeyboard();
                nextStatus = DFU_STATE_NOT_STARTED;
                break;
            case DFU_STATE_OBTAIN_INFO_ERROR:
                // TODO(mcchou): Send fail intent to Settings.
                disconnectFromKeyboard();
                nextStatus = DFU_STATE_NOT_STARTED;
                break;
            case DFU_STATE_SWITCH_TO_DFU_MODE_ERROR:
                // TODO(mcchou): Send abort intent to Settings.
                nextStatus = DFU_STATE_INFO_READY;
                break;
            case DFU_STATE_UPDATE_ABORTED:
                // TODO(mcchou): Send abort intent to Settings.
                nextStatus = DFU_STATE_NOT_STARTED;
                break;
            default:
                break;
        }
        mDfuStatus = newStatus;
        Log.d(TAG, "---- changeDfuStatus: " + getDfuStateString(mDfuStatus));
        if (nextStatus != newStatus) changeDfuStatus(nextStatus);
    }

    /* Helper function for logging DFU state change. */
    private String getDfuStateString(int state) {
        switch (state) {
            case DFU_STATE_NOT_STARTED:
                return "DFU_STATE_NOT_STARTED";
            case DFU_STATE_OBTAINING_INFO:
                return "DFU_STATE_OBTAINING_INFO";
            case DFU_STATE_INFO_READY:
                return "DFU_STATE_INFO_READY";
            case DFU_STATE_SWITCHING_TO_DFU_MODE:
                return "DFU_STATE_SWITCHING_TO_DFU_MODE";
            case DFU_STATE_MODE_SWITCHED:
                return "DFU_STATE_MODE_SWITCHED";
            case DFU_STATE_UPDATING:
                return "DFU_STATE_UPDATING";
            case DFU_STATE_UPDATE_COMPLETE:
                return "DFU_STATE_UPDATE_COMPLETE";
            case DFU_STATE_INFO_NOT_SUITABLE:
                return "DFU_STATE_INFO_NOT_SUITABLE";
            case DFU_STATE_OBTAIN_INFO_ERROR:
                return "DFU_STATE_OBTAIN_INFO_ERROR";
            case DFU_STATE_SWITCH_TO_DFU_MODE_ERROR:
                return "DFU_STATE_SWITCH_TO_DFU_MODE_ERROR";
            case DFU_STATE_UPDATE_ABORTED:
                return "DFU_STATE_UPDATE_ABORTED";
            default:
                return "Unknown state (" + state + ")";
        }
    }

    /* Handles GATT disconnection. */
    private void handleGattDisconnection() {
        Log.d(TAG, "handleGattDisconnection");
        if (mGattConnectionState == GATT_STATE_DISCONNECTED) return;

        // TODO(mcchou): add fall through comment
        // Handle update process termination based on the current DFU state.
        switch (mDfuStatus) {
            case DFU_STATE_SWITCHING_TO_DFU_MODE:
                scanLeDevice(false);
            case DFU_STATE_OBTAINING_INFO:
            case DFU_STATE_INFO_READY:
            case DFU_STATE_INFO_NOT_SUITABLE:
            case DFU_STATE_SWITCH_TO_DFU_MODE_ERROR:
                disconnectFromKeyboard();
            case DFU_STATE_NOT_STARTED:
            case DFU_STATE_MODE_SWITCHED:
            case DFU_STATE_UPDATE_COMPLETE:
            case DFU_STATE_UPDATE_ABORTED:
                break;
            case DFU_STATE_UPDATING:
                abortDfu();
                dismissUpdateNotification();
                break;
            default:
                break;
        }
        changeGattState(GATT_STATE_DISCONNECTED);
    }

    public class LocalBinder extends Binder {
        KeyboardFirmwareUpdateService getService() {
            return KeyboardFirmwareUpdateService.this;
        }
    }
}
