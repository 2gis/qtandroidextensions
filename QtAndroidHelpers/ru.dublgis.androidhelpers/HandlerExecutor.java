/*
    Android helper library for Qt

    Author:
    Sergey A. Galin <sergey.galin@gmail.com>

    Distrbuted under The BSD License

    Copyright (c) 2023, DoubleGIS, LLC.
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

import java.lang.Thread;

import android.os.Build;
import android.os.Handler;


public class HandlerExecutor {
     public static final String TAG = "Grym/HandlerExecutor";
     private Object mMutex = new Object();
     private long mNativePtr = 0;
     private final Handler mHandler;


     HandlerExecutor(final long nativePtr, final Handler handler) {
         mNativePtr = nativePtr;
         mHandler = handler;
     }


     public void terminate() {
         synchronized (mMutex) {
             mNativePtr = 0;
         }
     }


     public void execute() {
         try {
             mHandler.post(() -> {
                 try {
                     synchronized (mMutex) {
                        if (mNativePtr != 0) {
                            nativeCallback(mNativePtr);
                        }
                    }
                 } catch (final Throwable e) {
                     Log.e(TAG, "Callback exception: ", e);
                 }
             });
         } catch (final Throwable e) {
             Log.e(TAG, "Handler.post() exception: ", e);
         }
     }


     public boolean isHandlerThread() {
         try {
             if (Build.VERSION.SDK_INT >= 23) { // Android >= 6.0
                 return mHandler.getLooper().isCurrentThread();
             } else {
                 return Thread.currentThread() == mHandler.getLooper().getThread();
             }
         } catch (final Throwable e) {
             Log.e(TAG, "isHandlerThread() exception: ", e);
             return false;
         }
     }


     public static native void nativeCallback(long nativePtr);
}