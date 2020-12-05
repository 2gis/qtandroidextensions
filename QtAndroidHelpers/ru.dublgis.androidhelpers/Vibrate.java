/*
	Offscreen Android Views library for Qt

	Author:
	Vyacheslav O. Koscheev <vok1980@gmail.com>

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
package ru.dublgis.androidhelpers;

import android.content.Context;
import android.os.Build;
import android.os.Vibrator;
import android.os.VibrationEffect;

public class Vibrate {
	public static final String TAG = "Grym/Vibrator";
	private volatile long native_ptr_ = 0;


	public Vibrate(final long native_ptr)
	{
		Log.i(TAG, "Constructed");
		native_ptr_ = native_ptr;
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		native_ptr_ = 0;
	}


	public void vibrate(final long[] timings)
	{
		try
		{
			Context context = getContext();
			final Vibrator vibrator = (Vibrator)context.getSystemService(Context.VIBRATOR_SERVICE);
			if (null == vibrator)
			{
				Log.w(TAG, "Vibrator service not found.");
				return;
			}
			// Android 6+
			if (Build.VERSION.SDK_INT >= 26) {
				VibrationEffect effect = VibrationEffect.createWaveform(timings, -1);
				vibrator.vibrate(effect);
			} else {
				vibrator.vibrate(timings, -1);
			}
		}
		catch (final LinkageError e)
		{
			Log.e(TAG, "Vibrator linkage error: " + e);
		}
		catch (final Throwable e)
		{
			Log.e(TAG, "Vibrator exception: ", e);
		}
	}

	public native Context getContext();
}
