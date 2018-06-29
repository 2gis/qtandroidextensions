/*
  Offscreen Android Views library for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
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

import android.location.LocationManager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import ru.dublgis.androidhelpers.Log;


public class LocationManagerProvidersListener extends BroadcastReceiver
{
	static final String TAG = "Grym/LocMngProvListener";
	private volatile long native_ptr_ = 0;


	public LocationManagerProvidersListener(long native_ptr)
	{
		native_ptr_ = native_ptr;
		try {
			getActivity().registerReceiver(this, new IntentFilter(LocationManager.PROVIDERS_CHANGED_ACTION));
		} catch (final Throwable e) {
			Log.e(TAG, "Exception in LocationManagerProvidersListener constructor: ", e);
		}
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		try {
			getActivity().unregisterReceiver(this);
		} catch (final Throwable e) {
			Log.e(TAG, "Exception in cppDestroyed: ", e);
		}
		native_ptr_ = 0;
	}


	public void onReceive( Context context, Intent intent )
	{
		try {
			onProvidersChange(native_ptr_);
		}
		catch (final Throwable ex) {
			Log.e(TAG, "Failed to call onProvidersChange ", ex);
		}
	}


	public boolean isActiveProvidersEnabled()
	{
		return isGpsProviderEnabled() || isNetworkProviderEnabled();
	}


	public boolean isGpsProviderAvailable()
	{
		return isProviderAvailable(LocationManager.GPS_PROVIDER);
	}


	public boolean isNetworkProviderAvailable()
	{
		return isProviderAvailable(LocationManager.NETWORK_PROVIDER);
	}


	public boolean isGpsProviderEnabled()
	{
		return isProviderEnabled(LocationManager.GPS_PROVIDER);
	}


	public boolean isNetworkProviderEnabled()
	{
		return isProviderEnabled(LocationManager.NETWORK_PROVIDER);
	}


	public boolean isProviderEnabled(String provider)
	{
		boolean ret = false;

		try
		{
			final LocationManager lm =
				(LocationManager) getActivity().getSystemService(Context.LOCATION_SERVICE);

			if (lm != null) {
				ret = lm.isProviderEnabled(provider);
			}
		}
		catch(Throwable e)
		{
			Log.e(TAG, "isProviderEnabled exception: " + e);
		}

		return ret;
	}


	public boolean isProviderAvailable(String provider)
	{
		try
		{
			final LocationManager lm =
				(LocationManager) getActivity().getSystemService(Context.LOCATION_SERVICE);

			if (lm != null) {
				return lm.getAllProviders().contains(provider);
			}
		}
		catch(Throwable e)
		{
			Log.e(TAG, "LocationManager.getAllProviders() failed: ", e);
		}

		return false;
	}


	public native Activity getActivity();
	public native void onProvidersChange(long nativeptr);
}

