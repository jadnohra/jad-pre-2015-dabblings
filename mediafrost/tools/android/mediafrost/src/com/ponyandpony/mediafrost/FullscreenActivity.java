package com.ponyandpony.mediafrost;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Random;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.NotificationManager;
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
	
	enum Status
	{
		Idle,
		Working,
	};
	Status mStatus;
	int mWorkCount;
	NetworkThread mWorkThread;

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

			if (view.getId() == R.id.backup_button) {
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
						settings.minFiles = Integer.parseInt(getSetting(context, "MinFiles"));
						settings.maxFiles = Integer.parseInt(getSetting(context, "MaxFiles"));
						NetworkThread thread = new NetworkThread(mThreadMessageHandler, settings, images, mStatusTextViews);
						mWorkThread = thread;
						thread.start();
					}
					return true;
				}
			}
			else if (view.getId() == R.id.cancel_button) {
			
				if (mWorkThread != null && mWorkThread.mSocket != null)
				{
					try
					{
						mWorkThread.mCancel = 1;
						mWorkThread.mSocket.close();
					} catch (Exception e)
					{}
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
				FullscreenActivity.this.mWorkThread = null;
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
		long mTotalByteCount;
		StatusTextViews mStatusTextViews;
		Handler mTimerHandler;
		Runnable mTimerRunnable;
		String mStatusText;
		Socket mSocket;
		int mCancel;

		NetworkThread(Handler messageHandler, NetworkThreadSettings settings, List<String> images, StatusTextViews statusTextViews) {
			mMessageHandler = messageHandler;
			mSettings = settings;
			mImages = images;
			mStatusTextViews = statusTextViews;
			mStatusText = "";
			mCancel = 0;

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
				mMessageHandler.sendEmptyMessage(1);
			}
		}

		void logStatus(final String str) {
			//Log.v("Testing", str);
			mStatusText = String.format("%s\n%s", mStatusText, str);
			setText(mStatusTextViews.activity, str);
			setText(mStatusTextViews.console, mStatusText);
		}

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
			Socket sock = null;
			try {
				
				String server = mSettings.server; int port = mSettings.port;
				
				{
					logStatus("Discovering ...");
					DatagramSocket dsock = new DatagramSocket(2600);
					
					{
						byte[] buffer = "/mediafrost:discover".getBytes();
						DatagramPacket packet = new DatagramPacket(buffer, buffer.length, new InetSocketAddress("255.255.255.255", 1600));
						dsock.send(packet);
					}
					{
						byte[] buffer = "255.255.255.255:88888888".getBytes();
						DatagramPacket packet = new DatagramPacket(buffer, buffer.length, new InetSocketAddress("255.255.255.255", 1600));
						dsock.setSoTimeout(1500);
						try
						{
							dsock.receive(packet);
							String msg = new String(packet.getData(), 0, packet.getLength());
							String[] words = msg.split(":");
							server = words[0];
							port = Integer.parseInt(words[1]);
							logStatus(String.format("Discovered %s:%d", server, port));
						}
						catch(IOException e1) {
							logStatus("Discovery failed");
						}
						catch (Exception e2) {
							logStatus(Log.getStackTraceString(e2));
							logStatus("Discovery failed");
						}
					}
					
					dsock.close();
				}
				
				
				sock = new Socket();
				mSocket = sock;
				sock.connect(new InetSocketAddress(server, port), 1600);
				if (sock.isConnected()) {
					logStatus(String.format("Connected to %s", sock.getRemoteSocketAddress().toString()));
					
					int imageCount = mImages.size();
					if (imageCount > mSettings.maxFiles)
					{
						logStatus(String.format("Limiting %d to max. %d files", mImages.size(), mSettings.maxFiles));
						imageCount = mSettings.maxFiles;
					}
					if (imageCount < mSettings.minFiles)
					{
						logStatus(String.format("%d files is below %d", mImages.size(), mSettings.minFiles));
						imageCount = 0;
					}
					
					logStatus("Calculating total bytes ...");

					mTotalByteCount = 0;
					for (int i = 0; i < imageCount; i++) {
						String filePath = mImages.get(i);
						long bytes = new File(filePath).length();
						mTotalByteCount += bytes;
					}
					
					sock.getOutputStream().write("/start".getBytes("US-ASCII"));
					
					HashMap<String, Integer> fidMap = new HashMap<String, Integer>();
					logStatus(String.format("Senging %d fids ...", imageCount));
					{
						openProgress(mStatusTextViews.progress1);
						float prog1 = 0.0f;
						for (int i = 0; i < imageCount; i++) {
							String filePath = mImages.get(i);
							File file = new File(filePath);
							String fileName = file.getName();
	
							setText(mStatusTextViews.activity,
									String.format("Hashing %s (%d/%d)", fileName, i+1, imageCount));
							String fid = genMD5Hash(filePath);
							fidMap.put(fid, Integer.valueOf(i));
							sock.getOutputStream().write(
									String.format("/fid:%s:%s", fileName, fid)
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

									if ((bufCmd.available() >= (cmdFrequest.length() + fidLen))
											&& bufCmd.startsWith(cmdFrequest)) {
										bufCmd.skip(cmdFrequest.length());
										byte[] fidBytes = bufCmd.read(fidLen);
										String fid = new String(fidBytes,"US-ASCII");
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
					
					openProgress(mStatusTextViews.progress2);
					if (requestedFids.size() > 0) {
						logStatus(String.format("Sending %d files ...",
								requestedFids.size()));
						float prog2 = 0.0f;
						for (int i = 0; i < requestedFids.size(); i++) {
							String fid = requestedFids.get(i);
							int index = fidMap.get(fid).intValue();
							String filePath = mImages.get(index);

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

											logStatus(String.format("Success: %s", code));
											
											if (code.equals("1"))
											{
												logNotify(String.format("Backup succeeded, %d files.", requestedFids.size()));
												
												if (requestedFids.size() > 0)
												{
													String fid = requestedFids.get(requestedFids.size()-1);
													int index = fidMap.get(fid).intValue();
													String filePath = mImages.get(index);
													stampSuccess(FullscreenActivity.this, filePath);
												}
											}
											else
											{
												logNotify("Backup failed.");	
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
					} else {
						logStatus("Nothing to do...");
					}
					sock.close();
					logStatus("Done.");
				}

			} catch(IOException e1) {
				
				
				
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
				
				if (mCancel == 0)
					logNotify("Backup failed.");
				
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
				if (mCancel == 0)
					logNotify("Backup failed.");
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
		Paint paint = new Paint();
		c.drawColor(Color.TRANSPARENT);
	    paint.setTextSize(40);
	    paint.setTextScaleX(1.f);
	    paint.setAlpha(0);
	    paint.setAntiAlias(true);
	    paint.setColor(Color.GREEN);
	    Rect bounds = new Rect();
	    paint.getTextBounds(text,0,text.length(),bounds);
	    float scale =  (float) w / (1.25f * (float) bounds.width());
	    paint.setTextSize((int) ((float) paint.getTextSize() * scale));
	    c.drawText(text, w/2-bounds.width()/2, h/2, paint);
	    return b;
	}

	private static Bitmap stampOverlay(Bitmap bmp1, Bitmap bmp2) {
	    Bitmap bmOverlay = Bitmap.createBitmap(bmp1.getWidth(), bmp1.getHeight(), bmp1.getConfig());
	    Canvas canvas = new Canvas(bmOverlay);
	    canvas.drawBitmap(bmp1, new Matrix(), null);
	    canvas.drawBitmap(bmp2, 0, 0, null);
	    return bmOverlay;
	}
	
	@SuppressLint("SimpleDateFormat")
	public static void stampSuccess(Context context, String iconInputFilePath)
	{
		final String folder = Environment.getExternalStorageDirectory().toString() + CamFolder;
	
        String date = new SimpleDateFormat("dd-MM-yyyy").format(Calendar.getInstance().getTime());
		
		Bitmap iconBmp = BitmapFactory.decodeFile(iconInputFilePath);
		Bitmap textBmp = createStampBitmap(iconBmp.getWidth(), iconBmp.getHeight(), String.format("- Mediafrost (%s) -", date));
		Bitmap stampBmp = stampOverlay(iconBmp, textBmp);
	    Random generator = new Random();
	    File file; String fpath;
	    do
	    {
	    	String fname = "mediafrost_stamp-"+ generator.nextInt() +".jpg"; 
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
	
	public static void getMediaFiles(Context context, String proj, String buck, Uri uri, List<String> out) {

		try {

			final String[] projection = { proj  };
			final String selection = buck + " = ?";
			final String CAMERA_IMAGE_BUCKET_NAME = Environment
					.getExternalStorageDirectory().toString() + CamFolder;
			final String CAMERA_IMAGE_BUCKET_ID = getBucketId(CAMERA_IMAGE_BUCKET_NAME);
			final String[] selectionArgs = { CAMERA_IMAGE_BUCKET_ID };
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

	public static List<String> getCameraImages(Context context) {

		ArrayList<String> result = new ArrayList<String>();
		getMediaFiles(context, MediaStore.Images.Media.DATA, MediaStore.Images.Media.BUCKET_ID, MediaStore.Images.Media.EXTERNAL_CONTENT_URI, result);
		getMediaFiles(context, MediaStore.Video.Media.DATA, MediaStore.Video.Media.BUCKET_ID, MediaStore.Video.Media.EXTERNAL_CONTENT_URI, result);
		return result;
	}

}
