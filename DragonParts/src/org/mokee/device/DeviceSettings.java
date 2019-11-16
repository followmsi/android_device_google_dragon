/*
* Copyright (C) 2018 The Mokee Project
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*/
package org.mokee.device;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.util.Log;
import androidx.preference.PreferenceFragment;
import androidx.preference.ListPreference;
import androidx.preference.Preference;

import java.util.ArrayList;
import java.util.Arrays;

public class DeviceSettings extends PreferenceFragment implements Preference.OnPreferenceChangeListener {

    private static final String WIFI_COUNTRY_CODE_KEY = "wifi_countrycode";

    private static final String WIFI_CCODE_SYSTEM_PROPERTY = "persist.sys.wifi.country_code";

    private static final String LOG_TAG = "WifiCountryCodePref";

    private ListPreference mWifiCCodePref;
    private WifiManager mWifiManager;

    private String mCurrentCcode;
    private String mCurrentCcodeDesc;

    private ArrayList<String> mCountryCodeList;
    private ArrayList<String> mCountryCodeDescList;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.main, rootKey);
        mWifiCCodePref = (ListPreference) findPreference(WIFI_COUNTRY_CODE_KEY);
        mWifiCCodePref.setOnPreferenceChangeListener(this);

        Context context = getActivity();
        mWifiCCodePref.setSummary(context.getString(R.string.wifi_country_summary, mCurrentCcodeDesc));
        mWifiCCodePref.setValue(mCurrentCcode);
    }

    private void setWifiCountryCode(String ccode) {
        SystemProperties.set(WIFI_CCODE_SYSTEM_PROPERTY, ccode);

        try {
            mWifiManager.setCountryCode(ccode);

            if (mWifiManager.isWifiEnabled()) {
                /* Country code takes effect after WiFi restart */
                new Handler().post(new Runnable() {
                    @Override
                    public void run() {
                        mWifiManager.setWifiEnabled(false);
                        try {
                            Thread.sleep(3000);
                        } catch (InterruptedException e) {
                            /* noop */
                        } finally {
                            mWifiManager.setWifiEnabled(true);
                        }
                    }
                });
            }

            refreshCurrentCcode();

            Context context = getActivity();
            mWifiCCodePref.setSummary(context.getString(R.string.wifi_country_summary, mCurrentCcodeDesc));
        } catch (NullPointerException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "Could not set WiFi country code to " + ccode + ". " + e.getLocalizedMessage());
            e.printStackTrace();
            return;
        }

        Log.i(LOG_TAG, "WiFi country code changed to " + ccode);
    }

    private String ccodeToDescription(String ccode) {
        int index = mCountryCodeList.indexOf(ccode);
        return index != -1 ? mCountryCodeDescList.get(index) : ccode;
    }

    private void refreshCurrentCcode() {
        mCurrentCcode = mWifiManager.getCountryCode();
        mCurrentCcodeDesc = ccodeToDescription(mCurrentCcode);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        final String key = preference.getKey();
        String value;
        if (WIFI_COUNTRY_CODE_KEY.equals(key)) {
            setWifiCountryCode((String) newValue);
        }
        return true;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mWifiManager = (WifiManager) activity.getSystemService(Context.WIFI_SERVICE);

        Resources res = activity.getResources();
        mCountryCodeList = new ArrayList(Arrays.asList(res.getStringArray(R.array.wifi_countrycode_values)));
        mCountryCodeDescList = new ArrayList(Arrays.asList(res.getStringArray(R.array.wifi_countrycode_entries)));
        refreshCurrentCcode();
    }
}
