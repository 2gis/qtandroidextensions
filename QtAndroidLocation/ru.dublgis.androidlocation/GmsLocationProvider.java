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

package ru.dublgis.androidlocation;

import android.app.Activity;
import android.os.Looper;
import android.util.Log;
import android.app.Dialog;

import java.lang.Exception;
import java.lang.Override;
import java.util.LinkedHashMap;

import android.os.Bundle;
import android.location.Location;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.GoogleApiClient.ConnectionCallbacks;
import com.google.android.gms.common.api.GoogleApiClient.OnConnectionFailedListener;
import com.google.android.gms.location.LocationCallback;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;

import com.google.android.gms.location.LocationAvailability;
import com.google.android.gms.location.LocationResult;

import java.util.Map;


public class GmsLocationProvider
			  implements ConnectionCallbacks, OnConnectionFailedListener
{
	public static final String TAG = "Grym/GmsLocProvider";
	
	public final static int STATUS_DISCONNECTED			= 0;
	public final static int STATUS_CONNECTED			= 1;
	public final static int STATUS_CONNECTION_ERROR		= 2;
	public final static int STATUS_CONNECTION_SUSPENDED	= 3;
	public final static int STATUS_REQUEST_SUCCESS		= 4;
	public final static int STATUS_REQUEST_FAIL			= 5;

	private long native_ptr_ = 0;

	private GoogleApiClient mGoogleApiClient;
	private Location mCurrentLocation;
	private long mLastRequestId = 0;
	

	private class RequestHolder {
		public long mRequestId;
		public LocationRequest mRequest = null;
		public LocationCallback mCallback = null;
	}

	private Map<Long, RequestHolder> mRequests = new LinkedHashMap<Long, RequestHolder>();


	public GmsLocationProvider(long native_ptr)
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
	}


	@Override
	public void onConnected(Bundle connectionHint) 
	{
		Log.i(TAG, "Connected to GoogleApiClient");

		Location locationToSend = null;

		try
		{
			Location lastLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);

			synchronized (mRequests) {
				if (null == mCurrentLocation) {
					mCurrentLocation = lastLocation;
				} else if ((null != lastLocation) && (null != mCurrentLocation)) {
					if (lastLocation.getTime() > mCurrentLocation.getTime()) {
						mCurrentLocation = lastLocation;
					}
				}
				locationToSend = mCurrentLocation;
			}
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}

		if (null != locationToSend) {
			googleApiClientLocation(native_ptr_, locationToSend, true, 0);
		}

		googleApiClientStatus(native_ptr_, STATUS_CONNECTED);
		processAllRequests();
	}


	@Override
	public void onConnectionSuspended(int cause) 
	{
		Log.i(TAG, "Connection suspended, cause = " + cause);
		googleApiClientStatus(native_ptr_, STATUS_CONNECTION_SUSPENDED);
	}


	@Override
	public void onConnectionFailed(ConnectionResult result) 
	{
		Log.i(TAG, "Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
		googleApiClientStatus(native_ptr_, STATUS_CONNECTION_ERROR);
	}


	private RequestHolder reinitRequest(Long key)
	{
		synchronized (mRequests) {
			if (mRequests.containsKey(key)) {
				RequestHolder holder = mRequests.get(key);

				if (null != mGoogleApiClient && null != holder.mCallback) {
					try {
						LocationServices.FusedLocationApi.removeLocationUpdates(mGoogleApiClient, holder.mCallback);
					} catch (Exception e) {
						Log.e(TAG, "Failed to removeLocationUpdates: " + e.getMessage());
					}
				}
			}

			mRequests.remove(key);

			RequestHolder holder = new RequestHolder();
			holder.mRequestId = key;

			mRequests.put(key, holder);
			return holder;
		}
	}


	private void processAllRequests()
	{
		synchronized (mRequests) {
			for (Long key : mRequests.keySet()) {
				processRequest(key);
			}
		}
	}


	private void processRequest(Long key)
	{
		try
		{
			if (mGoogleApiClient.isConnected())
			{
				synchronized (mRequests) {
					RequestHolder holder = mRequests.get(key);
					LocationServices.FusedLocationApi.requestLocationUpdates(mGoogleApiClient, holder.mRequest, holder.mCallback, Looper.getMainLooper());
				}
			}
			else if (!mGoogleApiClient.isConnecting())
			{
				mGoogleApiClient.connect();
			}
		}
		catch(Exception e)
		{
			Log.e(TAG, "Failed to connect GoogleApiClient: " + e.getMessage());
		}
	}


	public long startLocationUpdates(final int priority,
									 final long interval,
									 final long fastestInterval,
									 final long maxWaitTime,
									 final int numUpdates,
									 final long expirationDuration,
									 final long expirationTime)
	{
		Log.i(TAG, "startLocationUpdates");

		RequestHolder holder = reinitRequest(++mLastRequestId);
		holder.mRequest = new LocationRequest();
		holder.mRequest
				.setPriority(priority)
				.setInterval(interval)
				.setFastestInterval(fastestInterval);

		if (maxWaitTime > 0) {
			holder.mRequest.setMaxWaitTime(maxWaitTime);
		}

		if (numUpdates > 0) {
			holder.mRequest.setNumUpdates(numUpdates);
		}

		if (expirationDuration > 0) {
			holder.mRequest.setExpirationDuration(expirationDuration);
		}

		if (expirationTime > 0) {
			holder.mRequest.setExpirationTime(expirationTime);
		}

		final long requestId = holder.mRequestId;

		holder.mCallback = new LocationCallback() {
			@Override
			public void onLocationAvailability (LocationAvailability locationAvailability)
			{
				boolean available = locationAvailability.isLocationAvailable();
				googleApiClientStatus(native_ptr_, available ? STATUS_REQUEST_SUCCESS : STATUS_REQUEST_FAIL);
			}

			@Override
			public void onLocationResult(LocationResult result)
			{
				Location location = result.getLastLocation();

				synchronized (mRequests) {
					mCurrentLocation = location;
				}

				if (null != location) {
					googleApiClientLocation(native_ptr_, location, false, requestId);
				}
			}
		};

		processRequest(holder.mRequestId);
		return holder.mRequestId;
	}


	public void stopLocationUpdates(long id)
	{
		Log.d(TAG, "stopLocationUpdates(" + id + ")" );

		synchronized (mRequests) {
			reinitRequest(id);
			mRequests.remove(id);
		}
	}


	static public boolean isAvailable(final Activity activity, final boolean allowDialog)
	{
		try {
			final GoogleApiAvailability apiAvailability = GoogleApiAvailability.getInstance();
			final int errorCode = apiAvailability.isGooglePlayServicesAvailable(activity);

			switch (errorCode) {
				case ConnectionResult.SERVICE_MISSING:
				case ConnectionResult.SERVICE_VERSION_UPDATE_REQUIRED:
				case ConnectionResult.SERVICE_DISABLED:

					if (allowDialog) {
						activity.runOnUiThread(new Runnable() {
							@Override
							public void run() {
								Dialog dialog = apiAvailability.getErrorDialog(activity, errorCode, 1);
								if (null != dialog) {
									dialog.show();
								}
							}
						});
					}
			}

			return ConnectionResult.SUCCESS == errorCode;
		}
		catch(Exception e) {
			Log.e(TAG, e.getMessage());
		}

		return false;
	}


	static public int getGmsVersion(Activity activity)
	{
		int versionCode = 0;

		try {
			versionCode = activity.getPackageManager().getPackageInfo(GoogleApiAvailability.GOOGLE_PLAY_SERVICES_PACKAGE, 0).versionCode;
		}
		catch(Exception e) {
			Log.e(TAG, e.getMessage());
		}

		return versionCode;
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	public native Activity getActivity();
	public native void googleApiClientStatus(long nativeptr, int status);
	public native void googleApiClientLocation(long nativeptr, Location location, boolean initial, long requestId);
}
