/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

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

import java.util.List;
import java.util.List;
import java.util.Iterator;
import java.io.File;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.Uri;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.os.Looper;
import android.os.Build;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.support.v4.content.FileProvider;
import android.telephony.TelephonyManager;
import android.telephony.CellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.provider.Settings.Secure;
import android.util.Log;
import android.app.Activity;
import android.view.Display;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

public class DesktopUtils
{
    private static final String tag = "Grym/DesktopServices";
    private static final boolean verbose = false;

    // Returns:
    // -1 - error
    // 1 - network connected
    // 0 - network not connected
    public static int isInternetActive(final Context ctx)
    {
        try
        {
            ConnectivityManager cmgr = (ConnectivityManager)ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cmgr == null )
                return -1;
            NetworkInfo netinfo = cmgr.getActiveNetworkInfo();
            if (netinfo != null && netinfo.isConnected())
                return 1;
            return 0;
        }
        catch (Exception e)
        {
            Log.e(tag, "IsInternetActive exception: "+e);
        }
        return -1;
    }

    // Returns:
    // a. -1 on error
    // b. See: http://developer.android.com/reference/android/net/ConnectivityManager.html#TYPE_BLUETOOTH
    // for possible returned values.
    public static int getNetworkType(final Context ctx)
    {
        try
        {
            ConnectivityManager cmgr = (ConnectivityManager)ctx.getSystemService(Context.CONNECTIVITY_SERVICE);
            if (cmgr == null )
                return -1;
            NetworkInfo netinfo = cmgr.getActiveNetworkInfo();
            if (netinfo == null)
                return -1;
            return netinfo.getType();
        }
        catch (Exception e)
        {
            Log.e(tag, "GetNetworkType exception: "+e);
        }
        return -1;
    }

    // Show generic "send to" menu
    public static boolean sendTo(final Context ctx, final String chooser_caption, final String text, final String content_type)
    {
        try
        {
            Log.d(tag, "Will send-to text \""+text+"\" of type: \""+content_type+"\"");
            Intent sendIntent = new Intent(Intent.ACTION_SEND);
            sendIntent.setType(content_type);
            sendIntent.putExtra(Intent.EXTRA_TEXT, text);
            Intent chooser = Intent.createChooser(sendIntent, chooser_caption);
            chooser.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(chooser);
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "sendTo exception: "+e);
            return false;
        }
    }

    public static boolean sendSMS(final Context ctx, final String number, final String text)
    {
        try
        {
            //Log.d(tag, "Will send sms \""+text+"\" to  \""+number+"\"");
            String uris = "smsto:";
            if (number != null && number.length()>2)
                uris += number;
            if (verbose)
                Log.i(tag, "URI: "+uris);
            Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse(uris));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra("sms_body", text);
            ctx.startActivity(intent);
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "sendSMS exception: "+e);
            return false;
        }
    }

    /*
        For this to work with attachments on Android 6 or with force_content_provider == true,
        please add this section into you AndroidManifest.xml:

        <provider
            android:name="android.support.v4.content.FileProvider"
            android:authorities="ru.dublgis.sharefileprovider"
            android:exported="false"
            android:grantUriPermissions="true">
            <meta-data
                android:name="android.support.FILE_PROVIDER_PATHS"
                android:resource="@xml/file_provider_paths" />
        </provider>

        Also, create res/xml/file_provider_paths.xml with the following content:

        <paths xmlns:android="http://schemas.android.com/apk/res/android">
            <external-path name="share" path="/" />
        </paths>
   */
    public static boolean sendEmail(
         final Context ctx,
         final String to,
         final String subject,
         final String body,
         final String attach_file,
         final boolean force_content_provider,
         final String authorities)
    {
        //Log.d(tag, "Will send email with subject \"" +
        //    subject + "\" to \"" + to + "\" with attach_file = \"" + attach_file + "\"" +
        //    ", force_content_provider = " + force_content_provider +
        //    ", authorities = \"" + authorities + "\"");
        try
        {
            // TODO: support multiple recipients
            String[] recipients = new String[]{to};

            Intent i = new Intent(Intent.ACTION_SEND);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setType("message/rfc822");
            i.putExtra(Intent.EXTRA_EMAIL, recipients);
            i.putExtra(Intent.EXTRA_SUBJECT, subject);
            i.putExtra(Intent.EXTRA_TEXT, body);
            if (attach_file != null && attach_file.length() > 0)
            {
                if (!force_content_provider && android.os.Build.VERSION.SDK_INT < 23)
                {
                    i.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(new File(attach_file)));
                }
                else
                {
                    // Android 6+: going the longer route.
                    // For more information, please see:
                    // http://stackoverflow.com/questions/32981194/android-6-cannot-share-files-anymore
                    i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    i.putExtra(Intent.EXTRA_STREAM, FileProvider.getUriForFile(
                         ctx,
                         authorities,
                         new File(attach_file)));
                }
            }
            Intent chooser = Intent.createChooser(
                i,
                null // "Select email application."
            );
            chooser.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(chooser);
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "sendEmail exception: "+e);
            return false;
        }
    }

    public static boolean openURL(final Context ctx, final String url)
    {
        try
        {
            Log.d(tag, "Will open URL: "+url);
            String openurl = url;

            Uri uri = Uri.parse(openurl);
            String scheme = uri.getScheme();
            if (scheme == null || (
                !scheme.equalsIgnoreCase("http") &&
                !scheme.equalsIgnoreCase("https") &&
                !scheme.equalsIgnoreCase("skype")))
            {
                openurl = "http://" + openurl;
            }

            Intent i = new Intent(Intent.ACTION_VIEW);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setData(Uri.parse(openurl));
            ctx.startActivity(i);
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "openURL exception: "+e);
            return false;
        }
    }

    // Open a file using some (or default) associated application
    public static boolean openFile(final Context ctx, final String fileName, final String mimeType)
    {
        Log.d(tag, "Will open file: "+fileName);
        try
        {
            Intent intent =new Intent(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            File file = new File(fileName);
            intent.setDataAndType(Uri.fromFile(file), mimeType);
            ctx.startActivity(intent);
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "Exception while opening a file: "+e);
            return false;
        }
    }

    // Request system to install a local apk file. System installer application will be open.
    public static boolean installApk(final Context ctx, final String apk)
    {
        Log.d(tag, "Will install APK: "+apk);
        try
        {
            Intent intent =new Intent(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setDataAndType(Uri.fromFile(new File(apk)), "application/vnd.android.package-archive");
            ctx.startActivity(intent);
            Log.i(tag, "Installation intent started successfully.");
            return true;
        }
        catch (Exception e)
        {
            Log.e(tag, "Exception while installing apk: "+e);
            return false;
        }
    }

    // Request system to uninstall a package. User will be prompted for confirmation by the system package manager application.
    public static void uninstallApk(final Context ctx, final String packagename)
    {
        Log.i(tag, "Will uninstall package: "+packagename);
        try
        {
            Intent intent = new Intent(Intent.ACTION_DELETE);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setData(Uri.parse("package:"+packagename));
            ctx.startActivity(intent);
            Log.i(tag, "Uninstallation intent started successfully.");
        }
        catch (Exception e)
        {
            Log.e(tag, "Exception while uninstalling package: "+e);
        }
    }

    // 'number' is an RFC-3966 URI without 'tel:'.
    // See: http://tools.ietf.org/html/rfc3966
    public static boolean callNumber(final Context ctx, final String number)
    {
        try
        {
            Log.i(tag, "Will call number: "+number);
            Intent i = new Intent(Intent.ACTION_VIEW);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setData(Uri.parse("tel:"+number));
            ctx.startActivity(i);
            return true;
        }
        catch(Exception e)
        {
            Log.e(tag, "callNumber exception: "+e);
            return false;
        }
    }

    public static String getTelephonyDeviceId(final Context ctx)
    {
        try
        {
            TelephonyManager tm = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
            if (tm == null)
            {
                return "";
            }
            return tm.getDeviceId();
        }
        catch (Exception e)
        {
            Log.e(tag, "getTelephonyDeviceId exception: "+e);
            return "";
        }
    }

    public static String getDisplayCountry(final Context ctx)
    {
        try
        {
            return ctx.getResources().getConfiguration().locale.getDisplayCountry();
        }
        catch (Exception e)
        {
            Log.e(tag, "getDisplayCountry exception: " + e);
            return "";
        }
    }

    public static String getCountry(final Context ctx)
    {
        try
        {
            return ctx.getResources().getConfiguration().locale.getCountry();
        }
        catch (Exception e)
        {
            Log.e(tag, "getCountry exception: " + e);
            return "";
        }
    }

    // Settings.Secure.ANDROID_ID
    public static String getAndroidId(final Context ctx)
    {
        try
        {
            String androidId = Secure.getString(ctx.getContentResolver(), Secure.ANDROID_ID);
            // Ignore buggy device (also Android simulator's) device id
            if (!"9774d56d682e549c".equals(androidId))
            {
                return androidId;
            }
        }
        catch (Exception e)
        {
            Log.e(tag, "getAndroidId exception: "+e);
        }
        return "";
    }

    // Warning: works on API >= 9 only!
    public static String getBuildSerial()
    {
       try
       {
           Build b = new Build();
           if (b != null)
           {
               return b.SERIAL;
           }
       }
       catch (Exception e)
       {
           Log.e(tag, "getBuildSerial exception: "+e);
       }
       return "";
    }

    public static String getInstalledAppsList(final Context ctx)
    {
        try
        {
            String result = new String();
            List<PackageInfo> packs = ctx.getPackageManager().getInstalledPackages(0);
            for (int i = 0; i < packs.size(); ++i)
            {
                result += packs.get(i).packageName;
                result += "\n";
            }
            return result; 
        }
        catch (Exception e)
        {
            Log.e(tag, "getInstalledAppsList exception: "+e);
        }
        return "";
    }
}
