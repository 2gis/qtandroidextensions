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
import android.os.Bundle;
import android.os.Looper;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.util.Log;
import androidx.core.content.ContextCompat;

import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;



public class PassiveLocationProvider implements LocationListener
{
	private static final String TAG = "PassiveLocationProvider";
	private volatile long native_ptr_ = 0;
	LocationManagerWrapper mProvider;

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
		mProvider = new LocationManagerWrapper(getContext(), LocationManager.PASSIVE_PROVIDER);
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


	public Location lastKnownPosition(boolean fromSatelliteOnly) {
		return mProvider.getLastKnownLocation();
	}


	public void onLocationChanged(Location location) {
		onLocationRecieved(native_ptr_, location);
	}


	public void onStatusChanged(String provider, int status, Bundle extras) {}


	public void onProviderEnabled(String provider) {}


	public void onProviderDisabled(String provider) {}


	public boolean requestSingleUpdate() {
		return mProvider.requestSingleUpdate(this, mlocationUpdatesLooper);
	}


	public boolean startLocationUpdates(final int minTime) {
		return mProvider.requestLocationUpdates(minTime, this, mlocationUpdatesLooper);
	}


	public void stopLocationUpdates() {
		mProvider.removeUpdates(this);
	}


	public native Context getContext();
	public native void onLocationRecieved(long nativeptr, Location location);
}
