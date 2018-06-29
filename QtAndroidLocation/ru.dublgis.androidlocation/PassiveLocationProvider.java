/*
	Offscreen Android Views library for Qt

	Author:
	  Vyacheslav O. Koscheev <vok1980@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2017, DoubleGIS, LLC.
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
import android.app.Activity;
import android.support.v4.content.ContextCompat;
import android.location.Criteria;
import android.os.Bundle;
import android.os.Looper;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.util.Log;

import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;



public class PassiveLocationProvider implements LocationListener
{
	private static final String TAG = "PassiveLocationProvider";
	private volatile long native_ptr_ = 0;
	private static LocationManager mLocationManager = null;
	private static Context mContext = null;

	private Looper mlocationUpdatesLooper = null;
	final private Thread mlocationUpdatesThread = new Thread() {
		public void run() {
			Looper.prepare();
			mlocationUpdatesLooper = Looper.myLooper();
			Looper.loop();
		}
	};


	public PassiveLocationProvider(long native_ptr)
	{
		Log.i(TAG, "PassiveLocationProvider");
		native_ptr_ = native_ptr;
		setContext(getActivity());
		mlocationUpdatesThread.start();
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		Log.i(TAG, "cppDestroyed");
		native_ptr_ = 0;

		if (null != mlocationUpdatesLooper)
		{
			mlocationUpdatesLooper.quit();
		}

		try {
			if (mlocationUpdatesThread.isAlive()) {
				mlocationUpdatesThread.join(300);
			}
		} catch (InterruptedException e) {
			Log.e(TAG, "Exception in cppDestroyed: ", e);
		}
	}


	static public void setContext(Context context) {
		Log.i(TAG, "setContext");
		mContext = context;
		if (null != context) {
			try {
				Log.i(TAG, "null != context");
				mLocationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
			} catch(Exception e) {
				Log.e(TAG, "Failed to get LocationManager", e);
			}
		}
	}


	static public boolean isPermissionGranted(Context ctx) {
		if (null == ctx) {
			Log.e(TAG, "Context is null in permition checker");
			return false;
		}
		if (Build.VERSION.SDK_INT >= 23 &&
				// If your application only has the coarse permission then it will not have access to <...> passive location providers.
				PERMISSION_GRANTED != ContextCompat.checkSelfPermission(ctx, ACCESS_FINE_LOCATION)) {
			Log.i(TAG, "Permission is not granted");
			return false;
		}
		else {
			return true;
		}
	}


	static public Location lastKnownPosition(boolean fromSatelliteOnly) {
		Log.i(TAG, "lastKnownPosition");
		try {
			if (null != mContext && null != mLocationManager && isPermissionGranted(mContext)) {
				Log.i(TAG, "lastKnownPosition, mLocationManager not null");
				return mLocationManager.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to get last known position", e);
		}

		Log.i(TAG, "lastKnownPosition, return null");
		return null;
	}


	public void onLocationChanged(Location location) {
		onLocationRecieved(native_ptr_, location);
	}


	public void onStatusChanged(String provider, int status, Bundle extras) {}


	public void onProviderEnabled(String provider) {}


	public void onProviderDisabled(String provider) {}


	static public boolean requestSingleUpdate(PendingIntent intent) {
		try {
			if (null != mContext && null != mLocationManager && isPermissionGranted(mContext)) {
				Criteria criteria = new Criteria();
				criteria.setPowerRequirement(Criteria.POWER_HIGH);
				mLocationManager.requestSingleUpdate(criteria, intent);
				return true;
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to start location updates", e);
		}
		return false;
	}

	public boolean startLocationUpdates(final int minTime) {
		try {
			if (null != mContext && null != mLocationManager && isPermissionGranted(mContext)) {
				mLocationManager.requestLocationUpdates(LocationManager.PASSIVE_PROVIDER, minTime, 0, this, mlocationUpdatesLooper);
				return true;
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to start location updates", e);
		}
		return false;
	}


	public void stopLocationUpdates() {
		try {
			if (null != mLocationManager) {
				mLocationManager.removeUpdates(this);
			}
		} catch(Throwable e) {
			Log.e(TAG, "Failed to remove updates", e);
		}
	}


	public native Activity getActivity();
	public native void onLocationRecieved(long nativeptr, Location location);
}
