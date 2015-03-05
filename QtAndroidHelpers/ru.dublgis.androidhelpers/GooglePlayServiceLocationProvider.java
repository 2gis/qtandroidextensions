/*
  Offscreen Android Views library for Qt

  Authors:
  Sergey A. Galin <sergey.galin@gmail.com>
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

package ru.dublgis.androidhelpers;

import android.app.Activity;
import android.util.Log;

import android.os.Bundle;
import android.location.Location;

import java.text.DateFormat;
import java.util.Date;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.GoogleApiClient.ConnectionCallbacks;
import com.google.android.gms.common.api.GoogleApiClient.OnConnectionFailedListener;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;



public class GooglePlayServiceLocationProvider 
			  implements ConnectionCallbacks, OnConnectionFailedListener, LocationListener
{
	public static final String TAG = "Grym/GooglePlayServiceLocationProvider";
	public final static int STATUS_DISCONNECTED		  = 0;
	public final static int STATUS_CONNECTED			 = 1;

	private long native_ptr_ = 0;
	private long mUpdateInterval = 1000;
	private long mUpdateIntervalFastest = mUpdateInterval / 2;

	protected GoogleApiClient mGoogleApiClient;
	protected Location mCurrentLocation;
	protected LocationRequest mLocationRequest;
	protected Boolean mRequestingLocationUpdates = false;




	public GooglePlayServiceLocationProvider(long native_ptr)
	{
		native_ptr_ = native_ptr;

		googleApiClientStatus(native_ptr_, STATUS_DISCONNECTED);

		buildGoogleApiClient();
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		if (mGoogleApiClient != null)
		{
			try 
			{
				if (mGoogleApiClient.isConnected())
				{
					mGoogleApiClient.disconnect();
				}
			}
			catch(Exception e)
			{
				Log.e(TAG, e.getMessage());
			}
		}

		googleApiClientStatus(native_ptr_, STATUS_DISCONNECTED);
		native_ptr_ = 0;
	}


	final public boolean runOnUiThread(final Runnable runnable)
	{
		try
		{
			if (runnable == null)
			{
				Log.e(TAG, "GooglePlayServiceLocationProvider.runOnUiThread: null runnable!");
				return false;
			}

			final Activity context = getActivity();
			
			if (context == null)
			{
				Log.e(TAG, "GooglePlayServiceLocationProvider.runOnUiThread: cannot schedule task because of the null context!");
				return false;
			}
			
			context.runOnUiThread(runnable);
			return true;
		}
		catch (Exception e)
		{
			Log.e(TAG, "Exception when posting a runnable:", e);
			return false;
		}
	}

	
	protected synchronized void buildGoogleApiClient() 
	{
		Log.i(TAG, "Building GoogleApiClient");

		try 
		{
			mGoogleApiClient = new GoogleApiClient.Builder(getActivity())
					.addConnectionCallbacks(this)
					.addOnConnectionFailedListener(this)
					.addApi(LocationServices.API)
					.build();

			mGoogleApiClient.connect();
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}
		createLocationRequest();
	}


	protected void createLocationRequest() 
	{
		mLocationRequest = new LocationRequest();
		mLocationRequest.setInterval(mUpdateInterval);
		mLocationRequest.setFastestInterval(mUpdateIntervalFastest);
		mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);
	}


	@Override
	public void onConnected(Bundle connectionHint) 
	{
		Log.i(TAG, "Connected to GoogleApiClient");

		googleApiClientStatus(native_ptr_, STATUS_CONNECTED);

		if (mCurrentLocation == null) 
		{
			try
			{
				mCurrentLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
			}
			catch(Exception e)
			{
				Log.e(TAG, e.getMessage());
			}
		}

		googleApiClientLocation(native_ptr_, mCurrentLocation, true);

		if (mRequestingLocationUpdates) {
			startLocationUpdates();
		}
	}


	protected void startLocationUpdates() 
	{
		try 
		{
			if (mGoogleApiClient.isConnected()) 
			{
				 PendingResult<Status> result = LocationServices.FusedLocationApi.requestLocationUpdates(
						mGoogleApiClient, mLocationRequest, this);
			}
		}
		catch(Exception e)
		{
		   Log.e(TAG, "Failed to start location updates: " + e.getMessage());
		}
	}


	protected void stopLocationUpdates() 
	{
		try 
		{
			if (mGoogleApiClient.isConnected()) 
			{
				LocationServices.FusedLocationApi.removeLocationUpdates(mGoogleApiClient, this);
			}
		}
		catch(Exception e)
		{
		   Log.e(TAG, "Failed to stop location updates: " + e.getMessage());
		}
	}


	@Override
	public void onConnectionSuspended(int cause) 
	{
		Log.i(TAG, "Connection suspended");
		googleApiClientStatus(native_ptr_, STATUS_DISCONNECTED);
		
		try
		{
			mGoogleApiClient.connect();
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}
	}


	@Override
	public void onConnectionFailed(ConnectionResult result) 
	{
		Log.i(TAG, "Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
		googleApiClientStatus(native_ptr_, STATUS_DISCONNECTED);
	}


	@Override
	public void onLocationChanged(Location location) 
	{
		mCurrentLocation = location;
		googleApiClientLocation(native_ptr_, location, false);
	}


	public void requestGoogleApiClientLocationUpdatesStart(long interval, long minimum_interval) 
	{
		final long interval_f = interval;
		final long minimum_interval_f = minimum_interval;

		runOnUiThread(new Runnable() 
		{
			public void run() 
			{
				if (!mRequestingLocationUpdates) 
				{
					mUpdateInterval = interval_f;
					mUpdateIntervalFastest = minimum_interval_f;
					createLocationRequest();
					mRequestingLocationUpdates = true;
					startLocationUpdates();
				}
			}
		});
	}


	public void requestGoogleApiClientLocationUpdatesStop() 
	{
		runOnUiThread(new Runnable() 
		{
			public void run() 
			{
				if (mRequestingLocationUpdates) 
				{
					mRequestingLocationUpdates = false;
					stopLocationUpdates();
				}
			}
		});
	}


	static public boolean isAvailable(Activity context)
	{
		try
		{
			return ConnectionResult.SUCCESS == GooglePlayServicesUtil.isGooglePlayServicesAvailable(context);
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}

		return false;
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	public native Activity getActivity();
	public native void googleApiClientStatus(long nativeptr, int status);
	public native void googleApiClientLocation(long nativeptr, Location location, boolean initial);
}
