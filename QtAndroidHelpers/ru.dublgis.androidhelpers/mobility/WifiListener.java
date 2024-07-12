/*
    Offscreen Android Views library for Qt

    Authors:
    Vyacheslav O. Koscheev <vok1980@gmail.com>
    Ivan Avdeev marflon@gmail.com
    Sergey A. Galin sergey.galin@gmail.com

    Distrbuted under The BSD License

    Copyright (c) 2015-2023, DoubleGIS, LLC.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the DoubleGIS, LLC nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.
*/

package ru.dublgis.androidhelpers.mobility;

import java.util.List;
import java.util.Iterator;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.LocationManager;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.os.Build;
import android.os.SystemClock;
import android.provider.Settings;
import androidx.core.content.ContextCompat;

import ru.dublgis.androidhelpers.Log;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.Manifest.permission.CHANGE_WIFI_STATE;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;


// Listens for wifi scan results. Passes them.
public class WifiListener extends BroadcastReceiver
{
	final static String TAG = "Grym/WifiListener";
	final private static boolean verbose = false;
	private long mNativePtr = 0;
	private WifiManager mWifiMan = null;


	public WifiListener(long native_ptr) {
		mNativePtr = native_ptr;
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public synchronized void cppDestroyed() {
		mNativePtr = 0;
	}


	// start listening for scan results and report them
	public synchronized boolean start() {
		try {
			Log.d(TAG, "WifiListener start");

			mWifiMan = (WifiManager)getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);

			if (null == mWifiMan) {
				Log.e(TAG, "No WifiManager available on start");
				return false;
			}

			getContext().registerReceiver(this,
				new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));

			return startScan();
		} catch(Throwable e) {
			Log.e(TAG, "Exception while starting WifiListener ", e);
			return false;
		}
	}


	public synchronized boolean startScan() {
		Log.w(TAG, "WifiListener startScan");
		if (null == mWifiMan) {
			Log.w(TAG, "No WifiManager available on startScan");
			return false;
		}

		try {
			final Context ctx = getContext();
			final boolean fine_loc_granted = (PERMISSION_GRANTED == ContextCompat.checkSelfPermission(ctx, ACCESS_FINE_LOCATION));
			final boolean coarse_loc_granted = (PERMISSION_GRANTED == ContextCompat.checkSelfPermission(ctx, ACCESS_COARSE_LOCATION));
			final boolean wifi_state_granted = (PERMISSION_GRANTED == ContextCompat.checkSelfPermission(ctx, CHANGE_WIFI_STATE));
			final boolean settings_loc_enabled = isLocationEnabled(ctx);
			final int targetSdkVersion = ctx.getApplicationInfo().targetSdkVersion;

			// see https://developer.android.com/develop/connectivity/wifi/wifi-scan#wifi-scan-permissions
			if ((Build.VERSION.SDK_INT < 28 && (fine_loc_granted || coarse_loc_granted || wifi_state_granted))
				|| (Build.VERSION.SDK_INT == 28 && wifi_state_granted && settings_loc_enabled && (fine_loc_granted || coarse_loc_granted))
				|| (Build.VERSION.SDK_INT >= 29 && wifi_state_granted && settings_loc_enabled && targetSdkVersion >= 29 && fine_loc_granted)
				|| (Build.VERSION.SDK_INT >= 29 && wifi_state_granted && settings_loc_enabled && targetSdkVersion < 29 && (fine_loc_granted || coarse_loc_granted))
			) {
				return mWifiMan.startScan();
			}
		} catch(Throwable e) {
			Log.e(TAG, "Exception while starting WifiListener ", e);
		}
		return false;
	}


	public synchronized void stop() {
		Log.d(TAG, "WifiListener stop");
		
		try {
			if (mWifiMan != null) {
				getContext().unregisterReceiver(this);
			}
		} catch(Throwable e) {
			Log.e(TAG, "Exception while stopping: ", e);
		}
	}

	// on new scan results
	public synchronized void onReceive(Context c, Intent intent) {
		if (verbose) {
			Log.d(TAG, "WifiListener onReceive");
		}

		scanUpdate(mNativePtr);
	}


	// This function is called from C++ side after it has been
	// notified about data state change by WifiListener.
	//
	// Note: Some evil things happening with using Java iterators over JNI there are,
	// which I see cannot as by The Dark Side of The Force hidden they are.
	// So just a string with a table of all Wi Fi spots seen pass us let;
	// and on C++ side will better and without oveflowing JNI reference
	// table it parsed be.
	public boolean getLastWifiScanResultsTable() {
		try {
			if (verbose) {
				Log.i(TAG, "getLastWifiScanResultsTable()");
			}

			if (null == mWifiMan) {
				Log.e(TAG, "No WifiManager available on getLastWifiScanResultsTable");
				return false;
			}

			List<ScanResult> srlist = mWifiMan.getScanResults();

			if (srlist==null || srlist.size()==0) {
				return false;
			}

			for (Iterator<ScanResult> it = srlist.iterator(); it.hasNext(); ) {
				ScanResult sr = it.next();
				setScanResult(mNativePtr, sr);
			}

			return true;
		}
		catch(Throwable e) {
			Log.e(TAG, "getLastWifiScanResultsTable exception: ", e);
			return false;
		}
	}


	@SuppressWarnings("deprecation")
	public static Boolean isLocationEnabled(Context context) {
		try {
			if (Build.VERSION.SDK_INT >= 28) { // Android 9
				// This is a new method provided in API 28
				LocationManager lm = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
				return null != lm && lm.isLocationEnabled();
			} else {
				// This was deprecated in API 28
				int mode = Settings.Secure.getInt(context.getContentResolver(), Settings.Secure.LOCATION_MODE,
						Settings.Secure.LOCATION_MODE_OFF);
				return (mode != Settings.Secure.LOCATION_MODE_OFF);
			}
		} catch(Throwable e) {
			Log.e(TAG, "isLocationEnabled exception: ", e);
			return false;
		}
	}

	// Will call getLastWifiScanResultsTable() to update WiFi status
	private native void scanUpdate(long native_ptr);
	private native void setScanResult(long native_ptr, ScanResult scan_res);
	private native Context getContext();
} // class WifiListener
