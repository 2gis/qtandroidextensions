/*
  Offscreen Android Views library for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2015, DoubleGIS, LLC.
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


package ru.dublgis.androidcompass;

import java.lang.Math;
import android.util.Log;
import android.content.Context;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEventListener;
import android.hardware.SensorEvent;


public class CompassProvider implements SensorEventListener
{
	private final String TAG = "Grym/CompassProvider";
	private long mNativePtr = 0;
	private SensorManager mSensorManager;
	private Sensor mOrientation;

	CompassProvider(long native_ptr)
	{
		mNativePtr = native_ptr;
		// initialize your android device sensor capabilities
		mSensorManager = (SensorManager) getContext().getSystemService(Context.SENSOR_SERVICE);

		if (null == mSensorManager)
		{
			Log.e(TAG, "SensorManager is null");
			return;
		}

		mOrientation = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		stop();
		mNativePtr = 0;
	}


	private static final int advancedListenerApiLevel = 19;


	public void start(int samplingPeriodUs, int maxReportLatencyUs)
	{
		if (null == mSensorManager || null == mOrientation)
		{
			Log.e(TAG, "Sensors are null");
			return;
		}

		/* API 9, not 19. That is not a mistake. */
		if ((android.os.Build.VERSION.SDK_INT < 9) || (samplingPeriodUs < 0))
		{
			samplingPeriodUs = SensorManager.SENSOR_DELAY_UI;
		}

		boolean registered = false;

		try
		{
			if (android.os.Build.VERSION.SDK_INT >= advancedListenerApiLevel)
			{
				if (maxReportLatencyUs < 0)
				{
					maxReportLatencyUs = samplingPeriodUs / 5;
				}

				Log.i(TAG, "Registering orientation listener for API >= " + advancedListenerApiLevel + 
							" with samplingPeriodUs = " + samplingPeriodUs + 
							", maxReportLatencyUs = " + maxReportLatencyUs);
				registered = mSensorManager.registerListener(this, mOrientation, samplingPeriodUs, maxReportLatencyUs);
			}
			else
			{
				Log.i(TAG, "Registering orientation listener for API < " + advancedListenerApiLevel + 
							" with samplingPeriodUs = " + samplingPeriodUs);
				registered = mSensorManager.registerListener(this, mOrientation, samplingPeriodUs);
			}
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}

		if (registered)
		{
			Log.i(TAG, "Sensor listener registered successfully");
		}
		else
		{
			Log.e(TAG, "Sensor listener failed to register");
		}
	}


	public void stop()
	{
		if (null == mSensorManager)
		{
			Log.e(TAG, "SensorManager is null");
			return;
		}

		try
		{
			Log.i(TAG, "Unregistering orientation listener");
			// to stop the listener and save battery
			mSensorManager.unregisterListener(this);
		}
		catch(Exception e)
		{
			Log.e(TAG, e.getMessage());
		}
	}


	public void onSensorChanged(SensorEvent event)
	{
		if (event.sensor.getType() == mOrientation.getType())
		{
			setAzimut(mNativePtr, event.values[0]);
		}
	}


	public void onAccuracyChanged (Sensor sensor, int accuracy)
	{
	}


	private native Context getContext();
	private native void setAzimut(long nativeptr, float azimut);
};

