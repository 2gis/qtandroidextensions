/*
  Offscreen Android Views library for Qt

  Author:
  Eugene Samoylov <ghelius@gmail.com>

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


package ru.dublgis.androidaccelerometer;

import android.app.Activity;
import android.content.Context;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEventListener;
import android.hardware.SensorEvent;

import ru.dublgis.androidhelpers.Log;


public class AccelerometerProvider implements SensorEventListener {

	private static final String TAG = "Grym/Accelerometer";
	private volatile long mNativePtr = 0;
	private volatile boolean mRegistered = false;

	private SensorManager mSensorManager;
	private Sensor mAccelerometer;

	private final float[] mAccelerometerReading = new float[3];

	AccelerometerProvider(long native_ptr) {
		mNativePtr = native_ptr;

		try {
			// initialize your android device sensor capabilities
			mSensorManager = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
			if (mSensorManager != null) {
				mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
			} else {
				Log.w(TAG, "SensorManager is null!");
			}
		} catch (final Throwable e) {
			Log.e(TAG, "Exception while getting sensors: ", e);
		}
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed() {
		synchronized (this) {
			mNativePtr = 0;
			stop();
		}
	}


	public boolean start(int samplingPeriodMicroSeconds, int maxReportLatencyMicroSeconds) {
		Log.i(TAG, "start");

		if (null == mSensorManager) {
			Log.e(TAG, "Sensor manager is null");
			return false;
		}

		if (mRegistered) {
			Log.w(TAG, "Listeners already registered");
			return false;
		}

		// Android < 2.3
		if ((android.os.Build.VERSION.SDK_INT < 9) || (samplingPeriodMicroSeconds < 0)) {
			samplingPeriodMicroSeconds = SensorManager.SENSOR_DELAY_NORMAL;
			maxReportLatencyMicroSeconds = SensorManager.SENSOR_DELAY_UI;
		}

		mRegistered = true;

		try {
			// Android >= 4.4
			if (android.os.Build.VERSION.SDK_INT >= 19 && maxReportLatencyMicroSeconds > 0) {
				mRegistered = mRegistered && mSensorManager.registerListener(this, mAccelerometer, samplingPeriodMicroSeconds, maxReportLatencyMicroSeconds);
			}
			else {
				mRegistered = mRegistered && mSensorManager.registerListener(this, mAccelerometer, samplingPeriodMicroSeconds);
			}

			Log.i(TAG, "Sensor listener registered successfully with samplingPeriodUs = " + samplingPeriodMicroSeconds);
		}
		catch(Throwable e) {
			Log.e(TAG, "Failed to register listeners", e);
			mRegistered = false;
		}

		return mRegistered;
	}


	public void stop() {
		Log.i(TAG, "stop");

		if (null == mSensorManager) {
			Log.e(TAG, "SensorManager is null");
			return;
		}

		try {
			Log.i(TAG, "Unregistering listener");
			// to stop the listener and save battery
			mSensorManager.unregisterListener(this);
			mRegistered = false;
		}
		catch(final Throwable e) {
			Log.e(TAG, "Failed to stop orientation listener: ", e);
		}
	}


	public float getAccelerationModule() {
		return (float)Math.sqrt(
				mAccelerometerReading[0] * mAccelerometerReading[0] +
						mAccelerometerReading[1] * mAccelerometerReading[1] +
						mAccelerometerReading[2] * mAccelerometerReading[2]);
	}


	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}


	@Override
	public void onSensorChanged(SensorEvent event) {
		synchronized(this) {
			if (event.sensor == mAccelerometer) {
				System.arraycopy(event.values, 0, mAccelerometerReading,
						0, mAccelerometerReading.length);
			}
			try {
				onUpdate(mNativePtr);
			}
			catch(final Throwable e) {
				Log.e(TAG, "Failed onUpdate native: ", e);
			}
		}
	}


	private native Activity getActivity();
	private native void onUpdate(long nativeptr);
};

