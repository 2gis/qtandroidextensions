/*
	Offscreen Android Views library for Qt

	Author:
	Aleksey I. Gribanov <a.gribanov@2gis.ru>

	Distrbuted under The BSD License

	Copyright (c) 2022, DoubleGIS, LLC.
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

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;

import java.util.ArrayList;

public class AndroidScreenOrientationHelper
{
    public static final String TAG = "Grym/AndroidScreenOrientationHelper";
    private static final ArrayList<LockerOrientationInfo> mLockers = new ArrayList<LockerOrientationInfo>();

    public static class LockerOrientationInfo
    {
        private static int mNextId = 0;
        private final int mId;
        private int mSavedOrientation;

        public LockerOrientationInfo(int orientation)
        {
            mId = mNextId;
            mNextId++;
            mSavedOrientation = orientation;
        }

        public int getId() { return mId; }
        public int getSavedOrientation() { return mSavedOrientation; }
        public void setSavedOrientation(int orientation) { mSavedOrientation = orientation; }
    };

    public AndroidScreenOrientationHelper(long native_ptr)
    {
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
    }

    public int getRequestedOrientation()
    {
        return getRequestedOrientation((Activity)getContext());
    }

    public LockerOrientationInfo lockOrientation(int desiredOrientation)
    {
        return lockOrientation(desiredOrientation, (Activity)getContext());
    }

    public void releaseLocker(LockerOrientationInfo lockerInfo)
    {
        releaseLocker(lockerInfo, (Activity)getContext());
    }

    private void setRequestedOrientaion(int orientaion)
    {
        setRequestedOrientaion(orientaion, (Activity)getContext());
    }

    public static LockerOrientationInfo lockOrientation(int desiredOrientation, Activity activity)
    {
        LockerOrientationInfo locker = new LockerOrientationInfo(getRequestedOrientation(activity));
        synchronized(AndroidScreenOrientationHelper.class) {
            mLockers.add(locker);
        }
        setRequestedOrientaion(desiredOrientation, activity);
        return locker;
    }

    public static void releaseLocker(LockerOrientationInfo lockerInfo, Activity activity)
    {
        synchronized(AndroidScreenOrientationHelper.class) {
            int lockersSize = mLockers.size();
            if (lockersSize == 0) {
                return;
            }

            // Check last locker
            if (mLockers.get(lockersSize - 1).getId() == lockerInfo.getId()) {
                // Restore saved orientation
                setRequestedOrientaion(lockerInfo.getSavedOrientation(), activity);
                mLockers.remove(lockersSize - 1);
                return;
            }

            for (int i = lockersSize - 2; i >= 0; --i) {
                LockerOrientationInfo locker = mLockers.get(i);
                if (locker.getId() == lockerInfo.getId()) {
                    // Change saved orientation for "next" locker in queue
                    mLockers.get(i + 1).setSavedOrientation(locker.getSavedOrientation());
                    mLockers.remove(i);
                    return;
                }
            }
        }

        Log.w(TAG, "releaseLocker failed: locker not found. LockerOrientationInfo.getId(): " + lockerInfo.getId());
    }

    public static int getRequestedOrientation(Activity activity)
    {
        int requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
        try {
            requestedOrientation = activity.getRequestedOrientation();
        } catch (final Throwable e) {
            Log.e(TAG, "getRequestedOrientation failed: " + e);
        }
        return requestedOrientation;
    }

    private static void setRequestedOrientaion(int orientaion, Activity activity)
    {
        try {
            activity.setRequestedOrientation(orientaion);
        } catch (final Throwable e) {
            Log.e(TAG, "setRequestedOrientaion failed: " + e);
        }
    }

    public native Context getContext();
}
