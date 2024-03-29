/*
  Offscreen Android Views library for Qt

  Author:
  Artöm R. Khvosch <a.khvosch@2gis.ru>

  Distrbuted under The BSD License

  Copyright (c) 2016, DoubleGIS, LLC.
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
import android.location.*;
import android.content.Context;
import ru.dublgis.androidhelpers.Log;


public class NmeaListener implements OnNmeaMessageListener
{
	public static final String TAG = "Grym/NmeaListener";

	private long mNativePtr = 0;
	private boolean listening_= false;

	NmeaListener(long native_ptr)
	{
		mNativePtr = native_ptr;
	}

	public void StartListening()
	{
		Log.i(TAG, "start nmea listening from thread " + Thread.currentThread().getName());
		if (listening_)
		{
			return;
		}

		try
		{
			getActivity().runOnUiThread(new Runnable()
			{
				public void run()
				{
					try
					{
						LocationManager location_manager = (LocationManager) getContext().getSystemService(Context.LOCATION_SERVICE);

						if (location_manager != null)
						{
							Log.i(TAG, "try to add listening from thread " + Thread.currentThread().getName());
							boolean added = location_manager.addNmeaListener(NmeaListener.this);
							if (added)
							{
								listening_= true;
								Log.i(TAG, "nmea listender added");
							}
							else
							{
								Log.e(TAG, "failing add nmea listener");
							}
						}
						else
						{
							Log.e(TAG, "Can not get location manager.");
						}
					}
					catch (Exception e)
					{
						Log.e(TAG, "Exception when starting nmea listener: ", e);
					}
				}
			});
		}
		catch (Exception e)
		{
			Log.e(TAG, "Exception when posting a runnable:", e);
		}
	}

	public void StopListening()
	{
		Log.i(TAG, "stop nmea listening from thread " + Thread.currentThread().getName());
		if (!listening_)
		{
			return;
		}

		try
		{
			getActivity().runOnUiThread(new Runnable()
			{
				public void run()
				{
					try
					{
						LocationManager location_mangaer = (LocationManager) getContext().getSystemService(Context.LOCATION_SERVICE);

						if (location_mangaer != null)
						{
							Log.i(TAG, "remove nmea listener from thread " + Thread.currentThread().getName());
							location_mangaer.removeNmeaListener(NmeaListener.this);
							listening_= false;
						}
						else
						{
							Log.e(TAG, "Can not get location manager.");
						}
					}
					catch (Exception e)
					{
						Log.e(TAG, "Exception when removing nmea listener: ", e);
					}
				}
			});
		}
		catch (Exception e)
		{
			Log.e(TAG, "Exception when posting a runnable:", e);
		}
	}

	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public synchronized void cppDestroyed()
	{
		mNativePtr = 0;
	}

	@Override
	public synchronized void onNmeaMessage(String nmea, long timestamp)
	{
		try
		{
			OnNmeaReceivedNative(mNativePtr, timestamp, nmea);
		}
		catch (Exception e)
		{
			Log.e(TAG, "exception calling native method OnNmeaReceived", e);
		}
	}

	private native Activity getActivity();
	private native Context getContext();
	private native void OnNmeaReceivedNative(long nativeptr, long timestamp, String nmea);
}
