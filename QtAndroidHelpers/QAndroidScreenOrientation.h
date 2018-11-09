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

#pragma once
#include <QtCore/QObject>
#include <QJniHelpers/QAndroidQPAPluginGap.h>

namespace QAndroidScreenOrientation
{
	static const int
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_UNSPECIFIED			= -1,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_LANDSCAPE			= 0,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_PORTRAIT			= 1,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_USER				= 2,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_BEHIND				= 3,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_SENSOR				= 4,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_NOSENSOR			= 5,
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_SENSOR_LANDSCAPE	= 6, // API 9
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_SENSOR_PORTRAIT		= 7, // API 9
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_PORTRAIT	= 9, // API 9
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_LANDSCAPE	= 8, // API 9
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_FULL_SENSOR			= 10, // API 9
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_USER_LANDSCAPE		= 11, // API 18
		ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_USER_PORTRAIT		= 12; // API 18

	static const int
		ANDROID_SURFACE_ROTATION_UNDEFINED	= -1,
		ANDROID_SURFACE_ROTATION_0			=  0,
		ANDROID_SURFACE_ROTATION_90			=  1,
		ANDROID_SURFACE_ROTATION_180		=  2,
		ANDROID_SURFACE_ROTATION_270		=  3;

	//! Get current surface rotation as defined by android Display.getRotation()
	int getSurfaceRotation();

	//! Get currently requested screen orientation.
	int getRequestedOrientation();

	//! Request certain screen orientation.
	void setRequestedOrientation(int orientation);

	/*!
	 * Get a fixed (not affected by sensor) screen orientation constant which
	 * matches current actual screen orientation.
	 * This is useful for locking screen in the current orientation for some time,
	 * for example, during some application phases when screen rotation is not desirable.
	 * Upon error, returns ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_UNSPECIFIED.
	 */
	int getCurrentFixedOrientation();


	/*!
	 * This is a helper class to lock screen in a certain orientation.
	 */
	class OrientationLock: public QObject
	{
		Q_OBJECT
	public:
		/*!
		 * Creates a lock which keeps the screen in its orientaion of the moment of the locking.
		 */
		OrientationLock();

		/*!
		 * Creates a lock which locks the screen in a specified orientation.
		 */
		OrientationLock(int desired_orientation);

		/*!
		 * Restores requested orientation to the one prior to creating the lock.
		 */
		~OrientationLock();
	protected:
		int saved_orientation_;
		bool locked_;
	};

} // namespace QAndroidScreenOrientation

