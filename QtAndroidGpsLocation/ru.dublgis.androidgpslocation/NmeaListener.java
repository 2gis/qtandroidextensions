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
package ru.dublgis.androidgpslocation;

import android.app.Activity;
import android.util.Log;
import android.os.Bundle;

import android.location.*;

import android.content.Context;

public class NmeaListener implements GpsStatus.NmeaListener
{
	public static final String TAG = "Grym/NmeaListener";

	private long native_ptr_ = 0;
	private boolean listening_= false;

	NmeaListener(long native_ptr)
	{
		native_ptr_ = native_ptr;
	}

	public void StartListening()
	{
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
	public void cppDestroyed()
	{
		native_ptr_= 0;
	}

	@Override
	public void onNmeaReceived(long timestamp, String nmea)
	{
		try
		{
			OnNmeaReceived(native_ptr_, timestamp, nmea);
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
