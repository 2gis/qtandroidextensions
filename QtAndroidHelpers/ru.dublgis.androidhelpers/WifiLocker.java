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

package ru.dublgis.androidhelpers;

import android.content.Context;
import android.net.wifi.WifiManager;


class WifiLocker
{
	public static final String TAG = "Grym/WifiLocker";
	private volatile long native_ptr_ = 0;
	private WifiManager.WifiLock mLock = null;


	public WifiLocker(long native_ptr)
	{
		Log.i(TAG, "WifiLocker constructor");
		native_ptr_ = native_ptr;
	}

	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		native_ptr_ = 0;
	}


	private void createLock()
	{
		Log.i(TAG, "createLock");

		try 
		{
			WifiManager wifiManager = (WifiManager)getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
			
			if (wifiManager != null)
			{
				int mode = android.os.Build.VERSION.SDK_INT >= 12 ?
					WifiManager.WIFI_MODE_FULL_HIGH_PERF :
					WifiManager.WIFI_MODE_FULL;
				mLock = wifiManager.createWifiLock(mode, TAG);
			}
			else
			{
				Log.w(TAG, "WiFiManager is null! No control over WiFi connection state.");
			}
		}
		catch (Exception e)
		{
			Log.e(TAG, "Exception in WifiLocker (WiFi manager init): "+e);
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
			Log.e(TAG, "no mLock");
			return false;
		}

		try 
		{
			if (!mLock.isHeld())
			{
				mLock.acquire();
			}
		}
		catch (Exception e)
		{
			Log.e(TAG, "Failed to acquire lock: " + e);
		}

		return IsLocked();
	}


	public boolean IsLocked()
	{
		boolean ret = mLock != null && mLock.isHeld();
		Log.i(TAG, "IsLocked = " + ret);
		return ret;
	}


	public void Unlock()
	{
		Log.i(TAG, "Unlock");

		if (null == mLock)
		{
			Log.e(TAG, "no mLock");
			return;
		}

		try 
		{
			if (mLock.isHeld())
			{
				mLock.release();
			}
		}
		catch (Exception e)
		{
			Log.e(TAG, "Failed to release lock: " + e);
		}
	}


	public boolean IsWifiEnabled()
	{
		try 
		{
			WifiManager wifiManager = (WifiManager)getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
			return null != wifiManager && wifiManager.isWifiEnabled();
		}
		catch (Exception e)
		{
			Log.e(TAG, "Failed to call isWifiEnabled: " + e);
		}

		return false;
	}


	public native Context getContext();
}

