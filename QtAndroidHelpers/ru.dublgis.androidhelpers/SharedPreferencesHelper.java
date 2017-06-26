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

import android.content.SharedPreferences;
import android.content.Context;


final public class SharedPreferencesHelper
{
	static final private String TAG = "Grym/ShrdPrefHelper";
	private volatile long native_ptr_ = 0;


	SharedPreferencesHelper(long native_ptr)
	{
		Log.i(TAG, "constructor");
		native_ptr_ = native_ptr;
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		native_ptr_ = 0;
	}


	private SharedPreferences getPreferences()
	{
		Context ctx = getContext();
		String name = ctx.getPackageName() + ".SharedPreferencesHelper";
		return ctx.getSharedPreferences(name, Context.MODE_PRIVATE);
	}


	public void WriteString(String key, String value)
	{
		try {
			Log.i(TAG, "WriteString " + value + " by key " + key);
			SharedPreferences sharedPref = getPreferences();
			SharedPreferences.Editor editor = sharedPref.edit();
			editor.putString(key, value);
			editor.apply();
		} catch (final Throwable e) {
			Log.e(TAG, "WriteString exception: ", e);
		}
	}

	public String ReadString(String key, String valueDefault)
	{
		try {
			SharedPreferences sharedPref = getPreferences();
			return sharedPref.getString(key, valueDefault);
		} catch (final Throwable e) {
			Log.e(TAG, "ReadString exception: ", e);
			return valueDefault;
		}
	}



	public void WriteInt(String key, int value)
	{
		try {
			SharedPreferences sharedPref = getPreferences();
			SharedPreferences.Editor editor = sharedPref.edit();
			editor.putInt(key, value);
			editor.apply();
		} catch (final Throwable e) {
			Log.e(TAG, "WriteInt exception: ", e);
		}
	}

	public int ReadInt(String key, int valueDefault)
	{
		try {
			SharedPreferences sharedPref = getPreferences();
			return sharedPref.getInt(key, valueDefault);
		} catch (final Throwable e) {
			Log.e(TAG, "ReadInt exception: ",  e);
			return valueDefault;
		}
	}


	public native Context getContext();
}

