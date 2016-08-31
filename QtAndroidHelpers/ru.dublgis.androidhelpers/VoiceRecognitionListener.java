/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2016, DoubleGIS, LLC.
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

import android.os.Bundle;
import android.speech.RecognitionListener;

public class VoiceRecognitionListener implements RecognitionListener {
    private long mNativePtr = 0;

    public void setNativePtr(long nativePtr)
    {
        synchronized(this) {
            mNativePtr = nativePtr;
        }
    }

    @Override
    public void onBeginningOfSpeech()
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnBeginningOfSpeech(mNativePtr);
            }
        }
    }

    @Override
    public void onBufferReceived(byte[] buffer)
    {
        // Unsupported yet
    }

    @Override
    public void onEndOfSpeech()
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnEndOfSpeech(mNativePtr);
            }
        }
    }

    @Override
    public void onError(int error)
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnError(mNativePtr, error);
            }
        }
    }

    @Override
    public void onEvent(int eventType, Bundle params)
    {
        // Unsupported yet
    }

    @Override
    public void onPartialResults(Bundle partialResults)
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnPartialResults(mNativePtr, partialResults);
            }
        }
    }

    @Override
    public void onReadyForSpeech(Bundle params)
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnReadyForSpeech(mNativePtr, params);
            }
        }
    }

    @Override
    public void onResults(Bundle results)
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnResults(mNativePtr, results);
            }
        }

    }

    @Override
    public void onRmsChanged(float rmsdB)
    {
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnRmsChanged(mNativePtr, rmsdB);
            }
        }

    }


    public native void nativeOnBeginningOfSpeech(long ptr);
    // public native void onBufferReceived(long ptr byte[] buffer);
    public native void nativeOnEndOfSpeech(long ptr);
    public native void nativeOnError(long ptr, int error);
    // public native void nativeOnEvent(long ptr, int eventType, Bundle params);
    public native void nativeOnPartialResults(long ptr, Bundle partialResults);
    public native void nativeOnReadyForSpeech(long ptr, Bundle params);
    public native void nativeOnResults(long ptr, Bundle results);
    public native void nativeOnRmsChanged(long ptr, float rmsdB);

}



