/*
  Offscreen Android Views library for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>
  Eugene A. Samoylov <ghelius@gmail.com>

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


package ru.dublgis.androidcompass;

import android.app.Activity;
import android.content.Context;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEventListener;
import android.hardware.SensorEvent;
import android.view.Surface;

import ru.dublgis.androidhelpers.Log;


public class OrientationRotationProvider implements SensorEventListener, OrientationProviderInterface {

	private static final String TAG = "Grym/OrientationRotationProvider";
	private long mNativePtr = 0;
	private volatile boolean mRegistered = false;

	private SensorManager mSensorManager;
	private Sensor mOrientation;

	private final float[] mOrientationVector = new float[9];
	private final float[] mRotationVector = new float[9];

	OrientationRotationProvider(long native_ptr) {
		mNativePtr = native_ptr;

		try {
			// initialize your android device sensor capabilities
			mSensorManager = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
			if (mSensorManager != null) {
				mOrientation = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
			} else {
				Log.w(TAG, "SensorManager is null!");
			}
		} catch (final Throwable e) {
			Log.e(TAG, "Exception while getting sensors: ", e);
		}
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed() {
		synchronized(this) {
			mNativePtr = 0;
			stop();
		}
	}


	public boolean start(int samplingPeriodUs, int maxReportLatencyUs) {
		Log.i(TAG, "start");

		if (null == mSensorManager) {
			Log.e(TAG, "Sensor manager is null");
			return false;
		}

		if (mRegistered) {
			Log.w(TAG, "Listeners already registered");
			return false;
		}

		/* API 9, not 19. That is not a mistake. */
		if ((android.os.Build.VERSION.SDK_INT < 9) || (samplingPeriodUs < 0)) {
			samplingPeriodUs = SensorManager.SENSOR_DELAY_NORMAL;
			maxReportLatencyUs = SensorManager.SENSOR_DELAY_UI;
		}

		mRegistered = true;

		try {
			if (android.os.Build.VERSION.SDK_INT >= 19 && maxReportLatencyUs > 0) {
				mRegistered = mRegistered && mSensorManager.registerListener(this, mOrientation, samplingPeriodUs, maxReportLatencyUs);
			}
			else {
				mRegistered = mRegistered && mSensorManager.registerListener(this, mOrientation, samplingPeriodUs);
			}

			Log.i(TAG, "Sensor listener registered successfully with samplingPeriodUs = " + samplingPeriodUs);
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


	public int getAzimuth(boolean applyDisplayRotation) {

		int value = (int)Math.toDegrees(SensorManager.getOrientation(mRotationVector, mOrientationVector)[0]);

		int angleShift = 0;

		if (applyDisplayRotation) {
			try {
				int rotation = getActivity().getWindowManager().getDefaultDisplay().getRotation();

				if (Surface.ROTATION_0 == rotation) {
					angleShift = 0;
				} else if (Surface.ROTATION_90 == rotation) {
					angleShift = 90;
				} else if (Surface.ROTATION_180 == rotation) {
					angleShift = 180;
				} else if (Surface.ROTATION_270 == rotation) {
					angleShift = 270;
				}
			} catch (final RuntimeException e) {
				// Most likely cause: android.os.DeadSystemException
				Log.e(TAG, "Failed to get rotation due to RuntimeException: " + e);
			} catch (final Throwable e) {
				Log.e(TAG, "Failed to get rotation: ", e);
			}
		}

		return (angleShift + value + 360) % 360;

	}


	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// Do something here if sensor accuracy changes.
		// You must implement this callback in your code.
	}


	// Get readings from accelerometer and magnetometer. To simplify calculations,
	// consider storing these readings as unit vectors.
	@Override
	public void onSensorChanged(SensorEvent event) {
		synchronized(this) {
			if(event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR)
			{
				SensorManager.getRotationMatrixFromVector(mRotationVector, event.values);

			}
			onUpdate(mNativePtr);
		}
	}

	private native Activity getActivity();
	private native void onUpdate(long nativeptr);
};

