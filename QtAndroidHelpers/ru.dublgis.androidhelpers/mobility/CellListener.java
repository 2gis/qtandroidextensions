/*
    Offscreen Android Views library for Qt

    Authors:
    Vyacheslav O. Koscheev <vok1980@gmail.com>
    Ivan Avdeev marflon@gmail.com
    Sergey A. Galin sergey.galin@gmail.com

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

package ru.dublgis.androidhelpers.mobility;

import android.util.Log;
import android.content.Context;
import android.os.Looper;

import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import android.telephony.CellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.SignalStrength;




public class CellListener
{
    final static String LOG_TAG = "Grym/CellListener";
    final static boolean verbose = false;    

    private TelephonyManager mTelMan = null;
    private CellLocation mLastLocation = null;
    private long native_ptr_ = 0;

    private TrueCellListener mListener = null;
    private Thread mListenerThread = null;
    private Looper mListenerLooper = null;


    public CellListener(long native_ptr)
    {
        native_ptr_ = native_ptr;
    }


    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        native_ptr_ = 0;
    }


    private class TrueCellListener extends PhoneStateListener
    {
        public synchronized void onCellLocationChanged(CellLocation location)
        {
            if (verbose)
            {
                Log.d(LOG_TAG,"CellListener onCellLocationChanged: "+location);
            }

            mLastLocation = location;
        }


        public synchronized void onSignalStrengthsChanged(SignalStrength signalStrength)
        {
            if (verbose)
            {
                Log.d(LOG_TAG,"CellListener onSignalStrengthsChanged: "+signalStrength);
            }
            try
            {
                CellData data = null;
                if (signalStrength.isGsm())
                {
                    // Calculate dBm from asu
                    int dbm = signalStrength.getGsmSignalStrength();
                    if (dbm == 99)
                    {
                        dbm = -113;
                    }
                    else if ((dbm >= 0) && (dbm <= 31))
                    {
                        dbm = dbm * 2 - 113;
                    }
                    else
                    {
                        dbm = -51;
                    }

                    GsmCellLocation loc = (GsmCellLocation)mLastLocation;
                    if (loc != null)
                    {
                        data = new CellData(loc.getCid(), loc.getLac(), -1, -1, dbm, 0);
                    }
                } else {
                    CdmaCellLocation loc = (CdmaCellLocation)mLastLocation;
                    if (loc != null)
                    {
                        data = new CellData(loc.getBaseStationId(), loc.getNetworkId(),
                            -1, loc.getSystemId(), signalStrength.getCdmaDbm(), 0);
                    }
                }

                if (data == null)
                {
                    if (verbose)
                    {
                        Log.d(LOG_TAG,"CellListener onCellLocationChanged: null data!");
                    }
                    return;
                }

                String op = mTelMan.getNetworkOperator();

                try
                {
                    if (op != null && op.length() > 0)
                    {
                        if (data.mobileCountryCode_ == -1)
                        {
                            try
                            {
                                data.mobileCountryCode_ = Integer.parseInt(op.substring(0, 3));
                            }
                            catch(Exception e)
                            {
                                Log.e(LOG_TAG, "Failed to parse mobile country code from: \""+op+"\"");
                            }
                        }
                        if (data.mobileNetworkCode_ == -1)
                        {
                            try
                            {
                                data.mobileNetworkCode_ = Integer.parseInt(op.substring(3));
                            }
                            catch(Exception e)
                            {
                                Log.e(LOG_TAG, "Failed to parse mobile network code from: \""+op+"\"");
                            }
                        }
                    }
                    else // (op is empty)
                    {
                        // (Normal situaution without SIM card.)
                        // Log.d(LOG_TAG, "Network operator string is empty.");
                    }
                } 
                catch(Exception e)
                {
                    Log.e(LOG_TAG, "Could not get network and country codes. CDMA?", e);
                }

                cellUpdate(native_ptr_, data.cellId_, data.locationAreaCode_, data.mobileCountryCode_, data.mobileNetworkCode_, data.signalStrength_);
            }
            catch(Exception e)
            {
                Log.e(LOG_TAG, "Exception in onSignalStrengthsChanged: "+e);
            }
        }
    }


    public synchronized boolean start()
    {
        Log.d(LOG_TAG, "CellListener start");
        try
        {
            if (mListenerThread != null)
            {
                return false;
            }

            if (mTelMan == null)
                mTelMan = (TelephonyManager)getContext().getSystemService(Context.TELEPHONY_SERVICE);

            if (mTelMan == null)
                return false;

            mListenerThread = new Thread() 
            {
                @Override
                public void run()
                {
                    Looper.prepare();
                    mListenerLooper = Looper.myLooper();
                    try
                    {
                        mListener = new TrueCellListener();
                        if (mListener != null)
                            mTelMan.listen(mListener, PhoneStateListener.LISTEN_CELL_LOCATION | PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
                    }
                    catch (Exception e) {
                        Log.e(LOG_TAG, "Create TrueCellListener exception: ", e);
                        mListener = null;
                    }
                    catch (NoSuchMethodError e)
                    {
                        // Похоже, на каких-то вёдрах отпилено лишнего.
                        Log.e(LOG_TAG, "Create TrueCellListener NoSuchMethodError exception: ", e);
                        mListener = null;
                    }
                    if (mListener != null)
                    {
                        Log.d(LOG_TAG, "cell listener thread will loop");
                        Looper.loop();
                        Log.d(LOG_TAG, "cell listener thread did end loop");
                        mTelMan.listen(mListener, PhoneStateListener.LISTEN_NONE);
                        mListener = null;
                    }
                }
            }; // new Thread

            mListenerThread.start();
            return true;
        }
        catch (Exception e)
        {
            Log.e(LOG_TAG, "Exception while starting cell listener: "+e);
            return false;
        }
        catch (NoSuchMethodError e)
        {
            Log.e(LOG_TAG, "NoSuchMethodError exception while starting cell listener: "+e);
            return false;
        }
    }


    public synchronized void stop()
    {
        Log.d(LOG_TAG, "CellListener stop");
        try
        {
            if (mListenerThread != null)
            {
                mListenerLooper.quit();
                while (mListener != null) {
                    try{
                        mListenerThread.join();
                    } catch (Exception e) {
                        Log.e(LOG_TAG, "Exception while waiting for cell listener thread to stop: ", e);
                    }
                }
            }
        }
        catch (Exception e)
        {
            Log.e(LOG_TAG, "Exception while stopping cell listener: "+e);
        }
        finally
        {
            mListenerThread = null;
            mListenerLooper = null;
            mListener = null;
        }
    }


    private native void cellUpdate(long native_ptr, int cid, int lac, int mcc, int mnc, int rssi);
    private native Context getContext();

} // class CellListener

