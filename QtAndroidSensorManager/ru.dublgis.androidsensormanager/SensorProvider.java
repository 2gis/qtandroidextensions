/*
  Offscreen Android Views library for Qt

  Author:
  Eugene Samoylov <ghelius@gmail.com>
  Vyacheslav Koscheev <vok1980@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2024, DoubleGIS, LLC.
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


package ru.dublgis.androidsensormanager;

import android.app.Activity;
import android.content.Context;
import android.hardware.SensorManager;
import android.hardware.Sensor;
import android.hardware.SensorEventListener;
import android.hardware.SensorEvent;
import android.view.Surface;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import ru.dublgis.androidhelpers.Log;


public class SensorProvider implements SensorEventListener {

	private static final String TAG = "Grym/SensorProvider";
	private long mNativePtr = 0;

	private SensorManager mSensorManager = null;

	public class Data {
		private volatile boolean mRegistered = false;
		private Sensor mSensor = null;
		private float[] mData = null;
	};

	private Map<Integer, Data> mSensorData = new TreeMap<Integer, Data>();


	SensorProvider(long native_ptr) {
		mNativePtr = native_ptr;
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed() {
		synchronized (this) {
			mNativePtr = 0;
			stopAll();
		}
	}


	public boolean start(int sensorType, int samplingPeriodMicroSeconds, int maxReportLatencyMicroSeconds) {
		Log.i(TAG, "start, sensorType = " + sensorType);

		Data sensorData = new Data();

		try {
			if (null == mSensorManager) {
				mSensorManager = (SensorManager) getActivity().getSystemService(Context.SENSOR_SERVICE);
			}

			if (mSensorData.containsKey(sensorType)) {
				Log.w(TAG, "Listeners already registered");
				return false;
			}

			if (null != mSensorManager) {
				sensorData.mSensor = mSensorManager.getDefaultSensor(sensorType);
			} else {
				Log.w(TAG, "SensorManager is null!");
				return false;
			}

			if (null == sensorData.mSensor) {
				Log.w(TAG, "Sensor is null");
				return false;
			}
		} catch (final Throwable e) {
			Log.e(TAG, "Exception while getting sensors: ", e);
		}

		// Android < 2.3
		if ((Build.VERSION.SDK_INT < 9) || (samplingPeriodMicroSeconds < 0)) {
			samplingPeriodMicroSeconds = SensorManager.SENSOR_DELAY_NORMAL;
			maxReportLatencyMicroSeconds = SensorManager.SENSOR_DELAY_UI;
		}

		sensorData.mRegistered = true;

		try {
			// Android >= 4.4
			if (Build.VERSION.SDK_INT >= 19 && maxReportLatencyMicroSeconds > 0) {
				sensorData.mRegistered = sensorData.mRegistered && mSensorManager.registerListener(this, sensorData.mSensor, samplingPeriodMicroSeconds, maxReportLatencyMicroSeconds);
			}
			else {
				sensorData.mRegistered = sensorData.mRegistered && mSensorManager.registerListener(this, sensorData.mSensor, samplingPeriodMicroSeconds);
			}

			Log.i(TAG, "Sensor listener registered successfully with samplingPeriodUs = " + samplingPeriodMicroSeconds);
		}
		catch(Throwable e) {
			Log.e(TAG, "Failed to register listeners", e);
			sensorData.mRegistered = false;
		}

		mSensorData.put(sensorType, sensorData);
		return sensorData.mRegistered;
	}


	public void stopAll() {
		Log.i(TAG, "stopAll");
		try {
			// stop() removes items from mSensorData, so we can't just iterate over it
			// and call stop(), that would case ConcurrentModificationException.
			final List<Integer> sensorTypes = new ArrayList<Integer>(mSensorData.size());
			for (Data data : mSensorData.values()) {
				sensorTypes.add(data.mSensor.getType());
			}
			for (Integer sensorType : sensorTypes) {
				try {
					stop(sensorType);
				} catch (final Throwable e) {
					Log.e(TAG, "Exception stopping sensor type=" + sensorType, e);
				}
			}
		} catch (final Throwable e) {
			Log.e(TAG, "Exception in stopAll: ", e);
		}
	}


	public void stop(int sensorType) {
		Log.i(TAG, "stop, sensorType = " + sensorType);

		if (null == mSensorManager) {
			Log.e(TAG, "SensorManager is null");
			return;
		}

		try {
			Log.i(TAG, "Unregistering listener for sensorType " + sensorType);
			Data data = mSensorData.get(sensorType);
			if (null != data) {
				mSensorManager.unregisterListener(this, data.mSensor);
			}
			mSensorData.remove(sensorType);
		} catch(final Throwable e) {
			Log.e(TAG, "Failed to stop sensor listener: ", e);
		}
	}


	public boolean isStarted(int sensorType) {
		try {
			if (mSensorData.containsKey(sensorType)) {
				return false;
			}
			Data data = mSensorData.get(sensorType);
			return data.mRegistered;
		} catch(final Throwable e) {
			Log.e(TAG, "Exception in isStarted(" + sensorType + "): ", e);
			return false;
		}
	}


	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}


	@Override
	public void onSensorChanged(SensorEvent event) {
		try {
			synchronized(this) {
				int sensorType = event.sensor.getType();

				if (mSensorData.containsKey(sensorType)) {

					Data data = mSensorData.get(sensorType);

					if (event.sensor == data.mSensor) {
						if (null == data.mData) {
							data.mData = new float[event.values.length];
						}

						System.arraycopy(
							event.values, 0,
							data.mData, 0, data.mData.length);

						mSensorData.put(sensorType, data);

						try {
							onUpdate(mNativePtr, sensorType, event.timestamp, data.mData);
						} catch(final Throwable e) {
							Log.e(TAG, "Failed onUpdate native: ", e);
						}
					}
				}
			}
		} catch(final Throwable e) {
			Log.e(TAG, "Failed onSensorChanged: ", e);
		}
	}


	public float displayRotation() {
		try {
			int rotation = getActivity().getWindowManager().getDefaultDisplay().getRotation();

			if (Surface.ROTATION_0 == rotation) {
				return 0;
			} else if (Surface.ROTATION_90 == rotation) {
				return 90;
			} else if (Surface.ROTATION_180 == rotation) {
				return 180;
			} else if (Surface.ROTATION_270 == rotation) {
				return 270;
			}
		} catch (final RuntimeException e) {
			// Most likely cause: android.os.DeadSystemException
			Log.e(TAG, "Failed to get rotation due to RuntimeException: " + e);
		} catch (final Throwable e) {
			Log.e(TAG, "Failed to get rotation: ", e);
		}
		return 0;
	}


	public float getAzimuthByRotationVector(boolean applyDisplayRotation) {
		double ret = 360.0;
		try {
			if (applyDisplayRotation) {
				ret += displayRotation();
			}

			synchronized(this) {
				if (mSensorData.containsKey(Sensor.TYPE_ROTATION_VECTOR)) {
					float[] rotationMatrix = new float[9];
					float[] orientationVector = new float[3];

					final Data data = mSensorData.get(Sensor.TYPE_ROTATION_VECTOR);

					if (null == data) {
						Log.e(TAG, "data is null");
						return 0.0f;
					}
					if (null == data.mData) {
						Log.e(TAG, "data.mData is null");
						return 0.0f;
					}

					SensorManager.getRotationMatrixFromVector(rotationMatrix, data.mData);
					ret = Math.toDegrees(SensorManager.getOrientation(rotationMatrix, orientationVector)[0]);
				}
			}

			ret = ret % 360.0;
		} catch (final Throwable e) {
			Log.e(TAG, "getAzimuthByRotationVector exception: ", e);
		}
		return (float)ret;
	}


	public float getAzimuthByMagneticField(boolean applyDisplayRotation) {
		double ret = 360.0;
		try {
			if (applyDisplayRotation) {
				ret += displayRotation();
			}

			synchronized(this) {
				if (mSensorData.containsKey(Sensor.TYPE_ACCELEROMETER) && mSensorData.containsKey(Sensor.TYPE_MAGNETIC_FIELD)) {
					float[] rotationMatrix = new float[9];
					float[] orientationVector = new float[3];

					final Data dataAccelerometer = mSensorData.get(Sensor.TYPE_ACCELEROMETER);
					final Data dataMagnetic = mSensorData.get(Sensor.TYPE_MAGNETIC_FIELD);

					if (null == dataAccelerometer) {
						Log.e(TAG, "dataAccelerometer is null");
						return 0.0f;
					}
					if (null == dataMagnetic) {
						Log.e(TAG, "dataMagnetic is null");
						return 0.0f;
					}
					if (null == dataAccelerometer.mData) {
						Log.e(TAG, "dataAccelerometer.mData is null");
						return 0.0f;
					}
					if (null == dataMagnetic.mData) {
						Log.e(TAG, "dataMagnetic.mData is null");
						return 0.0f;
					}

					SensorManager.getRotationMatrix(rotationMatrix, null, dataAccelerometer.mData, dataMagnetic.mData);
					float[] orientation = SensorManager.getOrientation(rotationMatrix, orientationVector);
					if (orientation == null) {
						Log.w(TAG, "Null orientation returned for compass");
					} else if (orientation.length < 1) {
						Log.w(TAG, "Empty orientation returned for compass");
					} else {
						ret += Math.toDegrees(orientation[0]);
					}
				}
			}

			while (ret > 360) {
				ret -= 360;
			}
		} catch (final Throwable e) {
			Log.e(TAG, "getAzimuthByMagneticField exception: ", e);
		}
		return (float)ret;
	}


	private native Activity getActivity();
	private native void onUpdate(long nativeptr, int sensorType, long timestamp_ns, float[] data);
};
