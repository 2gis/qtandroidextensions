/*
	Offscreen Android Views library for Qt

	Author:
	Vyacheslav O. Koscheev <vok1980@gmail.com>
	Sergey A.Galin <sergey.galin@gmail.com>

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

package ru.dublgis.androidhelpers;

import android.content.Context;
import android.os.PowerManager;


class WakeLocker
{
	public static final String TAG = "Grym/WakeLocker";
	private PowerManager.WakeLock mLock = null;
	private int mMode = PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE;


	public WakeLocker(final long native_ptr)
	{
		Log.i(TAG, "WakeLocker constructor");
	}


	// From C++
	// This function should be called only once before lock/unlock.
	public void setMode(final int lockMode)
	{
		mMode = lockMode;
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
	}


	private void createLock()
	{
		Log.i(TAG, "createLock " + mMode);
		try
		{
			final PowerManager powerManager = (PowerManager)getContext().getSystemService(Context.POWER_SERVICE);
			if (powerManager != null)
			{
				mLock = powerManager.newWakeLock(mMode, TAG);
			}
			else
			{
				Log.w(TAG, "PowerManager is null! No control over backlight or suspend.");
			}
		}
		catch (final Throwable e)
		{
			Log.e(TAG, "Exception in PowerController (power manager init): " + e);
		}
	}

	public boolean Lock()
	{
		if (null == mLock)
		{
			createLock();
		}

		if (null == mLock)
		{
			Log.e(TAG, "Failed to create the lock!");
			return false;
		}

		try 
		{
			if (!mLock.isHeld())
			{
				mLock.acquire();
			}
		}
		catch (final Throwable e)
		{
			Log.e(TAG, "Failed to acquire lock for mode " + mMode + ": " + e);
		}

		return IsLocked();
	}


	public boolean IsLocked()
	{
		boolean ret = mLock != null && mLock.isHeld();
		Log.i(TAG, "IsLocked: " + ret + " (mode: " + mMode + ")");
		return ret;
	}


	public void Unlock()
	{
		Log.i(TAG, "Unlock " + mMode);

		if (null == mLock)
		{
			Log.e(TAG, "Trying to unlock when not locked!");
			return;
		}

		try
		{
			if (mLock.isHeld())
			{
				mLock.release();
			}
		}
		catch (final Throwable e)
		{
			Log.e(TAG, "Failed to release lock for mode " + mMode + ": " + e);
		}
	}


	public native Context getContext();
}

