package com.ponyandpony.mediafrost;

import android.content.Context;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

public class SettingsActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

	public EditTextPreference mServer;
	public EditTextPreference mDiscoveryPort;
	public EditTextPreference mMinFiles;
	public EditTextPreference mMaxFiles;
	public EditTextPreference mTargets;
	public EditTextPreference mExtras;
	public EditTextPreference mUseTime;
	public EditTextPreference mLastTime;
	
	EditTextPreference createPreference(String key, String title, boolean useSummaryUpdater)
	{
		EditTextPreference pref = new EditTextPreference(this);
		pref.setKey(key);
		pref.setTitle(title); pref.setDialogTitle(title);
		String setting = FullscreenActivity.getSetting(this, key, "");
		pref.setSummary(setting); pref.setText(setting);
		if (useSummaryUpdater) pref.setOnPreferenceChangeListener(this);
		return pref;
	}
	
	public boolean onPreferenceChange(Preference preference, Object newValue)
	{
		preference.setSummary(newValue.toString());
		return true;
	}
	
	CheckBoxPreference createPreferenceBool(String key, String title)
	{
		CheckBoxPreference pref = new CheckBoxPreference(this);
		pref.setKey(key);
		pref.setTitle(title); 
		String setting = FullscreenActivity.getSetting(this, key, "");
		pref.setSummary(setting); pref.setChecked(setting.equalsIgnoreCase("yes"));
		return pref;
	}
	

	@SuppressWarnings("deprecation")
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// http://developer.android.com/reference/android/preference/PreferenceActivity.html
		addPreferencesFromResource(R.layout.activity_settings);
		//XmlPullParser parser = resources.getXml(myResouce); AttributeSet attributes = Xml.asAttributeSet(parser);
		PreferenceScreen screen = getPreferenceScreen();
		
		{
			mServer = createPreference("Server", "Server Address", true);
			screen.addPreference(mServer);
			mDiscoveryPort = createPreference("DiscoveryPort", "Discovery Port", true);
			screen.addPreference(mDiscoveryPort);
			mMinFiles = createPreference("MinFiles", "Minimum Files to Backup", true);
			screen.addPreference(mMinFiles);
			mMaxFiles = createPreference("MaxFiles", "Maximum Files to Backup", true);
			screen.addPreference(mMaxFiles);
			mTargets = createPreference("Targets", "Backup Targets", true);
			screen.addPreference(mTargets);
			mExtras = createPreference("Extras", "Extras", true);
			screen.addPreference(mExtras);
			mUseTime = createPreference("UseTime", "Use Date", true);
			screen.addPreference(mUseTime);
			mLastTime = createPreference("LastTime", "Last Date", true);
			screen.addPreference(mLastTime);
		}
	}
};
