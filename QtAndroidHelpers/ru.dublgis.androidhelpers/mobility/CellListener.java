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

import android.content.Context;
import android.os.Build;
import android.os.Looper;
import android.support.v4.app.ActivityCompat;
import android.telephony.CellInfo;
import android.telephony.CellInfoCdma;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoWcdma;
import android.telephony.CellLocation;
import android.telephony.NeighboringCellInfo;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;

import java.util.List;

import ru.dublgis.androidhelpers.Log;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;


public class CellListener {
    private final static String TAG = "Grym/CellListener";
    private volatile long native_ptr_ = 0;
    private final static boolean verbose = false;

    private TelephonyManager mManager = null;
    private SignalStrength mSignalStrengthLast = null;
    private CellListenerImpl mListener = null;
    private Thread mListenerThread = null;
    private Looper mListenerLooper = null;


    public CellListener(long native_ptr) {
        native_ptr_ = native_ptr;
    }


    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed() {
        native_ptr_ = 0;
    }


    public synchronized boolean start() {
        Log.d(TAG, "start");
        try {
            if (mManager == null) {
                mManager = (TelephonyManager) getContext().getSystemService(Context.TELEPHONY_SERVICE);
            }

            Runnable listenRunnable = new Runnable() {
                @Override
                public void run() {
                    if (mManager != null) {
                        try {
                            Looper.prepare();
                            mListenerLooper = Looper.myLooper();
                            mListener = new CellListenerImpl();
                            mManager.listen(mListener, PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);
                            Looper.loop();
                            mManager.listen(mListener, PhoneStateListener.LISTEN_NONE);
                        }
                        catch (Throwable ex) {
                            Log.e(TAG, "Failed to start TelephonyManager listener", ex);
                        }
                    }
                }
            };

            mListenerThread = new Thread(listenRunnable, "Listen TelephonyManager");
            mListenerThread.start();
            return true;
        } catch (Throwable e) {
            Log.e(TAG, "Exception while starting cell listener: ", e);
            return false;
        }
    }


    public synchronized void stop() {
        Log.d(TAG, "stop");

        try {
            if (mManager != null) {
                mManager.listen(mListener, PhoneStateListener.LISTEN_NONE);
            }
        }
        catch (SecurityException ex) {
            Log.e(TAG, "Failed to stop listener", ex);
        }

        if (null != mListenerLooper) {
            mListenerLooper.quit();
        }

        try {
            if (mListenerThread.isAlive()) {
                mListenerThread.join(300);
            }
        } catch (InterruptedException e) {
            Log.e(TAG, "Exception in cppDestroyed: ", e);
        }
    }


    private void reportDataCellInfo(List<CellInfo> cellInfoList) {
        for (CellInfo cellInfo : cellInfoList) {
            if (Build.VERSION.SDK_INT >= 17) {
                 if (cellInfo instanceof CellInfoCdma) {
                     CellInfoCdma cellInfoCdma = (CellInfoCdma) cellInfo;
                     cellUpdate(
                             native_ptr_,
                             "cdma",
                             cellInfoCdma.getCellIdentity().getBasestationId(), // Base Station Id 0..65535, Integer.MAX_VALUE if unknown
                             cellInfoCdma.getCellIdentity().getNetworkId(), // Network Id 0..65535, Integer.MAX_VALUE if unknown
                             Integer.MAX_VALUE,
                             cellInfoCdma.getCellIdentity().getSystemId(), // System Id 0..32767, Integer.MAX_VALUE if unknown
                             cellInfoCdma.getCellSignalStrength().getCdmaDbm(), // Get the CDMA RSSI value in dBm
                             Integer.MAX_VALUE
                     );
                 }
                if (cellInfo instanceof CellInfoGsm) {
                    CellInfoGsm cellInfoGsm = (CellInfoGsm) cellInfo;
                    cellUpdate(
                            native_ptr_,
                            "gsm",
                            cellInfoGsm.getCellIdentity().getCid(), // CID Either 16-bit GSM Cell Identity described in TS 27.007, 0..65535, Integer.MAX_VALUE if unknown
                            cellInfoGsm.getCellIdentity().getLac(), // 16-bit Location Area Code, 0..65535, Integer.MAX_VALUE if unknown
                            cellInfoGsm.getCellIdentity().getMcc(), // 3-digit Mobile Country Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoGsm.getCellIdentity().getMnc(), // 2 or 3-digit Mobile Network Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoGsm.getCellSignalStrength().getDbm(), // Get the signal strength as dBm
                            android.os.Build.VERSION.SDK_INT >= 26 ? cellInfoGsm.getCellSignalStrength().getTimingAdvance() : Integer.MAX_VALUE // Get the GSM timing advance between 0..219 symbols (normally 0..63). Integer.MAX_VALUE is reported when there is no RR connection.
                    );
                }
                if (cellInfo instanceof CellInfoLte) {
                    CellInfoLte cellInfoLte = (CellInfoLte) cellInfo;
                    cellUpdate(
                            native_ptr_,
                            "lte",
                            cellInfoLte.getCellIdentity().getCi(), // 28-bit Cell Identity, Integer.MAX_VALUE if unknown
                            cellInfoLte.getCellIdentity().getTac(), // 16-bit Tracking Area Code, Integer.MAX_VALUE if unknown
                            cellInfoLte.getCellIdentity().getMcc(), // 3-digit Mobile Country Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoLte.getCellIdentity().getMnc(), // 2 or 3-digit Mobile Network Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoLte.getCellSignalStrength().getDbm(), // Get signal strength as dBm
                            cellInfoLte.getCellSignalStrength().getTimingAdvance() // Get the timing advance value for LTE, as a value between 0..63. Integer.MAX_VALUE is reported when there is no active RRC connection. Refer to 3GPP 36.213 Sec 4.2.3
                    );
                }
            }
            if (Build.VERSION.SDK_INT >= 18) {
                if (cellInfo instanceof CellInfoWcdma) {
                    CellInfoWcdma cellInfoWcdma = (CellInfoWcdma) cellInfo;
                    cellUpdate(
                            native_ptr_,
                            "wcdma",
                            cellInfoWcdma.getCellIdentity().getCid(), // CID 28-bit UMTS Cell Identity described in TS 25.331, 0..268435455, Integer.MAX_VALUE if unknown
                            cellInfoWcdma.getCellIdentity().getLac(), // 16-bit Location Area Code, 0..65535, Integer.MAX_VALUE if unknown
                            cellInfoWcdma.getCellIdentity().getMcc(), // 3-digit Mobile Country Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoWcdma.getCellIdentity().getMnc(), // 2 or 3-digit Mobile Network Code, 0..999, Integer.MAX_VALUE if unknown
                            cellInfoWcdma.getCellSignalStrength().getDbm(), // Get the signal strength as dBm
                            Integer.MAX_VALUE
                    );
                }
            }
        }
    }


    private void reportDataNeighboringCellInfo(List<NeighboringCellInfo> neighboringCellInfoList) {
        for (NeighboringCellInfo cellInfo : neighboringCellInfoList) {
            cellUpdate(
                    native_ptr_,
                    "unknown",
                    cellInfo.getCid (), // cell id in GSM, 0xffff max legal value UNKNOWN_CID if in UMTS or CDMA or unknown
                    cellInfo.getLac(), // LAC in GSM, 0xffff max legal value UNKNOWN_CID if in UMTS or CMDA or unknown
                    Integer.MAX_VALUE,
                    Integer.MAX_VALUE,
                    getGsmDbm(cellInfo.getRssi()), // received signal strength or UNKNOWN_RSSI if unknown For GSM, it is in "asu" ranging from 0 to 31 (dBm = -113 + 2*asu) 0 means "-113 dBm or less" and 31 means "-51 dBm or greater" For UMTS, it is the Level index of CPICH RSCP defined in TS 25.125
                    Integer.MAX_VALUE
            );
        }
    }


    private void reportData() {
        if (ActivityCompat.checkSelfPermission(getContext(), ACCESS_FINE_LOCATION) != PERMISSION_GRANTED &&
                ActivityCompat.checkSelfPermission(getContext(), ACCESS_COARSE_LOCATION) != PERMISSION_GRANTED) {
            return;
        }

        int dbm = 99;
        boolean gsm = false;

        synchronized(TAG) {
            gsm = mSignalStrengthLast.isGsm();
            dbm =
                    gsm
                            ? getGsmDbm(mSignalStrengthLast.getGsmSignalStrength())
                            : mSignalStrengthLast.getCdmaDbm();
        }

        CellLocation loc = mManager.getCellLocation();

        if (gsm && loc instanceof GsmCellLocation) {
            GsmCellLocation locGsm = (GsmCellLocation) loc;
            cellUpdate(
                    native_ptr_,
                    "gsm",
                    locGsm.getCid(), // gsm cell id, -1 if unknown, 0xffff max legal value
                    locGsm.getLac(), // gsm location area code, -1 if unknown, 0xffff max legal value
                    Integer.MAX_VALUE,
                    Integer.MAX_VALUE,
                    dbm,
                    Integer.MAX_VALUE);
        } else if (loc instanceof CdmaCellLocation) {
            CdmaCellLocation locCdma = (CdmaCellLocation) loc;
            cellUpdate(
                    native_ptr_,
                    "cdma",
                    locCdma.getBaseStationId(), // cdma base station identification number, -1 if unknown
                    locCdma.getNetworkId(),     // cdma network identification number, -1 if unknown
                    Integer.MAX_VALUE,
                    Integer.MAX_VALUE,
                    dbm,
                    Integer.MAX_VALUE);
        }
    }


    public synchronized void requestData() {
        List<CellInfo> cellInfoList = null;
        List<NeighboringCellInfo> neighboringCellInfoList = null;

        if (Build.VERSION.SDK_INT >= 17 &&
            ActivityCompat.checkSelfPermission(getContext(), ACCESS_COARSE_LOCATION) == PERMISSION_GRANTED) {
                cellInfoList = mManager.getAllCellInfo();
        }

        if (cellInfoList != null && cellInfoList.size() > 0) {
            reportDataCellInfo(cellInfoList);
            return;
        // } else if (ActivityCompat.checkSelfPermission(getContext(), ACCESS_COARSE_LOCATION) == PERMISSION_GRANTED) {
        //     neighboringCellInfoList = mManager.getNeighboringCellInfo();
        }

        if (neighboringCellInfoList != null && neighboringCellInfoList.size() > 0) {
            reportDataNeighboringCellInfo(neighboringCellInfoList);
        } else {
            // reportData();
        }
    }


    // Get the GSM Signal Strength, valid values are (0-31, 99) as defined in TS 27.007 8.5
    private int getGsmDbm(int strength) {
        // <rssi>:
        // 0 -113 dBm or less
        // 1 -111 dBm
        // 2...30 -109... -53 dBm
        // 31 -51 dBm or greater
        // 99 not known or not detectable

        if (0 <= strength && strength <= 31) {
            return (strength - 2) * (109 - 53) / (30 - 2) - 109;
        }
        else if (strength == 99)
        {
            return Integer.MAX_VALUE;
        }

        Log.e(TAG, "Enexpected signal strength " + strength);
        return Integer.MAX_VALUE;
    }


    private class CellListenerImpl extends PhoneStateListener {

        public void onSignalStrengthsChanged(SignalStrength signalStrength) {
            super.onSignalStrengthsChanged(signalStrength);

            if (verbose) {
                Log.d(TAG, "onSignalStrengthsChanged: " + signalStrength.toString());
            }

            synchronized(TAG) {
                mSignalStrengthLast = signalStrength;
            }

            onSignalChanged(native_ptr_);
        }
    }

    private native void onSignalChanged(long native_ptr);
    private native void cellUpdate(long native_ptr, String type, int cid, int lac, int mcc, int mnc, int rssi, int ta);
    private native Context getContext();
}
