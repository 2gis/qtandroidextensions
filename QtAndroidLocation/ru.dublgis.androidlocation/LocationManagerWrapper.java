/*
	Offscreen Android Views library for Qt

	Author:
	  Vyacheslav O. Koscheev <vok1980@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2020, DoubleGIS, LLC.
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

package ru.dublgis.androidlocation;

import android.app.PendingIntent;
import android.content.Context;
import android.os.Build;
import android.os.Looper;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.util.Log;
import androidx.core.content.ContextCompat;

import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;


public class LocationManagerWrapper
{
	private static final String TAG = "Grym/LocationManagerWrp";
	private LocationManager mLocationManager = null;
	private Context mContext = null;
	private String mProvider;


	public LocationManagerWrapper(Context ctx, String provider) {
		Log.d(TAG, "LocationManagerWrapper");
		mContext = ctx;
		mProvider = provider;

		if (null != ctx) {
			try {
				mLocationManager = (LocationManager)ctx.getSystemService(Context.LOCATION_SERVICE);
			} catch(Exception e) {
				Log.e(TAG, "Failed to get LocationManager", e);
			}
		}
	}


	public boolean isPermissionGranted() {
		if (null == mContext) {
			Log.e(TAG, "Context is null in permition checker");
			return false;
		}
		if (Build.VERSION.SDK_INT >= 23) { // Android 6.0
			// If your application only has the coarse permission then it will not have access to <...> passive location providers.
			if ((LocationManager.PASSIVE_PROVIDER != mProvider && PERMISSION_GRANTED != ContextCompat.checkSelfPermission(mContext, ACCESS_COARSE_LOCATION)) ||
				PERMISSION_GRANTED != ContextCompat.checkSelfPermission(mContext, ACCESS_FINE_LOCATION)) {
				Log.i(TAG, "Permission is not granted");
				return false;
			}
		}
		return true;
	}


	public Location getLastKnownLocation() {
		Log.d(TAG, "lastKnownPosition");
		try {
			if (null != mLocationManager && isPermissionGranted()) {
				Log.d(TAG, "lastKnownPosition, mLocationManager not null");
				return mLocationManager.getLastKnownLocation(mProvider);
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to get last known position", e);
		}

		Log.i(TAG, "lastKnownPosition, return null");
		return null;
	}


	public boolean requestSingleUpdate(LocationListener listener, Looper looper) {
		try {
			if (null != mLocationManager && isPermissionGranted()) {
				mLocationManager.requestSingleUpdate(mProvider, listener, looper);
				return true;
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to start single location update", e);
		}
		return false;
	}


	public boolean requestLocationUpdates(long minTimeMs, LocationListener listener, Looper looper) {
		try {
			if (null != mLocationManager && isPermissionGranted()) {
				mLocationManager.requestLocationUpdates(mProvider, minTimeMs, 0, listener, looper);
				return true;
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to start location updates", e);
		}
		return false;
	}


	public void removeUpdates(LocationListener listener) {
		try {
			if (null != mLocationManager) {
				mLocationManager.removeUpdates(listener);
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to remove updates", e);
		}
	}
}
