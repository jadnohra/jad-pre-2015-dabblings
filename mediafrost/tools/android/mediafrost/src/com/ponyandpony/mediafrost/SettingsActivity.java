package com.ponyandpony.mediafrost;

import java.util.ArrayList;
import java.util.List;

import android.annotation.TargetApi;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.util.Log;
import android.widget.Button;

@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class SettingsActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

	public EditTextPreference mServer;
	public EditTextPreference mDiscoveryPort;
	public EditTextPreference mMinFiles;
	public EditTextPreference mMaxFiles;
	public EditTextPreference mTargets;
	//public EditTextPreference mSources;
	public EditTextPreference mExtras;
	//public SwitchPreference mUseTime;
	//public EditTextPreference mLastTime;
	public SwitchPreference mNotify;
	public SwitchPreference mClearConsole;
	
	public List<SwitchPreference> mSources;
	public List<SwitchPreference> mUseTime;
	public List<EditTextPreference> mLastTime;
	
	EditTextPreference createPreference(Context ctx, String key, String title, Preference.OnPreferenceChangeListener cl)
	{
		EditTextPreference pref = new EditTextPreference(ctx);
		pref.setKey(key);
		pref.setTitle(title); pref.setDialogTitle(title);
		String setting = FullscreenActivity.getSetting(ctx, key, "");
		pref.setSummary(setting); pref.setText(setting);
		if (cl != null) pref.setOnPreferenceChangeListener(cl);
		return pref;
	}
	
	SwitchPreference createPreferenceToggle(Context ctx, String key, String title, Preference.OnPreferenceChangeListener cl)
	{
		SwitchPreference pref = new SwitchPreference(ctx);
		pref.setKey(key);
		pref.setTitle(title); //pref.setDialogTitle(title);
		//String setting = FullscreenActivity.getSettingBool(ctx, key, "");
		//pref.setSummary(setting); //pref.setText(setting);
		//if (cl != null) pref.setOnPreferenceChangeListener(cl);
		return pref;
	}
	
	public boolean onPreferenceChange(Preference preference, Object newValue)
	{
		preference.setSummary(newValue.toString());
		return true;
	}

	  public static class Prefs1aFragment extends PreferenceFragment {
	        @Override
	        public void onCreate(Bundle savedInstanceState) {
	            super.onCreate(savedInstanceState);

	            addPreferencesFromResource(R.xml.fragemented_preferences);
	            
	            
	            SettingsActivity act = (SettingsActivity) getActivity();
	            PreferenceScreen screen = getPreferenceScreen();

	            {
					act.mSources = new ArrayList<SwitchPreference>();

					for (int i=0; i<act.mBucketInfos.bucketNames.size(); i++) {
						String buckName = act.mBucketInfos.bucketNames.get(i);
						{
							SwitchPreference pref = act.createPreferenceToggle(act, "Source_"+buckName, act.mBucketInfos.bucketNames.get(i), act);
							screen.addPreference(pref);
							act.mSources.add(pref);
						}
					}
				}
	        }
	    }
	  
	  public static class Prefs1bFragment extends PreferenceFragment {
	        @Override
	        public void onCreate(Bundle savedInstanceState) {
	            super.onCreate(savedInstanceState);

	            addPreferencesFromResource(R.xml.fragemented_preferences);
	            
	            
	            SettingsActivity act = (SettingsActivity) getActivity();
	            PreferenceScreen screen = getPreferenceScreen();

	            {
					act.mUseTime = new ArrayList<SwitchPreference>();
					act.mLastTime = new ArrayList<EditTextPreference>();
					
					for (int i=0; i<act.mBucketInfos.bucketNames.size(); i++) {
						String buckName = act.mBucketInfos.bucketNames.get(i);
						
						{
							SwitchPreference pref = act.createPreferenceToggle(act, "UseTime_"+buckName, buckName, act);
							screen.addPreference(pref);
							act.mUseTime.add(pref);
						}
						
						{
							EditTextPreference pref = act.createPreference(act, "LastTime_"+buckName, "Last Date", act);
							screen.addPreference(pref);
							act.mLastTime.add(pref);
						}
					}
				}
	        }
	    }
	  
	  public static class Prefs2Fragment extends PreferenceFragment {
	        @Override
	        public void onCreate(Bundle savedInstanceState) {
	            super.onCreate(savedInstanceState);

	            // Load the preferences from an XML resource
	            addPreferencesFromResource(R.xml.fragemented_preferences2);
	        
	            SettingsActivity act = (SettingsActivity) getActivity();
	            PreferenceScreen screen = getPreferenceScreen();
				
	            act.mTargets = act.createPreference(act, "Targets", "Backup Targets", act);
				screen.addPreference(act.mTargets);

				act.mMinFiles = act.createPreference(act, "MinFiles", "Minimum Files to Backup", act);
				screen.addPreference(act.mMinFiles);
				act.mMaxFiles = act.createPreference(act, "MaxFiles", "Maximum Files to Backup", act);
				screen.addPreference(act.mMaxFiles);

				
				act.mServer = act.createPreference(act, "Server", "Server Address", act);
				screen.addPreference(act.mServer);
				act.mDiscoveryPort = act.createPreference(act, "DiscoveryPort", "Discovery Port", act);
				screen.addPreference(act.mDiscoveryPort);

				act.mNotify = act.createPreferenceToggle(act, "Notify", "Notify", act);
				screen.addPreference(act.mNotify);
				
				act.mClearConsole = act.createPreferenceToggle(act, "ClearConsole", "Clear Console", act);
				screen.addPreference(act.mClearConsole);
				
				act.mExtras = act.createPreference(act, "Extras", "Extras", act);
				screen.addPreference(act.mExtras);
	        }
	    }
	  
	  @Override
	    public void onBuildHeaders(List<Header> target) {
	        loadHeadersFromResource(R.xml.preference_headers, target);
	    }

	  @Override
	public boolean  isValidFragment(String fragmentName) { return true; }

	  FullscreenActivity.BucketInfos mBucketInfos;  
	  
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		mBucketInfos = FullscreenActivity.getBuckets(this);
		
		 // Add a button to the header list.
		/*
        if (hasHeaders()) {
            Button button = new Button(this);
            button.setText("Some action");
            setListFooter(button);
           
        }*/

	}
};
