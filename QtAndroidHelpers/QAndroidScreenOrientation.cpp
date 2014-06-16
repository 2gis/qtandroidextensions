/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
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

#include "QAndroidScreenOrientation.h"


/*

private static int mRestoreScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;


        if (QtActivity.openGlMode() && !mShouldRestoreScreenOrientation) {
            int ro = mActivity.getRequestedOrientation();
            if (ro != ActivityInfo.SCREEN_ORIENTATION_NOSENSOR) {
                mRestoreScreenOrientation = ro;
                mShouldRestoreScreenOrientation = true;
                Log.d(QtActivity.mTag, "Disabling screen rotation. Previous value: "+mRestoreScreenOrientation);
                mActivity.setRequestedOrientation(getCurrentScreenOrientation(mActivity));
            }
        }


            activity.setRequestedOrientation(mRestoreScreenOrientation);


    private int getCurrentScreenOrientation(Activity a)
    {
        // Thanks to StackOverflow for idea of this hack, somewhat bugfixed and refactored though.
        try {
            DisplayMetrics dm = null;
            try {
                dm = new DisplayMetrics();
                a.getWindowManager().getDefaultDisplay().getMetrics(dm);
                int rotation = a.getWindowManager().getDefaultDisplay().getRotation();
                int orientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
                if (((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180)
                        && dm.heightPixels > dm.widthPixels) || // Not rotated or turned over & portrait
                    ((rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) 
                        && dm.widthPixels > dm.heightPixels)) // Rotated 90x & landscape
                {
                    // Device's natural orientation is portrait:
                    switch (rotation)
                    {
                        case Surface.ROTATION_0:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=portrait, rotation=0 => portrait");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                            break;
                        case Surface.ROTATION_90:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=portrait, rotation=90 => landscape");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                            break;
                        case Surface.ROTATION_180:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=portrait, rotation=180 => reverse portrait");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                            break;
                        case Surface.ROTATION_270:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=portrait, rotation=270 => reverse landscape");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                            break;
                        default:
                            Log.w(QtActivity.mTag, "RSZ getCurrentScreenOrientation Unknown screen orientation (1): "+rotation);
                            break;
                    }
                }
                else
                {
                    // Device's natural orientation is landscape (or if the device is square):
                    switch (rotation)
                    {
                        case Surface.ROTATION_0:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=landscape, rotation=0 => landscape");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                            break;
                        case Surface.ROTATION_90:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=landscape, rotation=90 => reverse portrait");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                            break;
                        case Surface.ROTATION_180:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=landscape, rotation=180 => reverse landscape");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                            break;
                        case Surface.ROTATION_270:
                            Log.d(QtActivity.mTag, "RSZ getCurrentScreenOrientation default=landscape, rotation=270 => portrait");
                            orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                            break;
                        default:
                            Log.w(QtActivity.mTag, "RSZ getCurrentScreenOrientation Unknown screen orientation (2): "+rotation);
                            break;
                    }
                }
                return orientation;
            } catch (Exception e) {
                Log.e(QtActivity.mTag, "RSZ getCurrentScreenOrientation exception (2): "+e);
            }
            if (dm == null)
                return ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
            if (dm.heightPixels <= dm.widthPixels)
                return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
            else
                return ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        } catch (Exception e) {
            Log.e(QtActivity.mTag, "RSZ getCurrentScreenOrientation exception (1): "+e);
        }
        return ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
    }

*/
