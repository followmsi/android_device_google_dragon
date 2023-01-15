package com.dragon.parts;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.MenuItem;

import androidx.preference.PreferenceFragment;

import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity;
import com.android.settingslib.widget.R;

public class DeviceSettingsActivity extends CollapsingToolbarBaseActivity {

    private DeviceSettings mDeviceSettingsFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getActionBar().setDisplayHomeAsUpEnabled(true);

        Fragment fragment = getFragmentManager().findFragmentById(android.R.id.content);
        if (fragment == null) {
            mDeviceSettingsFragment = new DeviceSettings();
            getFragmentManager().beginTransaction()
                .add(android.R.id.content, mDeviceSettingsFragment)
                .commit();
        } else {
            mDeviceSettingsFragment = (DeviceSettings) fragment;
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }
}
