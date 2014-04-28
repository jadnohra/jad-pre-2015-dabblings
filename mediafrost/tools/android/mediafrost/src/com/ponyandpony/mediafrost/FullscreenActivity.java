package com.ponyandpony.mediafrost;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import android.annotation.TargetApi;
import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
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
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.database.Cursor;
import android.text.format.Time;
import android.util.Log;
import android.content.Context;

public class FullscreenActivity extends Activity {

	class StatusTextViews {
		TextView time;
		TextView activity;
		TextView console;
		ProgressBar progress;
		Button backup;
	};

	ViewFlipper mViewFlipper;
	StatusTextViews mStatusTextViews;
	SettingsActivity mSettingsActivity;
	
	enum Status
	{
		Idle,
		Working,
	};
	Status mStatus;
	int mWorkCount;

	static public String getSetting(Context context, String key, String dft) 
	{ 
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		if (prefs.contains(key) == false)
		{
			SharedPreferences.Editor editor = prefs.edit();
			editor.putString(key, dft); editor.commit();
		}
		return prefs.getString(key, dft); 
	}
	
	static public String getSetting(Context context, String key) { return getSetting(context, key, null); }
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mStatus = Status.Idle;
		mWorkCount = 0;
		
		{
			getSetting(this, "Server", "192.168.1.4:24107");
			getSetting(this, "MinFiles", "30");
			getSetting(this, "MaxFiles", "300");
		}
		
		setContentView(R.layout.activity_fullscreen);
		findViewById(R.id.dummy_button).setOnTouchListener(mTouchListener);
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
			mStatusTextViews.progress = (ProgressBar) findViewById(R.id.progressBar1);
			mStatusTextViews.progress.setVisibility(View.INVISIBLE);
			mStatusTextViews.backup = (Button) findViewById(R.id.dummy_button);
		}

		{
			mSettingsActivity = new SettingsActivity();
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

	View.OnTouchListener mTouchListener = new View.OnTouchListener() {
		@Override
		public boolean onTouch(View view, MotionEvent motionEvent) {

			if (view.getId() == R.id.dummy_button) {
				Context context = view.getContext();
				if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
					
					FullscreenActivity.this.mStatus = Status.Working;
					FullscreenActivity.this.mWorkCount++;
					FullscreenActivity.this.mStatusTextViews.backup.setEnabled(false);
					
					List<String> images = getCameraImages(context);
					// Log.v("Testing", images.toString());

					boolean dbg = false;
					if (dbg && images.size() > 0) {
						String hash = genMD5Hash(images.get(0));
						Log.v("Testing", images.get(0));
						Log.v("Testing", hash);
					}

					if (images.size() > 0) {
						
						mStatusTextViews.console.setText("Connecting ...");
						mStatusTextViews.time.setText("00:00:00");
						mStatusTextViews.activity.setText("Connecting...");
						mStatusTextViews.progress.setVisibility(View.VISIBLE);

						NetworkThreadSettings settings = new NetworkThreadSettings();
						String address[] = getSetting(context, "Server", null).split(":");
						settings.server = address[0];
						settings.port = Integer.parseInt(address[1]);
						settings.minFiles = Integer.parseInt(getSetting(context, "MinFiles"));
						settings.maxFiles = Integer.parseInt(getSetting(context, "MaxFiles"));
						NetworkThread thread = new NetworkThread(mThreadMessageHandler, settings, images, mStatusTextViews);
						thread.start();
					}
					return true;
				}
			}

			return false;
		}
	};

	public Handler mThreadMessageHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			
			if (msg.what == 1)
			{
				FullscreenActivity.this.mStatus = Status.Idle;
				FullscreenActivity.this.mStatusTextViews.backup.setEnabled(true);
			}
		}
	};
	
	class NetworkThreadSettings
	{
		String server;
		int port;
		int minFiles;
		int maxFiles;
	};
	
	public class NetworkThread extends Thread {
		
		NetworkThreadSettings mSettings;
		Handler mMessageHandler;
		List<String> mImages;
		StatusTextViews mStatusTextViews;
		Handler mTimerHandler;
		Runnable mTimerRunnable;

		NetworkThread(Handler messageHandler, NetworkThreadSettings settings, List<String> images, StatusTextViews statusTextViews) {
			mMessageHandler = messageHandler;
			mSettings = settings;
			mImages = images;
			mStatusTextViews = statusTextViews;

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
				byte[] outBytes = new byte[length];
				int leftBytes = length;
				int arrIndex = 0;
				int offset = m_offset;
				while (leftBytes > 0) {
					byte[] readBytes = m_arrays.get(arrIndex);
					int readCount = Math.min(leftBytes, readBytes.length);
					System.arraycopy(readBytes, offset, outBytes, length
							- leftBytes, readCount);
					leftBytes -= readCount;
					arrIndex++;
					offset = 0;
				}
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
				Log.v("Testing", String.format("PeekBuf: [%s]", dbgStr));
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

		void stopProgress() {
			if (mStatusTextViews.progress != null)
				mStatusTextViews.progress.post(new Runnable() {
					public void run() {
						mStatusTextViews.progress.setVisibility(View.INVISIBLE);
					}
				});
			if (mMessageHandler != null)
			{
				mMessageHandler.sendEmptyMessage(1);
			}
		}

		void logStatus(final String str) {
			Log.v("Testing", str);
			setText(mStatusTextViews.activity, str);
			setText(mStatusTextViews.console, String.format("%s\n%s",
					mStatusTextViews.console.getText(), str));
		}

		public void run() {
			try {
				
				{
					byte[] buffer = "hello".getBytes();
					DatagramSocket dsock = new DatagramSocket(2600);
					DatagramPacket packet = new DatagramPacket(buffer, buffer.length, new InetSocketAddress("255.255.255.255", 1600));
					dsock.send(packet);
					dsock.close();
				}
				
				
				Socket sock = new Socket();
				sock.connect(new InetSocketAddress(mSettings.server, mSettings.port), 2500);
				if (sock.isConnected()) {
					logStatus(String.format("Connected to %s", sock
							.getRemoteSocketAddress().toString()));
					logStatus(String.format("Senging %d fids ...",
							mImages.size()));
					sock.getOutputStream().write("/start".getBytes("US-ASCII"));
					int imageCount = mImages.size();
					if (imageCount > 20)
						imageCount = 20;
					HashMap<String, Integer> fidMap = new HashMap<String, Integer>();

					for (int i = 0; i < imageCount; i++) {
						String filePath = mImages.get(i);
						String fileName = new File(filePath).getName();

						setText(mStatusTextViews.activity,
								String.format("Hashing %s", fileName));
						String fid = genMD5Hash(filePath);
						fidMap.put(fid, Integer.valueOf(i));
						sock.getOutputStream().write(
								String.format("/fid:%s:%s", fileName, fid)
										.getBytes("US-ASCII"));
					}
					sock.getOutputStream()
							.write("/fidend".getBytes("US-ASCII"));
					sock.getOutputStream().flush();

					ArrayList<String> requestedFids = new ArrayList<String>();
					logStatus("Gathering file requests ...");
					{
						sock.setSoTimeout(100);
						InputStream istream = sock.getInputStream();
						byte[] bufRead = new byte[sock.getReceiveBufferSize()];
						ByteSocketBuffer bufCmd = new ByteSocketBuffer();

						boolean requesting = true;
						String cmdFrequest = "/frequest:";
						int fidLen = 64;
						String cmdEnd = "/frequestend";
						while (requesting) {
							try {

								int bufSize = istream.read(bufRead);
								if (bufSize == -1)
									break;
								bufCmd.add(bufRead, bufSize);

								boolean consumed;
								do {
									consumed = false;

									if (bufCmd.available() >= cmdFrequest
											.length() + fidLen
											&& bufCmd.startsWith(cmdFrequest)) {
										bufCmd.skip(cmdFrequest.length());
										byte[] fidBytes = bufCmd.read(fidLen);
										String fid = new String(fidBytes,
												"US-ASCII");
										requestedFids.add(fid);
										// Log.v("Testing",
										// String.format("Requested %s [%d].",
										// fid, fidIndex.intValue()));
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
						logStatus(String.format("Sending %d files ...",
								requestedFids.size()));
						for (int i = 0; i < requestedFids.size(); i++) {
							String fid = requestedFids.get(i);
							int index = fidMap.get(fid).intValue();
							String filePath = mImages.get(index);

							File file = new File(filePath);
							setText(mStatusTextViews.activity,
									String.format("Sending %s", file.getName()));
							FileInputStream fileInputStream = new FileInputStream(
									file);
							int fileSize = fileInputStream.available();
							sock.getOutputStream().write(
									String.format("/fdata:%s:%d:", fid,
											fileSize).getBytes("US-ASCII"));

							byte[] buffer = new byte[128];
							int read = 0;
							while ((read = fileInputStream.read(buffer, 0,
									buffer.length)) != -1)
								sock.getOutputStream().write(buffer, 0, read);
							fileInputStream.close();
						}
						sock.getOutputStream().write(
								"/fdataend".getBytes("US-ASCII"));
						sock.getOutputStream().flush();

						logStatus("Waiting for backup ...");
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

										if (bufCmd.available() >= cmdSuccess
												.length() + codeLen
												&& bufCmd
														.startsWith(cmdSuccess)) {
											bufCmd.skip(cmdSuccess.length());
											byte[] codeBytes = bufCmd
													.read(codeLen);
											String code = new String(codeBytes,
													"US-ASCII");

											logStatus(String.format(
													"Success: %s", code));
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
					} else {
						logStatus("Nothing to do...");
					}
					sock.close();
					logStatus("Done.");
				}

			} catch (Exception e) {
				logStatus(e.toString());
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
			byte[] buffer = new byte[128];
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

	public static List<String> getCameraImages(Context context) {

		try {

			final String[] projection = { MediaStore.Images.Media.DATA };
			final String selection = MediaStore.Images.Media.BUCKET_ID + " = ?";
			final String CAMERA_IMAGE_BUCKET_NAME = Environment
					.getExternalStorageDirectory().toString() + "/DCIM/Camera";
			final String CAMERA_IMAGE_BUCKET_ID = getBucketId(CAMERA_IMAGE_BUCKET_NAME);
			final String[] selectionArgs = { CAMERA_IMAGE_BUCKET_ID };
			final Cursor cursor = context.getContentResolver().query(
					Images.Media.EXTERNAL_CONTENT_URI, projection, selection,
					selectionArgs, null);
			ArrayList<String> result = new ArrayList<String>(cursor.getCount());
			if (cursor.moveToFirst()) {
				final int dataColumn = cursor
						.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
				do {
					final String data = cursor.getString(dataColumn);
					result.add(data);
				} while (cursor.moveToNext());
			}
			cursor.close();
			return result;
		} catch (Exception e) {
			Log.v("Testing", e.toString());
			return new ArrayList<String>();
		}
	}

}
