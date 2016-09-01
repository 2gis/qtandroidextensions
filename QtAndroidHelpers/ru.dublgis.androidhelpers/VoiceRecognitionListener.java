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

import java.util.ArrayList;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.speech.RecognitionListener;
import android.util.Log;


public class VoiceRecognitionListener implements RecognitionListener {
    private long mNativePtr = 0;
    private final String TAG = "Grym/SpeechRecognizer";
    private Activity mActivity = null;
    private SpeechRecognizer mSpeechRecognizer = null;

    // From C++
    public void setNativePtr(long nativePtr)
    {
        synchronized(this) {
            mNativePtr = nativePtr;
        }
    }

    // From C++
    public void initialize(final Activity activity)
    {
        mActivity = activity;
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                synchronized(this) {
                    try {
                        mSpeechRecognizer = SpeechRecognizer.createSpeechRecognizer(mActivity);
                        mSpeechRecognizer.setRecognitionListener(VoiceRecognitionListener.this);
                    } catch (final Exception e) {
                        Log.e(TAG, "Exception while creating SpeechRecognizer:", e);
                        mSpeechRecognizer = null;
                    }
                }
            }
        });
    }

    // From C++
    public void requestLanguageDetails()
    {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    Intent intent = new Intent(RecognizerIntent.ACTION_GET_LANGUAGE_DETAILS);
                    mActivity.sendOrderedBroadcast(
                        intent
                        , null
                        , new BroadcastReceiver() {
                            @Override
                            public void onReceive(Context context, Intent intent) {
                                // Log.d(TAG, "onReceive(" + intent.toUri(0) + ")");
                                if (getResultCode() != Activity.RESULT_OK) {
                                    return;
                                }
                                ArrayList<CharSequence> hints = getResultExtras(true)
                                   .getCharSequenceArrayList(RecognizerIntent.EXTRA_SUPPORTED_LANGUAGES);
                                ArrayList<String> result = new ArrayList<String>();
                                int sz = hints.size();
                                result.ensureCapacity(sz);
                                for (int i = 0; i < hints.size(); ++i) {
                                    result.add(hints.get(i).toString());
                                }
                                synchronized(this) {
                                    if (mNativePtr != 0) {
                                        nativeSupportedLanguagesReceived(mNativePtr, result);
                                    }
                                }
                            }
                        }
                        , null
                        , Activity.RESULT_OK
                        , null
                        , null);
                } catch (final Exception e) {
                    Log.e(TAG, "Exception in requestLanguageDetails:", e);
                    mSpeechRecognizer = null;
                }
            }
        });
    }

    // From C++
    public void startListening(final Intent intent)
    {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mSpeechRecognizer != null) {
                    mSpeechRecognizer.startListening(intent);
                } else {
                    Log.e(TAG, "startListening: the recognizer is null!");
                }
            }
        });
    }

    // From C++
    public void stopListening()
    {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mSpeechRecognizer != null) {
                    mSpeechRecognizer.stopListening();
                } else {
                    Log.e(TAG, "stopListening: the recognizer is null!");
                }
            }
        });
    }

    // From C++
    public void cancel()
    {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mSpeechRecognizer != null) {
                    mSpeechRecognizer.cancel();
                } else {
                    Log.e(TAG, "cancel: the recognizer is null!");
                }
            }
        });
    }


    @Override
    public void onBeginningOfSpeech()
    {
        Log.v(TAG, "onBeginningOfSpeech");
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
        Log.v(TAG, "onEndOfSpeech");
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnEndOfSpeech(mNativePtr);
            }
        }
    }

    @Override
    public void onError(int error)
    {
        Log.v(TAG, "onError " + error);
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
        // Log.v(TAG, "onEvent " + eventType);
    }

    @Override
    public void onPartialResults(Bundle partialResults)
    {
        Log.v(TAG, "onPartialResults");
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnPartialResults(mNativePtr, partialResults);
            }
        }
    }

    @Override
    public void onReadyForSpeech(Bundle params)
    {
        Log.v(TAG, "onReadyForSpeech");
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnReadyForSpeech(mNativePtr, params);
            }
        }
    }

    @Override
    public void onResults(Bundle results)
    {
        Log.v(TAG, "onResults");
        synchronized(this) {
            if (mNativePtr != 0) {
                nativeOnResults(mNativePtr, results);
            }
        }

    }

    @Override
    public void onRmsChanged(float rmsdB)
    {
        // Log.v(TAG, "onRmsChanged"); - super noisy
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
    public native void nativeSupportedLanguagesReceived(long ptr, ArrayList<String> languages);

}



