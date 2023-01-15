package com.dragon.parts;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.Context;
import android.content.res.Resources;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragment;

import java.util.ArrayList;
import java.util.Arrays;

import com.dragon.parts.R;

public class DeviceSettings extends PreferenceFragment implements
        Preference.OnPreferenceChangeListener {

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
        Log.i(LOG_TAG, "WiFi country code changed to " + ccode + ", will take effect on next reboot");
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
