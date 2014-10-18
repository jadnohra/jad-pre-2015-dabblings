package com.ponyandpony.mediafrost;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.UnsupportedEncodingException;
import java.net.ConnectException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.TreeSet;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.NotificationManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.EditTextPreference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ViewFlipper;
import android.content.ClipData;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Paint.Style;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.support.v4.app.NotificationCompat;
import android.text.Layout.Alignment;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.text.format.Time;
import android.util.Log;
import android.content.Context;

public class FullscreenActivity extends Activity {

	class StatusTextViews {
		TextView time;
		TextView activity;
		TextView console;
		ProgressBar progress;
		ProgressBar progress1;
		ProgressBar progress2;
		ProgressBar progress3;
		Button backup;
		Button cancel;
	};

	ViewFlipper mViewFlipper;
	StatusTextViews mStatusTextViews;
	SettingsActivity mSettingsActivity;
	
	List<String> mSources;
	
	enum Status
	{
		Idle,
		Working,
	};
	Status mStatus;
	int mWorkCount;
	NetworkThread mWorkThread;

	static public void setSetting(Context context, String key, String val) 
	{ 
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		SharedPreferences.Editor editor = prefs.edit();
		editor.putString(key, val); editor.commit();
	}
	
	static public String getSetting(Context context, String key, String dft) 
	{ 
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		if (prefs.contains(key) == false)
			setSetting(context, key, dft);
		return prefs.getString(key, dft); 
	}
	
	static public String getSetting(Context context, String key) { return getSetting(context, key, null); }
	static public String nonEmptySetting(String str, String dft) { if (str == null || str.length() == 0) return dft; return str; }
	
	static public void setSettingBool(Context context, String key, Boolean val) 
	{ 
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		SharedPreferences.Editor editor = prefs.edit();
		editor.putBoolean(key, val); editor.commit();
	}
	
	static public Boolean getSettingBool(Context context, String key, Boolean dft) 
	{ 
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		if (prefs.contains(key) == false)
			setSettingBool(context, key, dft);
		return prefs.getBoolean(key, dft); 
	}
	
	static public Boolean getSettingBool(Context context, String key) { return getSettingBool(context, key, null); }
	static public Boolean nonEmptySettingBool(Boolean str, Boolean dft) { if (str == null) return dft; return str; }
	
	
	@SuppressLint("InlinedApi")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mStatus = Status.Idle;
		mWorkCount = 0;
		
		{
			getSetting(this, "Server", "192.168.1.4:24107");
			getSetting(this, "MinFiles", "30");
			getSetting(this, "MaxFiles", "9999");
			getSetting(this, "DiscoveryPort", "1600");
			getSetting(this, "Targets", "vault_lena,vault_jad");
			//getSetting(this, "Sources", "camera");
			getSetting(this, "UseTime", "Yes");
			
			{
				{
					mSources = getBucketNames(this);
					for (int i=0; i<mSources.size(); i++) {
						boolean bkp = false;
						if (mSources.get(i).equalsIgnoreCase("Camera")) bkp = true;
						//setSettingBool(this, "Source_"+mSources.get(i), Boolean.valueOf(bkp));
						getSettingBool(this, "Source_"+mSources.get(i), Boolean.valueOf(bkp));
					}
				}
			}
		}
		
		setContentView(R.layout.activity_fullscreen);
		findViewById(R.id.backup_button).setOnTouchListener(mTouchListener);
		findViewById(R.id.cancel_button).setOnTouchListener(mTouchListener);
		mViewFlipper = (ViewFlipper) findViewById(R.id.viewFlipper1);
		mViewFlipper.setOnTouchListener(new ActivitySwipeDetector(this));

		mStatusTextViews = new StatusTextViews();
		{
			RelativeLayout layout = (RelativeLayout) findViewById(R.id.statusTextFieldsLayout1);
			int firstId = 1000;
			for (int i = 0; i < 2; i++) {
				TextView valueTV = new TextView(this);
				valueTV.setText("");
				valueTV.setId(firstId + i);
				valueTV.setGravity(Gravity.CENTER_HORIZONTAL);
				valueTV.setTextAppearance(this,
						android.R.style.TextAppearance_Large);
				RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
						RelativeLayout.LayoutParams.MATCH_PARENT,
						RelativeLayout.LayoutParams.WRAP_CONTENT);
				if (i > 0)
					lp.addRule(RelativeLayout.BELOW, firstId + i - 1);
				layout.addView(valueTV, lp);

				if (i == 0)
					mStatusTextViews.time = valueTV;
				if (i == 1)
					mStatusTextViews.activity = valueTV;
			}
			mStatusTextViews.activity.setTextAppearance(this,
					android.R.style.TextAppearance_Medium);
			mStatusTextViews.console = (TextView) findViewById(R.id.consoleView2);
			//mStatusTextViews.progress = (ProgressBar) findViewById(R.id.progressBar1);
			//mStatusTextViews.progress.setVisibility(View.INVISIBLE);
			mStatusTextViews.backup = (Button) findViewById(R.id.backup_button);
			mStatusTextViews.cancel = (Button) findViewById(R.id.cancel_button);
			mStatusTextViews.cancel.setVisibility(View.INVISIBLE);
			mStatusTextViews.progress1 = (ProgressBar) findViewById(R.id.progressBar2);
			mStatusTextViews.progress1.setVisibility(View.INVISIBLE);
			mStatusTextViews.progress2 = (ProgressBar) findViewById(R.id.progressBar3);
			mStatusTextViews.progress2.setVisibility(View.INVISIBLE);
			//mStatusTextViews.progress3 = (ProgressBar) findViewById(R.id.progressBar4);
			//mStatusTextViews.progress3.setVisibility(View.INVISIBLE);
		}

		{
			mSettingsActivity = new SettingsActivity();
		}
	
		if (getSetting(this, "Extras", "").contains("dbg_buckets"))
		{
			List<String> names = getBucketNames(this);
			for (int i=0; i<names.size(); ++i)
			{
				Log.v("Testing", String.format(names.get(i)));	
			}
		}
		
		if (android.os.Build.VERSION.SDK_INT >= 18)
		{
			if (getSetting(this, "Extras", "").contains("dbg_sel"))
			{
				Intent intent = new Intent();
				intent.setType("image/*");
				intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
				intent.setAction(Intent.ACTION_GET_CONTENT);
				startActivityForResult(Intent.createChooser(intent,"Select Picture"), 1);
			}
		}
	}

	@Override
	protected void onPostCreate(Bundle savedInstanceState) {
		super.onPostCreate(savedInstanceState);
	}

	class ActivitySwipeDetector implements View.OnTouchListener {

		static final String logTag = "ActivitySwipeDetector";
		private FullscreenActivity activity;
		static final int MIN_DISTANCE = 100;
		private float downX, downY, upX, upY;

		public ActivitySwipeDetector(FullscreenActivity activity) {
			this.activity = activity;
		}

		public void onRightToLeftSwipe() {
			if (activity.mWorkCount > 0 || activity.mStatus == Status.Working)
				activity.mViewFlipper.showPrevious();
		}

		public void onLeftToRightSwipe() {
			if (activity.mWorkCount > 0 || activity.mStatus == Status.Working)
				activity.mViewFlipper.showNext();
		}

		public void onTopToBottomSwipe() {
			Intent intent = new Intent();
			intent.setClass(FullscreenActivity.this, SettingsActivity.class);
			startActivityForResult(intent, 0);
		}

		public void onBottomToTopSwipe() {
		}

		public boolean onTouch(View v, MotionEvent event) {
			switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN: {
				downX = event.getX();
				downY = event.getY();
				return true;
			}
			case MotionEvent.ACTION_UP: {
				upX = event.getX();
				upY = event.getY();

				float deltaX = downX - upX;
				float deltaY = downY - upY;

				// swipe horizontal?
				if (Math.abs(deltaX) > MIN_DISTANCE) {
					// left or right
					if (deltaX < 0) {
						this.onLeftToRightSwipe();
						return true;
					}
					if (deltaX > 0) {
						this.onRightToLeftSwipe();
						return true;
					}
				} else {
					Log.i(logTag, "Swipe was only " + Math.abs(deltaX)
							+ " long, need at least " + MIN_DISTANCE);
					// return false; // We don't consume the event
				}

				// swipe vertical?
				if (Math.abs(deltaY) > MIN_DISTANCE) {
					// top or down
					if (deltaY < 0) {
						this.onTopToBottomSwipe();
						return true;
					}
					if (deltaY > 0) {
						this.onBottomToTopSwipe();
						return true;
					}
				} else {
					Log.i(logTag, "Swipe was only " + Math.abs(deltaY)
							+ " long, need at least " + MIN_DISTANCE);
					// return false; // We don't consume the event
				}

				return false;
			}
			}
			return false;
		}
	};

	@SuppressLint("NewApi")
	public String getRealPathFromURI(Uri uri){
	   Cursor cursor = getContentResolver().query(uri, null, null, null, null);
	   cursor.moveToFirst();
	   String document_id = cursor.getString(0);
	   document_id = document_id.substring(document_id.lastIndexOf(":")+1);
	   cursor.close();

	   cursor = getContentResolver().query( 
	   android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
	   null, MediaStore.Images.Media._ID + " = ? ", new String[]{document_id}, null);
	   cursor.moveToFirst();
	   String path = cursor.getString(cursor.getColumnIndex(MediaStore.Images.Media.DATA));
	   cursor.close();

	   return path;
	}
	
	@SuppressLint("NewApi")
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		
		if (resultCode == Activity.RESULT_OK)	
		
	      if(requestCode == 12) {
	    	  List<String> images = new ArrayList<String>();
	    	 if (data.getData() != null) {
	    		 /* Disallow single selection (by mistake)
	    		  Uri imageUri = data.getData();
	    		  images.add(getRealPathFromURI( imageUri));
	    	  	*/
	    	  } else if (data.getClipData() != null) {
	    		  //Bitmap bitmap = MediaStore.Images.Media.getBitmap(this.getContentResolver(), clipdata.getItemAt(i).getUri());
	    		  ClipData clipdata = data.getClipData();
	    		  if (clipdata != null)
	    		  {
	                for (int i=0; i<clipdata.getItemCount();i++)
	                	images.add(getRealPathFromURI( clipdata.getItemAt(i).getUri()));
	    		  }   
	    	  }
	    	 if (images.size() > 0)
	    		 launchBackup(this, null, images, false);
	    	  //for (int i=0; i<images.size(); ++i) Log.v("Testing", String.format(images.get(i)));	
	      }
	      
	}
	
	
	
	@SuppressLint("SimpleDateFormat")
	public void launchBackup(Context context, List<String> sources, List<String> images, boolean automatic) {
			
			FullscreenActivity.this.mStatus = Status.Working;
			FullscreenActivity.this.mWorkCount++;
			
			mStatusTextViews.console.setText("");
			mStatusTextViews.time.setText("00:00:00");
			mStatusTextViews.activity.setText("Connecting...");
			//mStatusTextViews.progress.setVisibility(View.VISIBLE);
			mStatusTextViews.cancel.setVisibility(View.VISIBLE);
			mStatusTextViews.progress1.setVisibility(View.VISIBLE);
			mStatusTextViews.progress1.setProgress(0);
			mStatusTextViews.progress2.setVisibility(View.VISIBLE);
			mStatusTextViews.progress2.setProgress(0);
			//mStatusTextViews.progress3.setVisibility(View.VISIBLE);
			//mStatusTextViews.progress3.setProgress(0);

			NetworkThreadSettings settings = new NetworkThreadSettings();
			String address[] = getSetting(context, "Server", null).split(":");
			settings.server = address[0];
			settings.port = Integer.parseInt(address[1]);
			
			settings.dport = Integer.parseInt(nonEmptySetting(getSetting(context, "DiscoveryPort"), "1600"));
			settings.targets = getSetting(context, "Targets", "");
			if (automatic == false) {
				settings.minFiles = Integer.parseInt(getSetting(context, "MinFiles"));
				settings.maxFiles = Integer.parseInt(getSetting(context, "MaxFiles"));
				settings.stamp = true;
			} else {
				settings.minFiles = 0;
				settings.maxFiles = images.size();
				settings.stamp = false;
			}
				
			long minTime = 0;
			if (automatic && getSetting(context, "UseTime", "").equalsIgnoreCase("yes"))
			{
				try
				{
					minTime = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss").parse(getSetting(context, "LastTime", null)).getTime();
				} catch (Exception e) {}
			}
			settings.minTime = minTime;
			
			NetworkThread thread = new NetworkThread(mThreadMessageHandler, settings, sources, images, mStatusTextViews);
			mWorkThread = thread;
			thread.start();
	}

	View.OnTouchListener mTouchListener = new View.OnTouchListener() {
		
		boolean isHolding = false;
		boolean triggeredHold = false;
		
		@SuppressLint("InlinedApi")
		@Override
		public boolean onTouch(View view, MotionEvent evt) {

			isHolding = false;
			if (view.getId() == R.id.backup_button) {
				
				if (evt.getAction() == MotionEvent.ACTION_DOWN) {
					
					isHolding = true;
					triggeredHold = false;
					Runnable holdRunnable = new Runnable() {
						public void run() {
							if (isHolding) {
								triggeredHold = true;
								Intent intent = new Intent();
								intent.setType("image/*");
								if (android.os.Build.VERSION.SDK_INT >= 18)
									intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
								intent.setAction(Intent.ACTION_GET_CONTENT);
								startActivityForResult(Intent.createChooser(intent,"Select Files"), 12);
							}
						}
					};
					
					Handler holdHandler = new Handler();
					holdHandler.postDelayed(holdRunnable, 1000);
					
				} else if (evt.getAction() == MotionEvent.ACTION_UP) {
					
					if (!triggeredHold) {
						Context context = view.getContext();
						{	
							List<String> images = getCameraImages(context);
							launchBackup(context, images, true);
	
							return true;
						}
					}
					triggeredHold = false;
				}	
			}
			else if (view.getId() == R.id.cancel_button && evt.getAction() == MotionEvent.ACTION_DOWN) {
			
				if (mWorkThread != null && mWorkThread.mSocket != null)
				{
					try
					{
						mWorkThread.mUserCancelled = true;
						mWorkThread.mSocket.close();
					} catch (Exception e)
					{}
				}
				
				return true;
			}
			
			return false;
		}
	};
	
	static void prepareImages(Context context, List<String> images)
	{
		boolean dbg = false;
		if (dbg && images.size() > 0) {
			String hash = genMD5Hash(images.get(0));
			Log.v("Testing", images.get(0));
			Log.v("Testing", hash);
		}
		
		boolean dbg_stamp = getSetting(context, "Extras", "").contains("dbg_stamp");
		if (dbg_stamp)
		{
			int ti = 0;
			for (int i=0; i<images.size(); ++i)
				if (!images.get(i).contains("jpg"))
					{	ti = i; break; }
			stampSuccess(context, images.get(ti));
			long time = new File(images.get(ti)).lastModified();
			setSetting(context, "LastTime", new SimpleDateFormat("dd-MM-yyyy HH:mm:ss").format(time));
			return;
		}
		
		boolean dbg_sort = getSetting(context, "Extras", "").contains("dbg_sort");
		if (dbg_sort)
		{
			class ImageComp implements Comparator<String>
			{
				public int compare(String f1, String f2) {
					long t1 = new File(f1).lastModified();
					long t2 = new File(f2).lastModified();
		  			return t1>t2?1:(t1<t2?-1:0);
		  		}
			};
			Collections.sort(images, new ImageComp());
			
			for (int i=0; i<images.size(); ++i)
			{
				Log.v("Testing", String.format(images.get(i)));	
			}
			return;
		}
		
		Iterator<String> iter = images.iterator();
		while (iter.hasNext()) {
		    if (iter.next().startsWith(getStampPrefix())) {
		        iter.remove();
		    }
		}
	}

	public Handler mThreadMessageHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			
			if (msg.what == 1)
			{
				FullscreenActivity act = FullscreenActivity.this;
				act.mStatus = Status.Idle;
				act.mStatusTextViews.backup.setEnabled(true);
				act.mWorkThread = null;
				boolean succeeded = msg.getData().containsKey("Succeeded") && msg.getData().getBoolean("Succeeded");
				if (succeeded && msg.getData().containsKey("LastTime") && msg.getData().getString("LastTime") != null)
					act.setSetting(act, "LastTime", msg.getData().getString("LastTime"));
			}
		}
	};
	
	class NetworkThreadSettings
	{
		String server;
		int port;
		int minFiles;
		int maxFiles;
		int dport;
		String targets;
		long minTime;
		boolean stamp;
	};
	
	public class NetworkThread extends Thread {
		
		NetworkThreadSettings mSettings;
		Handler mMessageHandler;
		BucketInfos mBucketInfos;
		List<String> mSources;
		List<String> mImages;
		long mTotalByteCount;
		StatusTextViews mStatusTextViews;
		Handler mTimerHandler;
		Runnable mTimerRunnable;
		String mStatusText;
		Socket mSocket;
		boolean mUserCancelled;
		boolean mSucceeded;
		String mFailString;
		String mLastTime;

		NetworkThread(Handler messageHandler, NetworkThreadSettings settings, BucketInfos bucketInfos, List<String> sources, List<String> images, StatusTextViews statusTextViews) {
			mMessageHandler = messageHandler;
			mSettings = settings;
			mBucketInfos = bucketInfos;
			mSources = sources;
			mImages = images;
			mStatusTextViews = statusTextViews;
			mStatusText = "";
			mUserCancelled = false;
			mSucceeded = false;
			mFailString = "";
			mLastTime = null;

			mTimerHandler = new Handler();
			final NetworkThread mThis = this;
			mTimerRunnable = new Runnable() {
				int secs = 0;

				public void run() {
					secs = secs + 1;
					int hours = secs / 3600;
					int minutes = (secs % 3600) / 60;
					int seconds = secs % 60;
					mThis.setText(mThis.mStatusTextViews.time, String.format(
							"%02d:%02d:%02d", hours, minutes, seconds));
					mTimerHandler.postDelayed(this, 1000);
				}
			};
			mTimerHandler.postDelayed(mTimerRunnable, 1000);
		}

		class ByteSocketBuffer {

			ArrayList<byte[]> m_arrays = new ArrayList<byte[]>();
			int m_totalLength = 0;
			int m_offset = 0;

			void add(byte[] data, int size) throws UnsupportedEncodingException {
				if (size <= 0)
					return;

				byte[] sdata = new byte[size];
				System.arraycopy(data, 0, sdata, 0, size);
				m_arrays.add(sdata);
				m_totalLength += size;

				// String dbg = new String(sdata, "US-ASCII");
				// Log.v("Testing", String.format("Buf: [%s]", dbg));
			}

			int available() {
				return m_totalLength - m_offset;
			}

			boolean startsWith(String str) throws UnsupportedEncodingException {
				if (available() < str.length())
					return false;
				byte[] strBytes = str.getBytes("US-ASCII");

				int leftBytes = str.length();
				int arrIndex = 0;
				int offset = m_offset;
				while (leftBytes > 0) {
					byte[] compBytes = m_arrays.get(arrIndex);
					for (int i = offset; leftBytes > 0 && i < compBytes.length; i++, leftBytes--)
						if (strBytes[str.length() - leftBytes] != compBytes[i])
							return false;
					arrIndex++;
					offset = 0;
				}
				return true;
			}

			void skip(int length) throws UnsupportedEncodingException {
				while (length > 0) {
					int arrLength = m_arrays.get(0).length;
					if (arrLength - m_offset > length) {
						m_offset += length;
						break;
					}
					length -= arrLength - m_offset;
					m_totalLength -= arrLength;
					m_offset = 0;
					m_arrays.remove(0);
				}
				// debug(32);
			}

			byte[] peek(int length) {
				//Log.v("Testing", String.format("peek0 length:%d arrays:%d off:%d total:%d", length, m_arrays.size(), m_offset, m_totalLength));
				byte[] outBytes = new byte[length];
				int leftBytes = length;
				int arrIndex = 0;
				int offset = m_offset;
				while (leftBytes > 0) {
					byte[] readBytes = m_arrays.get(arrIndex);
					int readCount = Math.min(leftBytes, readBytes.length-offset);
					//Log.v("Testing", String.format("peek1 left:%d offs:%d offd:%d cnt:%d", leftBytes, offset, length-leftBytes, readCount));
					
					System.arraycopy(readBytes, offset, outBytes, length - leftBytes, readCount);
					leftBytes -= readCount;
					arrIndex++;
					offset = 0;
				}
				//Log.v("Testing", "peek2");
				return outBytes;
			}

			byte[] read(int length) throws UnsupportedEncodingException {
				byte[] outBytes = peek(length);
				skip(length);
				return outBytes;
			}

			void debug(int length) throws UnsupportedEncodingException {
				byte[] dbgBytes = peek(Math.min(available(), length));
				String dbgStr = new String(dbgBytes, "US-ASCII");
				//Log.v("Testing", String.format("PeekBuf: [%s]", dbgStr));
			}
		};

		void setText(final TextView view, final String str) {
			if (view != null)
				view.post(new Runnable() {
					public void run() {
						view.setText(str);
					}
				});
		}
		
		void setProgress(final ProgressBar bar, final int percentage)
		{
			if (bar != null)
				bar.post(new Runnable() {
					public void run() {
						bar.setProgress(Math.max(5, percentage));
					}
				});
		}
		
		void openProgress(final ProgressBar bar)
		{
			if (bar != null)
				bar.post(new Runnable() {
					public void run() {
						bar.setProgress(5);
					}
				});
		}
		
		void closeProgress(final ProgressBar bar)
		{
			if (bar != null)
				bar.post(new Runnable() {
					public void run() {
						bar.setProgress(100);
					}
				});
		}
		
		void stopProgress() {
			if (mStatusTextViews.progress1 != null)
				mStatusTextViews.progress1.post(new Runnable() {
					public void run() {
						//mStatusTextViews.progress.setVisibility(View.INVISIBLE);
						mStatusTextViews.progress1.setVisibility(View.INVISIBLE);
						mStatusTextViews.progress2.setVisibility(View.INVISIBLE);
						mStatusTextViews.cancel.setVisibility(View.INVISIBLE);
					}
				});
			if (mMessageHandler != null)
			{
				Message msg = Message.obtain(mMessageHandler, 1, 0, 0);
				Bundle data = new Bundle(); 
				data.putBoolean("Succeeded", mSucceeded);
				if (mSettings.stamp)
					data.putString("LastTime", mLastTime);
				msg.setData(data);
				msg.sendToTarget();
			}
		}

		void logStatus(final String str, final boolean eol) {
			//Log.v("Testing", str);
			mStatusText = String.format("%s%s%s", mStatusText, str, eol?"\n":"");
			setText(mStatusTextViews.activity, str);
			setText(mStatusTextViews.console, mStatusText);
		}
		void logStatus(final String str) { logStatus(str, true); }

		void logNotify(final String str)
		{
			if (true)
			{
				NotificationCompat.Builder builder =
					    new NotificationCompat.Builder(FullscreenActivity.this)
					    .setSmallIcon(R.drawable.ic_launcher)
					    .setContentTitle("Mediafrost")
					    .setContentText(str);
	
				long pattern[] = {500, 500, 500};
				builder.setVibrate(pattern);
				builder.setLights(0xFFFFFFFF,500,3000);
				NotificationManager notifyMgr =  (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
				notifyMgr.notify(1, builder.build());
			}
		}
		
		public void run() {
			if (mImages != null)
			{
				if (mImages.size() > 0)
					runBatch(mImages);
			}
			if (mSources != null)
			{
				for (int i=0; i<mSources.size(); i++)
				{
					logStatus(String.format("Backing up '%s'.", mSources.get(i)));
					List<String> images = getMediaFiles(FullscreenActivity.this, mBucketInfos, mSources.get(i));
					if (images.size() > 0)
						runBatch(images);
				}
			}
			
		}
			
		public void runBatch(List<String> images) {	
			{
				{
					Iterator<String> iter = images.iterator();
					while (iter.hasNext()) {
					    if (iter.next().startsWith(getStampPrefix())) {
					        iter.remove();
					    }
					}
				}
				{					
					if (mSettings.minTime > 0)
					{
						int removed = 0;
						long minTime = mSettings.minTime - 1000;
						Iterator<String> it = images.iterator();
						while (it.hasNext()) {
							String f = it.next();
							if(new File(f).lastModified() < minTime)
								{ it.remove(); removed++; }
						}
						logStatus(String.format("Filtered out %d of %d files by date.", removed, images.size()+removed));
					}
					else
					{
						logStatus(String.format("Detected %d files.", images.size()));	
					}
					
					class ImageComp implements Comparator<String>
					{
						public int compare(String f1, String f2) {
							long t1 = new File(f1).lastModified();
							long t2 = new File(f2).lastModified();
				  			return t1>t2?1:(t1<t2?-1:0);
				  		}
					};
					Collections.sort(images, new ImageComp());
				}
			}
			
			Socket sock = null;
			try {
				
				String server = mSettings.server; int port = mSettings.port; int dport = mSettings.dport;
				{
					logStatus("Discovering ... ", false);
					DatagramSocket dsock = new DatagramSocket(2600);
					{
						byte[] buffer = "/mediafrost:discover".getBytes();
						DatagramPacket packet = new DatagramPacket(buffer, buffer.length, new InetSocketAddress("255.255.255.255", dport));
						dsock.send(packet);
					}
					{
						byte[] buffer = "255.255.255.255:88888888".getBytes();
						DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
						dsock.setSoTimeout(1700);
						try
						{
							dsock.receive(packet);
							String msg = new String(packet.getData(), 0, packet.getLength());
							String[] words = msg.split(":");
							server = words[0];
							port = Integer.parseInt(words[1]);
							logStatus(String.format("%s:%d.", server, port));
						}
						catch(SocketTimeoutException e1) {
							logStatus("failed.");
						}
						catch (Exception e2) {
							logStatus(Log.getStackTraceString(e2));
							logStatus("failed.");
						}
					}
					
					dsock.close();
				}
				
				
				sock = new Socket();
				mSocket = sock;
				sock.connect(new InetSocketAddress(server, port), 0);
				if (sock.isConnected()) {
					logStatus(String.format("Connected to %s.", sock.getRemoteSocketAddress().toString()));
					
					int imageCount = images.size();
					if (imageCount > mSettings.maxFiles)
					{
						logStatus(String.format("Limiting %d to max. %d files.", images.size(), mSettings.maxFiles));
						imageCount = mSettings.maxFiles;
					}
					if (imageCount < mSettings.minFiles)
					{
						logStatus(String.format("%d files is below %d.", images.size(), mSettings.minFiles));
						imageCount = 0;
					}
					
					logStatus("Calculating total bytes ...", false);
					mTotalByteCount = 0;
					for (int i = 0; i < imageCount; i++) {
						String filePath = images.get(i);
						long bytes = new File(filePath).length();
						mTotalByteCount += bytes;
					}
					logStatus(String.format(" %d.", mTotalByteCount));
					
					//TODO: escape ':'
					sock.getOutputStream().write(String.format("/start:%s:eoc", mSettings.targets).getBytes("US-ASCII"));
					
					HashMap<String, Integer> fidMap = new HashMap<String, Integer>();
					ArrayList<String> requestedFids = new ArrayList<String>();
					{
						sock.setSoTimeout(100);
						InputStream istream = sock.getInputStream();
						byte[] bufRead = new byte[sock.getReceiveBufferSize()];
						ByteSocketBuffer bufCmd = new ByteSocketBuffer();

						boolean requesting = true;
						String cmdFrequest = "/frequest:";
						int fidLen = 64;
						String cmdEnd = "/frequestend";
						String cmdGo = "/go";
						while (requesting) {
							try {

								int bufSize = istream.read(bufRead);
								if (bufSize == -1)
									break;
								bufCmd.add(bufRead, bufSize);

								boolean consumed;
								do {
									consumed = false;

									if (bufCmd.startsWith(cmdGo)) {
										bufCmd.skip(cmdGo.length());
										consumed = false;
										{
											logStatus(String.format("Senging %d fids ...", imageCount));
											{
												openProgress(mStatusTextViews.progress1);
												float prog1 = 0.0f;
												for (int i = 0; i < imageCount; i++) {
													String filePath = images.get(i);
													File file = new File(filePath);
													String fileName = file.getName();
							
													setText(mStatusTextViews.activity,
															String.format("Hashing %s (%d/%d)", fileName, i+1, imageCount));
													String fid = genMD5Hash(filePath);
													fidMap.put(fid, Integer.valueOf(i));
													sock.getOutputStream().write(
															String.format("/fid:%s:%d:%s", fileName, file.length(), fid)
																	.getBytes("US-ASCII"));
													float lprog1 = prog1;
													prog1 += ((100.0f * (float)(file.length()))/((float) mTotalByteCount));
													if ((int) lprog1 != (int) prog1)
														setProgress(mStatusTextViews.progress1, (int) prog1);
												}
												closeProgress(mStatusTextViews.progress1);
											}
											
											sock.getOutputStream().write("/fidend".getBytes("US-ASCII"));
											sock.getOutputStream().flush();
											logStatus("Gathering file requests ...");
										}
									} else if ((bufCmd.available() >= (cmdFrequest.length() + fidLen))
											&& bufCmd.startsWith(cmdFrequest)) {
										bufCmd.skip(cmdFrequest.length());
										byte[] fidBytes = bufCmd.read(fidLen);
										String fid = new String(fidBytes,"US-ASCII");
										requestedFids.add(fid);
										consumed = true;
									} else if (bufCmd.startsWith(cmdEnd)) {
										requesting = false;
										consumed = false;
									}

								} while (consumed);

							} catch (InterruptedIOException e) {
							}
						}
					}
					
					
					if (requestedFids.size() > 0) {
						openProgress(mStatusTextViews.progress2);
						logStatus(String.format("Sending %d files ...", requestedFids.size()));
						float prog2 = 0.0f;
						for (int i = 0; i < requestedFids.size(); i++) {
							String fid = requestedFids.get(i);
							int index = fidMap.get(fid).intValue();
							String filePath = images.get(index);

							File file = new File(filePath);
							setText(mStatusTextViews.activity,
									String.format("Sending %s (%d/%d)", file.getName(), i+1, requestedFids.size()));
							FileInputStream fileInputStream = new FileInputStream(
									file);
							int fileSize = fileInputStream.available();
							sock.getOutputStream().write(String.format("/fdata:%s:%d:", fid, fileSize).getBytes("US-ASCII"));

							byte[] buffer = new byte[64*1024];
							int read = 0;
							while ((read = fileInputStream.read(buffer, 0,
									buffer.length)) != -1)
							{
								sock.getOutputStream().write(buffer, 0, read);
								float lprog2 = prog2;
								prog2 += ((100.0f * (float)(read))/((float) mTotalByteCount));
								if ((int) lprog2 != (int) prog2	)
									setProgress(mStatusTextViews.progress2, (int) prog2);
							}
							fileInputStream.close();
						}
						closeProgress(mStatusTextViews.progress2);
						sock.getOutputStream().write("/fdataend".getBytes("US-ASCII"));
						sock.getOutputStream().flush();
						logStatus("Waiting for backup ...");
					}
					{
						sock.setSoTimeout(1000);
						InputStream istream = sock.getInputStream();
						byte[] bufRead = new byte[sock
								.getReceiveBufferSize()];
						ByteSocketBuffer bufCmd = new ByteSocketBuffer();

						boolean requesting = true;
						String cmdSuccess = "/success:";
						int codeLen = 1;
						String cmdEnd = "/end";
						while (requesting) {
							try {

								int bufSize = istream.read(bufRead);
								if (bufSize == -1)
									break;
								bufCmd.add(bufRead, bufSize);

								boolean consumed;
								do {
									consumed = false;

									if ((bufCmd.available() >= (cmdSuccess.length() + codeLen))
											&& bufCmd.startsWith(cmdSuccess)) {
										bufCmd.skip(cmdSuccess.length());
										byte[] codeBytes = bufCmd.read(codeLen);
										String code = new String(codeBytes,"US-ASCII");
										mFailString = code;
										logStatus(String.format("Success: %s", code));
										
										if (code.equals("1"))
										{
											mSucceeded = true;
											String str = String.format("Backup succeeded (%d files).", requestedFids.size());
											logStatus(str);
											logNotify(str);
											
											if (mSettings.stamp && requestedFids.size() > 0)
											{
												int maxIndex = 0;
												{
													String fid = requestedFids.get(requestedFids.size()-1);
													int index = fidMap.get(fid).intValue();
													maxIndex = index;
												}
												
												for (int i=0; i<requestedFids.size(); i++)
												{
													String fid = requestedFids.get(i);
													int index = fidMap.get(fid).intValue();
													if (index > maxIndex) maxIndex = index;
												}
												
												String filePath = images.get(maxIndex);
												try
												{
													long time = new File(filePath).lastModified();
													mLastTime = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss").format(time);
												} catch (Exception e) {}
												stampSuccess(FullscreenActivity.this, filePath);
											}
										}
												
										consumed = true;
									} else if (bufCmd.startsWith(cmdEnd)) {
										requesting = false;
										consumed = false;
									}
								} while (consumed);

							} catch (InterruptedIOException e) {
							}
						}
					}
					sock.close();
					logStatus("Done.");
				}

			}
			catch (SocketTimeoutException e3) {
				
				logStatus(e3.toString());
				logNotify("Connect failed.");
			}
			catch (ConnectException e4) {
				
				logStatus(e4.toString());
				logNotify("Connect failed.");
			}
			catch(IOException e1) {
				
				logStatus(e1.toString());
				
				try
				{
					sock.close();
					logStatus("Failed (Exception).");
				}
				catch(Exception e2)
				{
					logStatus(Log.getStackTraceString(e2));
				}
				
			} catch (Exception e) {
				
				logStatus(Log.getStackTraceString(e));
				
				try
				{
					sock.close();
					logStatus("Failed (Exception).");
				}
				catch(Exception e2)
				{
					logStatus(Log.getStackTraceString(e2));
				}
			}
			
			if (mUserCancelled)
			{
				logStatus("Cancelled by user.");
			} 
			else if (!mSucceeded)
			{
				String code = (mFailString.length() > 0 ? String.format(" (%s)", mFailString) : "");
				String str = String.format("Backup failed%s.", code);
				logStatus(str);
				logNotify(str);
			}
			
			mTimerHandler.removeCallbacks(mTimerRunnable);
			stopProgress();
		}
	};

	public static String genMD5Hash(String filePath) {
		String ret = "";
		try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			File file = new File(filePath);
			FileInputStream fileInputStream = new FileInputStream(file);

			DigestInputStream dis = new DigestInputStream(fileInputStream, md);
			byte[] buffer = new byte[64*1024];
			while (dis.read(buffer, 0, buffer.length) != -1) {
			}
			dis.close();

			byte[] digest = md.digest();

			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < digest.length; ++i) {
				sb.append(Integer.toHexString((digest[i] & 0xFF) | 0x100)
						.substring(1, 3));
			}
			ret = sb.toString();

		} catch (Exception e) {
			Log.v("Testing", e.toString());
		}

		return ret;
	}

	public static String getBucketId(String path) {
		return String.valueOf(path.toLowerCase().hashCode());
	}
	
	static String CamFolder = "/DCIM/Camera";
	
	private static Bitmap createStampBitmap(int w, int h, String text) 
	{
		Bitmap b = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
		Canvas c = new Canvas(b);
		
		{
			c.drawColor(Color.TRANSPARENT);
		}
		{
			Paint paint = new Paint();
			paint.setStyle(Paint.Style.FILL_AND_STROKE);
			paint.setColor(0xAA331133);
			c.drawCircle(w/2, h/2, 0.45f*Math.min(w, h), paint);
			paint.setStyle(Paint.Style.STROKE);
			paint.setColor(Color.GREEN);
			paint.setStrokeWidth(0.03f*Math.min(w, h));
			c.drawCircle(w/2, h/2, 0.45f*Math.min(w, h), paint);
		}
		{
			Paint paint = new Paint();
			paint.setTypeface(Typeface.defaultFromStyle(Typeface.BOLD));
			paint.setTextSize(40);
		    paint.setTextScaleX(1.f);
		    paint.setAlpha(0);
		    paint.setAntiAlias(true);
		    paint.setColor(Color.GREEN);
		    paint.setStrokeWidth(24.0f);
		    Rect bounds = new Rect();
		    paint.getTextBounds(text,0,text.length(),bounds);
		    float scale =  (float) w / (1.25f * (float) bounds.width());
		    paint.setTextSize((int) ((float) paint.getTextSize() * scale));
		    paint.getTextBounds(text,0,text.length(),bounds);
		    c.rotate(45.0f, w/2, h/2+bounds.height()/2);
		    c.drawText(text, w/2-bounds.width()/2, h/2, paint);
		}
	    return b;
	}

	private static Bitmap stampOverlay(Bitmap bmp1, Bitmap bmp2) {
	    Bitmap bmOverlay = Bitmap.createBitmap(bmp1.getWidth(), bmp1.getHeight(), bmp1.getConfig());
	    Canvas canvas = new Canvas(bmOverlay);
	    canvas.drawBitmap(bmp1, new Matrix(), null);
	    canvas.drawBitmap(bmp2, 0, 0, null);
	    return bmOverlay;
	}
	
	private static int calculateInSampleSize(BitmapFactory.Options options, int reqWidth, int reqHeight) {
	    final int height = options.outHeight;
	    final int width = options.outWidth;
	    int inSampleSize = 1;
	    if (height > reqHeight || width > reqWidth) {
	        while (((height/2) / inSampleSize) > reqHeight
	                && ((width/2) / inSampleSize) > reqWidth) {
	            inSampleSize *= 2;
	        }
	    }
	    return inSampleSize;
	}
	
	private static Bitmap decodeSampledFileBitmap(String filePath, int reqWidth, int reqHeight) 
	{
	    final BitmapFactory.Options options = new BitmapFactory.Options();
	    options.inJustDecodeBounds = true;
	    BitmapFactory.decodeFile(filePath, options);
	    options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight);
	    options.inJustDecodeBounds = false;
	    return BitmapFactory.decodeFile(filePath, options);
	}
	
	private static boolean isImageFile(String filePath)
	{
		try
		{
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inJustDecodeBounds = true;
			BitmapFactory.decodeFile(filePath, options);
			return (options.outWidth > 0);
		} catch(Exception e) {}
		return false;
	}
	
	private static Bitmap tryExtractVideoThumb(String filePath)
	{
		Bitmap thumb = null;
		try
		{
			thumb = ThumbnailUtils.createVideoThumbnail(filePath, MediaStore.Images.Thumbnails.MINI_KIND);
		} catch(Exception e) {}
		return thumb;
	}
	
	private static String getStampPrefix()
	{
		return "mediafrost_stamp-";
	}
	
	@SuppressLint("SimpleDateFormat")
	public static void stampSuccess(Context context, String iconInputFilePath)
	{
		final String folder = Environment.getExternalStorageDirectory().toString() + CamFolder;
       
		
        Bitmap iconBmp;
        if (isImageFile(iconInputFilePath))
        {
        	iconBmp = decodeSampledFileBitmap(iconInputFilePath, 320, 320);
        }
        else
        {
        	iconBmp = tryExtractVideoThumb(iconInputFilePath);
        }
        
        String date = new SimpleDateFormat("dd-MM-yyyy").format(Calendar.getInstance().getTime());
        String stampText = String.format("- Mediafrost (%s) -", date);
        Bitmap stampBmp = null;
        if (iconBmp != null)
        {
        	Bitmap textBmp = createStampBitmap(iconBmp.getWidth(), iconBmp.getHeight(), stampText);
        	stampBmp = stampOverlay(iconBmp, textBmp);
        }
        else
        {
        	stampBmp = createStampBitmap(240, 240, stampText);
        }
        
	    Random generator = new Random();
	    File file; String fpath;
	    do
	    {
	    	String fname = getStampPrefix() + generator.nextInt() +".jpg"; 
	    	fpath = folder + "/" + fname;
	    	file = new File (fpath);
	    } while (file.exists());
	    
	    try {
	           FileOutputStream out = new FileOutputStream(file);
	           stampBmp.compress(Bitmap.CompressFormat.JPEG, 50, out);
	           out.flush();
	           out.close();
	           context.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.parse("file://" + fpath)));

	    } catch (Exception e) {
	           e.printStackTrace();
	    }
	}
	
	public static void getMediaFiles(Context context, String bukid, String proj, String buck, Uri uri, List<String> out) {

		try {

			final String[] projection = { proj  };
			final String selection = buck + " = ?";
			/*
			final String CAMERA_IMAGE_BUCKET_NAME = Environment
					.getExternalStorageDirectory().toString() + CamFolder;
			final String CAMERA_IMAGE_BUCKET_ID = getBucketId(CAMERA_IMAGE_BUCKET_NAME);
			final String[] selectionArgs = { CAMERA_IMAGE_BUCKET_ID };
			*/
			final String[] selectionArgs = { bukid };
			final Cursor cursor = context.getContentResolver().query(
					uri, projection, selection,
					selectionArgs, null);
			if (cursor.moveToFirst()) {
				final int dataColumn = cursor
						.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
				do {
					final String data = cursor.getString(dataColumn);
					out.add(data);
				} while (cursor.moveToNext());
			}
			cursor.close();
		} catch (Exception e) {
			Log.v("Testing", e.toString());
		}
	}
	
	
	static class BucketInfo
	{
		String id;
		String name;
		Uri uri;
	};
	
	static class BucketInfos
	{
		List<BucketInfo> imageBuckets;
		List<BucketInfo> videoBuckets;
		List<String> bucketNames;
	};

	static class BucketInfoIdComp implements Comparator<BucketInfo>{
		 
	    @Override
	    public int compare(BucketInfo e1, BucketInfo e2) {
	        return e1.id.compareTo(e2.id);
	    }
	}   
	
	public static List<BucketInfo> getBucketNames(Context context, Uri EXTERNAL_CONTENT_URI, String BUCKET_ID, String BUCKET_DISPLAY_NAME, String DATE_TAKEN, String DATA, String DATE_MODIFIED) {
		Uri images = EXTERNAL_CONTENT_URI;
	    String[] projection = new String[]{   
	            BUCKET_ID,
	            BUCKET_DISPLAY_NAME,
	            DATE_TAKEN,
	            DATA
	    };

	    String BUCKET_ORDER_BY = DATE_MODIFIED + " DESC";
	    String BUCKET_GROUP_BY = "1) GROUP BY 1,(2";
	    Cursor imagecursor = context.getContentResolver().query(images,
	            projection, // Which columns to return
	            BUCKET_GROUP_BY,       // Which rows to return (all rows)
	            null,       // Selection arguments (none)
	            BUCKET_ORDER_BY        // Ordering
	            );

	    //this.imageUrls = new ArrayList<String>();
	    ArrayList<BucketInfo> imageBuckets  = new ArrayList<BucketInfo>();
	    for (int i = 0; i < imagecursor.getCount(); i++)
	    {
	        imagecursor.moveToPosition(i);
	        BucketInfo info = new BucketInfo();
	        {
	        	int bucketColumnIndex = imagecursor.getColumnIndex(BUCKET_DISPLAY_NAME);
	        	String bucketDisplayName = imagecursor.getString(bucketColumnIndex);
	        	info.name = bucketDisplayName;
	        }
	        {
	        	int bucketColumnIndex = imagecursor.getColumnIndex(BUCKET_ID);
	        	String bucketDisplayId = imagecursor.getString(bucketColumnIndex);
	        	info.id = bucketDisplayId;
	        }
	        imageBuckets.add(info);
	        //int dataColumnIndex = imagecursor.getColumnIndex(MediaStore.Images.Media.DATA);
	        //imageUrls.add(imagecursor.getString(dataColumnIndex));

	    }
	    return imageBuckets;
	}
	
	
	
	public static BucketInfos getBuckets(Context context) {
		
		BucketInfos infos = new BucketInfos();
		
		infos.imageBuckets = getBucketNames(context, MediaStore.Images.Media.EXTERNAL_CONTENT_URI,  MediaStore.Images.Media.BUCKET_ID,
	            MediaStore.Images.Media.BUCKET_DISPLAY_NAME,
	            MediaStore.Images.Media.DATE_TAKEN,
	            MediaStore.Images.Media.DATA,
	            MediaStore.Images.Media.DATE_MODIFIED);
		infos.videoBuckets = getBucketNames(context, MediaStore.Video.Media.EXTERNAL_CONTENT_URI,  MediaStore.Video.Media.BUCKET_ID,
	            MediaStore.Video.Media.BUCKET_DISPLAY_NAME,
	            MediaStore.Video.Media.DATE_TAKEN,
	            MediaStore.Video.Media.DATA,
	            MediaStore.Video.Media.DATE_MODIFIED);	
		
		TreeSet<BucketInfo> col1 = new TreeSet<BucketInfo>(new BucketInfoIdComp());
		col1.addAll(infos.imageBuckets); col1.addAll(infos.videoBuckets);
		
		Iterator<BucketInfo> it = col1.iterator();
		 while(it.hasNext()) {
			 infos.bucketNames.add(it.next().name);
	      }

		return infos;
	}
	
	public static List<String> getMediaFiles(Context context, BucketInfos infos, String bucketName) {

		String imgId = null; String vidId = null;
		
		for (int i=0; i<infos.imageBuckets.size(); i++)
		{
			if (bucketName.compareTo(infos.imageBuckets.get(i).name) == 0)
			{
				imgId = infos.imageBuckets.get(i).id;
				break;
			}
		}
		
		for (int i=0; i<infos.videoBuckets.size(); i++)
		{
			if (bucketName.compareTo(infos.videoBuckets.get(i).name) == 0)
			{
				vidId = infos.videoBuckets.get(i).id;
				break;
			}
		}
		
		ArrayList<String> result = new ArrayList<String>();
		if (imgId != null)
			getMediaFiles(context, imgId, MediaStore.Images.Media.DATA, MediaStore.Images.Media.BUCKET_ID, MediaStore.Images.Media.EXTERNAL_CONTENT_URI, result);
		if (vidId != null)
			getMediaFiles(context, vidId, MediaStore.Video.Media.DATA, MediaStore.Video.Media.BUCKET_ID, MediaStore.Video.Media.EXTERNAL_CONTENT_URI, result);
		return result;
	}
	
	/*
	public static List<String> getMediaFiles(Context context, String bucketId) {

		ArrayList<String> result = new ArrayList<String>();
		getMediaFiles(context, MediaStore.Images.Media.DATA, MediaStore.Images.Media.BUCKET_ID, MediaStore.Images.Media.EXTERNAL_CONTENT_URI, result);
		getMediaFiles(context, MediaStore.Video.Media.DATA, MediaStore.Video.Media.BUCKET_ID, MediaStore.Video.Media.EXTERNAL_CONTENT_URI, result);
		return result;
	}
	*/

}
