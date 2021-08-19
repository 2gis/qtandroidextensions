/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2015-2016, DoubleGIS, LLC.
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

import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import android.os.LocaleList;
import android.os.Parcelable;
import java.io.File;
import java.util.ArrayList;
import java.util.Set;
import java.util.TreeSet;
import java.util.TimeZone;
import java.time.ZoneId;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.provider.Settings.Secure;
import androidx.annotation.NonNull;
import androidx.core.content.FileProvider;
import android.media.RingtoneManager;
import android.media.Ringtone;


public class DesktopUtils
{
    private static final String TAG = "Grym/DesktopServices";
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
        catch (final Throwable e)
        {
            Log.e(TAG, "IsInternetActive exception: ", e);
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
        catch (final Throwable e)
        {
            Log.e(TAG, "GetNetworkType exception: ", e);
        }
        return -1;
    }

    // Open application settings
    public static void showApplicationSettings(final Context ctx) {
        try {
            final Intent intent = new Intent();

            intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            intent.setData(Uri.fromParts("package", ctx.getPackageName(), null));
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
            intent.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);

            ctx.startActivity(intent);
        } catch (final Throwable throwable) {
            Log.e(TAG, "showApplicationSettings throwable: " + throwable);
        }
    }

    // Show generic "send to" menu
    public static boolean sendTo(
        final Context ctx,
        final String chooserCaption,
        final String text,
        final String contentType)
    {
        try
        {
            Log.d(TAG, "sendTo " + contentType);
            final Intent sendIntent = new Intent(Intent.ACTION_SEND);
            sendIntent.setType(contentType);
            sendIntent.putExtra(Intent.EXTRA_TEXT, text);
            final Intent chooser = Intent.createChooser(sendIntent, chooserCaption);
            chooser.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(chooser);
            return true;
        } catch (final Throwable e) {
            Log.e(TAG, "sendTo exception: ", e);
            return false;
        }
    }


    private static boolean isFiltered(final String value, final String[] filters) {
        if (filters == null) {
            return false;
        }
        for (String filter : filters) {
            if (filter == null || filter.isEmpty()) {
                continue;
            }
            if (filter.endsWith("*")) {
                if (value.startsWith(filter.substring(0, filter.length() - 1))) {
                    return true;
                }
            } else if (value.equals(filter)) {
                return true;
            }
        }
        return  false;
    }

    private static List<ComponentName> filterActivities(
            final Context ctx, Intent intent, final String[] filterPackages) {
        final List<ComponentName> result = new ArrayList<ComponentName>();
        final List<ResolveInfo> resolveInfos = ctx.getPackageManager().queryIntentActivities(intent, 0);
        for (ResolveInfo resolveInfo : resolveInfos) {
            final String pn = resolveInfo.activityInfo.packageName;
            final String name = resolveInfo.activityInfo.name;
            if (isFiltered(pn, filterPackages)) {
                Log.d(TAG, "filterActivities: skipping package: " + pn + ": " + name);
                result.add(new ComponentName(pn, name));
            }
        }
        return  result;
    }

    // Requires Android API 24+
    private static void sendToByExclude(
            final Context ctx,
            final String chooserCaption,
            final String text,
            final String contentType,
            final String[] filterPackages)
    {
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.setType(contentType);
        intent.putExtra(Intent.EXTRA_TEXT, text);

        Intent chooserIntent = Intent.createChooser(intent, chooserCaption);
        List<ComponentName> filtered = filterActivities(ctx, intent, filterPackages);
        chooserIntent.putExtra(Intent.EXTRA_EXCLUDE_COMPONENTS, filtered.toArray(new Parcelable[]{}));
        chooserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        ctx.startActivity(chooserIntent);
    }

    private static void sendToByInclude(
            final Context ctx,
            final String chooserCaption,
            final String text,
            final String contentType,
            final String[] filterPackages)
    {
        // Create intent with the text
        final Intent i = new Intent(Intent.ACTION_SEND);
        i.setType(contentType);
        i.putExtra(Intent.EXTRA_TEXT, text);
        // Find what activities can handle the text intent
        final List<ResolveInfo> resolveInfos = ctx.getPackageManager().queryIntentActivities(i, 0);
        // Filter the activities
        Intent chooserIntent = null;
        final List<Intent> intentList = new ArrayList<Intent>();
        for (ResolveInfo resolveInfo : resolveInfos) {
            final String pn = resolveInfo.activityInfo.packageName;
            final String name = resolveInfo.activityInfo.name;
            if (isFiltered(pn, filterPackages)) {
                Log.d(TAG, "sendTo: skipping package: " + pn + ": " + name);
                continue;
            }

            final Intent targetedIntent = (Intent)i.clone();
            targetedIntent.setComponent(new ComponentName(pn, name));
            if (chooserIntent == null) {
                chooserIntent = targetedIntent;
            } else {
                intentList.add(targetedIntent);
            }
        }
        if (chooserIntent == null) {
            // No activities found, fall back to simple sharing
            chooserIntent = Intent.createChooser(i, chooserCaption);
            chooserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        } else if (!intentList.isEmpty()) {
            // Have more than one activity - build chooser with the list
            final Intent keepIntent = chooserIntent; // should be moved to extra
            chooserIntent = Intent.createChooser(keepIntent, chooserCaption);
            chooserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            final Intent[] extraIntents = intentList.toArray(new Intent[intentList.size()]);
            chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, extraIntents);
        }
        ctx.startActivity(chooserIntent);
    }

    // Show generic "send to" menu for a plain text and optionally filter out some applications.
    // filterPackages is a "\n" - separated list of package names of prefixes (ended by "*").
    public static boolean sendTo(
        final Context ctx,
        final String chooserCaption,
        final String text,
        final String contentType,
        final String filterPackages) {
        try {
            Log.d(TAG, "sendTo: sending " + contentType + " with package filtering.");
            final String[] packages = (filterPackages == null) ? null : filterPackages.split("\n");

            // Intent.EXTRA_INITIAL_INTENTS does not work correctly on Android 10 Beta - 
            // https://issuetracker.google.com/issues/136027280
            if (Build.VERSION.SDK_INT >= 29) {
                sendToByExclude(ctx, chooserCaption, text, contentType, packages);
            }
            else {
                sendToByInclude(ctx, chooserCaption, text, contentType, packages);
            }
            return true;
        } catch (final Throwable e) {
            Log.e(TAG, "sendTo (2) exception: ", e);
            return false;
        }
    }


    public static boolean sendSMS(final Context ctx, final String number, final String text)
    {
        try
        {
            //Log.d(TAG, "Will send sms \""+text+"\" to  \""+number+"\"");
            String uris = "smsto:";
            if (number != null && number.length()>2)
                uris += number;
            if (verbose)
                Log.i(TAG, "URI: "+uris);
            Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse(uris));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra("sms_body", text);
            ctx.startActivity(intent);
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "sendSMS exception: ", e);
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
        //Log.d(TAG, "Will send email with subject \"" +
        //    subject + "\" to \"" + to + "\" with attach_file = \"" + attach_file + "\"" +
        //    ", force_content_provider = " + force_content_provider +
        //    ", authorities = \"" + authorities + "\"");
        try
        {
            // TODO: support multiple recipients
            String[] recipients = new String[]{to};

            final Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("mailto", to, null));
            List<ResolveInfo> resolveInfos = ctx.getPackageManager().queryIntentActivities(intent, 0);
            Intent chooserIntent = null;
            List<Intent> intentList = new ArrayList<Intent>();

            Intent i = new Intent(Intent.ACTION_SEND);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setType("message/rfc822");
            i.putExtra(Intent.EXTRA_EMAIL, recipients);
            i.putExtra(Intent.EXTRA_SUBJECT, subject);
            i.putExtra(Intent.EXTRA_TEXT, body);

            Uri workaround_grant_permission_for_uri = null;
            if (attach_file != null && !attach_file.isEmpty()) {
                if (!force_content_provider && android.os.Build.VERSION.SDK_INT < 23) {
                    i.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(new File(attach_file)));
                } else {
                    // Android 6+: going the longer route.
                    // For more information, please see:
                    // http://stackoverflow.com/questions/32981194/android-6-cannot-share-files-anymore
                    i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    workaround_grant_permission_for_uri = FileProvider.getUriForFile(
                        ctx,
                        authorities,
                        new File(attach_file));
                    i.putExtra(Intent.EXTRA_STREAM, workaround_grant_permission_for_uri);
                }
            }

            for (ResolveInfo resolveInfo : resolveInfos) {
                String packageName = resolveInfo.activityInfo.packageName;
                String name = resolveInfo.activityInfo.name;

                // Some mail clients will not read the URI unless this is done.
                // See here: https://stackoverflow.com/questions/24467696/android-file-provider-permission-denial
                if (workaround_grant_permission_for_uri != null) {
                    try {
                        ctx.grantUriPermission(
                            packageName
                            , workaround_grant_permission_for_uri
                            , Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    } catch (final Throwable e) {
                        Log.e(TAG, "grantUriPermission error: ", e);
                    }
                }

                Intent fakeIntent = (Intent)i.clone();
                fakeIntent.setComponent(new ComponentName(packageName, name));
                if (chooserIntent == null) {
                    chooserIntent = fakeIntent;
                } else {
                    intentList.add(fakeIntent);
                }
            }

            if (chooserIntent == null) {
                chooserIntent = Intent.createChooser(
                     i,
                     null // "Select email application."
                );
                chooserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            } else if (!intentList.isEmpty()) {
                Intent fakeIntent = chooserIntent;
                chooserIntent = new Intent(Intent.ACTION_CHOOSER);
                chooserIntent.putExtra(Intent.EXTRA_INTENT, fakeIntent);
                Intent[] extraIntents = intentList.toArray(new Intent[intentList.size()]);
                chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, extraIntents);
            }

            ctx.startActivity(chooserIntent);
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "sendEmail exception: ", e);
            return false;
        }
    }


    public static boolean sendEmail(
            final Context ctx
            , final String to
            , final String subject
            , final String body
            , final String[] attachment
            , final String authorities)
    {
        try
        {
            final String[] recipients = new String[]{ to };

            final Intent intent = new Intent(attachment.length > 1 ? Intent.ACTION_SEND_MULTIPLE : Intent.ACTION_SENDTO);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setType("message/rfc822");
            intent.putExtra(Intent.EXTRA_EMAIL, recipients);
            intent.putExtra(Intent.EXTRA_SUBJECT, subject);
            intent.putExtra(Intent.EXTRA_TEXT, body);

            boolean grant_permissions_workaround = false;
            final ArrayList<Uri> uri = new ArrayList<>();
            if (attachment.length > 0) {
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                    for (final String fileName: attachment) {
                        uri.add(Uri.fromFile(new File(fileName)));
                    }
                } else {
                    // Android 6+: going the longer route.
                    // For more information, please see:
                    // http://stackoverflow.com/questions/32981194/android-6-cannot-share-files-anymore
                    intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    grant_permissions_workaround = true;
                    for (final String fileName: attachment) {
                        uri.add(FileProvider.getUriForFile(ctx, authorities, new File(fileName)));
                    }
                }
                // Should not put array with only one element into intent because of a bug in GMail.
                if (uri.size() == 1) {
                    intent.putExtra(Intent.EXTRA_STREAM, uri.get(0));
                } else {
                    intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, uri);
                }
            }

            final IntentResolverInfo mailtoIntentResolvers = new IntentResolverInfo(ctx.getPackageManager());
            mailtoIntentResolvers.appendResolvers(new Intent(Intent.ACTION_SENDTO, Uri.fromParts("mailto", to, null)));

            final Intent chooserIntent;

            if (mailtoIntentResolvers.isEmpty()) {
                chooserIntent = Intent.createChooser(intent, null);
            } else {
                final IntentResolverInfo messageIntentResolvers = new IntentResolverInfo(ctx.getPackageManager());
                messageIntentResolvers.appendResolvers(new Intent(Intent.ACTION_SENDTO, Uri.fromParts("sms", "", null)));
                messageIntentResolvers.appendResolvers(new Intent(Intent.ACTION_SENDTO, Uri.fromParts("mms", "", null)));
                messageIntentResolvers.appendResolvers(new Intent(Intent.ACTION_SENDTO, Uri.fromParts("tel", "", null)));

                mailtoIntentResolvers.removeSamePackages(messageIntentResolvers.getResolveInfos());

                final List<Intent> intentList = new ArrayList<>();

                for (final ActivityInfo activityInfo : mailtoIntentResolvers.getResolveInfos()) {
                    final String packageName = activityInfo.getPackageName();
                    final String name = activityInfo.getName();

                    // Some mail clients will not read the URI unless this is done.
                    // See here: https://stackoverflow.com/questions/24467696/android-file-provider-permission-denial
                    if (grant_permissions_workaround) {
                        for (int i = 0; i < uri.size(); ++i) {
                            try {
                                ctx.grantUriPermission(
                                   packageName
                                   , uri.get(i)
                                   , Intent.FLAG_GRANT_READ_URI_PERMISSION);
                            } catch (final Throwable e) {
                                Log.e(TAG, "grantUriPermission error: ", e);
                            }
                        }
                    }

                    final Intent cloneIntent = (Intent) intent.clone();
                    cloneIntent.setComponent(new ComponentName(packageName, name));
                    intentList.add(cloneIntent);
                }

                final Intent targetIntent = intentList.get(0);
                intentList.remove(0);

                chooserIntent = Intent.createChooser(targetIntent, null);
                if (!intentList.isEmpty()) {
                    final Intent[] extraIntents = intentList.toArray(new Intent[intentList.size()]);
                    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                        chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, extraIntents);
                    } else {
                        chooserIntent.putExtra(Intent.EXTRA_ALTERNATE_INTENTS, extraIntents);
                    }
                }
            }

            chooserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

            ctx.startActivity(chooserIntent);

            return true;
        }
        catch (final Throwable exception)
        {
            Log.e(TAG, "sendEmail exception: ", exception);
            return false;
        }
    }

    public static boolean openURL(final Context ctx, final String url)
    {
        try
        {
            Log.d(TAG, "Will open URL: "+url);
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
        catch (final Throwable e)
        {
            Log.e(TAG, "openURL exception: ", e);
            return false;
        }
    }

    // Open a file using some (or default) associated application
    public static boolean openFile(final Context ctx, final String fileName, final String mimeType)
    {
        Log.d(TAG, "Will open file: "+fileName);
        try
        {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            File file = new File(fileName);
            intent.setDataAndType(Uri.fromFile(file), mimeType);
            ctx.startActivity(intent);
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "Exception while opening a file: ", e);
            return false;
        }
    }

    // Request system to install a local apk file. System installer application will be open.
    public static boolean installApk(final Context ctx, final String apk)
    {
        Log.d(TAG, "Will install APK: "+apk);
        try
        {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setDataAndType(Uri.fromFile(new File(apk)), "application/vnd.android.package-archive");
            ctx.startActivity(intent);
            Log.i(TAG, "Installation intent started successfully.");
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "Exception while installing apk: ", e);
            return false;
        }
    }

    // Request system to uninstall a package. User will be prompted for confirmation by the system package manager application.
    public static void uninstallApk(final Context ctx, final String packagename)
    {
        Log.i(TAG, "Will uninstall package: "+packagename);
        try
        {
            Intent intent = new Intent(Intent.ACTION_DELETE);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setData(Uri.parse("package:"+packagename));
            ctx.startActivity(intent);
            Log.i(TAG, "Uninstallation intent started successfully.");
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "Exception while uninstalling package: ", e);
        }
    }


    // See: https://stackoverflow.com/questions/5196833/android-device-phone-call-ability
    public static boolean isVoiceTelephonyAvailable(final Context ctx)
    {
        try
        {
            if (!ctx.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TELEPHONY))
            {
                return false;
            }
            final TelephonyManager manager = (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
            if (manager == null)
            {
                return false;
            }
            if (manager.getPhoneType() == TelephonyManager.PHONE_TYPE_NONE)
            {
                return false;
            }
            if (Build.VERSION.SDK_INT >= 22) // Android 5.1+
            {
                if (!manager.isVoiceCapable())
                {
                    return false;
                }
            }
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "isVoiceTelephonyAvailable exception (will return 'false'): ", e);
            return false;
        }
    }


    // 'number' is an RFC-3966 URI without 'tel:'.
    // If 'action' is null or empty string it defaults to Intent.ACTION_VIEW.
    // See: http://tools.ietf.org/html/rfc3966
    public static boolean callNumber(final Context ctx, final String number, final String action)
    {
        try
        {
            Log.i(TAG, "Will call number: " + number);
            final String doAction = (action == null || action.isEmpty()) ? Intent.ACTION_VIEW : action;
            final String doPhone = (number.startsWith("tel:")) ? number: "tel:" + number;
            final Intent i = new Intent(doAction);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setData(Uri.parse(doPhone));
            ctx.startActivity(i);
            return true;
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "callNumber exception: ", e);
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
        catch (final Throwable e)
        {
            Log.e(TAG, "getTelephonyDeviceId exception: ", e);
            return "";
        }
    }

    public static String getDisplayCountry(final Context ctx)
    {
        try
        {
            return getDefaultLocale(ctx).getDisplayCountry();
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "getDisplayCountry exception: " + e);
            return "";
        }
    }

    public static String getCountry(final Context ctx)
    {
        try
        {
            return getDefaultLocale(ctx).getCountry();
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "getCountry exception: " + e);
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
        catch (final Throwable e)
        {
            Log.e(TAG, "getAndroidId exception: ", e);
        }
        return "";
    }

    // Warning: works on API >= 9 only!
    public static String getBuildSerial()
    {
       try
       {
           if (Build.VERSION.SDK_INT >= 9)
           {
               return Build.SERIAL;
           }
       }
       catch (final Throwable e)
       {
           Log.e(TAG, "getBuildSerial exception: ", e);
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
        catch (final Throwable e)
        {
            Log.e(TAG, "getInstalledAppsList exception: ", e);
        }
        return "";
    }


    // Get user's default locale.
    private static Locale getDefaultLocale(final Context context) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) { // API 24 / Android 7.0
                return context.getResources().getConfiguration().getLocales().getDefault().get(0);
            } else {
                return context.getResources().getConfiguration().locale;
            }
        } catch (final Throwable e) {
            Log.e(TAG, "getDefaultLocale exception: ", e);
            return Locale.getDefault();
        }
    }


    public static String getDefaultLocaleName(final Context context)
    {
        try {
            return getDefaultLocale(context).toString();
        } catch (final Throwable e) {
            Log.e(TAG, "getDefaultLocaleName exception: ", e);
            return "";
        }
    }


    public static String getUserLocaleNames(final Context context)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) { // API 24 / Android 7.0
            try {
                final LocaleList list = context.getResources().getConfiguration().getLocales().getDefault();
                if (list.size() > 0) {
                    String result = "";
                    for (int i = 0; i < list.size(); ++i)
                        result += list.get(i).toString() + "\n";
                    return result;
                }
            } catch (final Throwable e) {
                Log.e(TAG, "getUserLocaleNames exception: ", e);
            }
        }
        return getDefaultLocaleName(context);
    }


    public static String getTimezoneId()
    {
        try {
            final TimeZone timezone = TimeZone.getDefault();
            final ZoneId zoneid = timezone.toZoneId();
            return zoneid.toString();
        } catch (final Throwable e) {
            Log.e(TAG, "getTimezoneId exception: ", e);
        }
        return "";
    }


    // https://developer.android.com/guide/topics/connectivity/bluetooth-le.html
    public static boolean isBluetoothLEAvailable(final Context ctx)
    {
        try
        {
            if (ctx.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE))
            {
                return true;
            }
        } catch (final Throwable e) {
            Log.e(TAG, "isBluetoothLEAvailable exception (will return 'false'): ", e);
        }

        return false;
    }


    public static void playNotificationSound(final Context context)
    {
        try {
            Uri notification = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
            Ringtone r = RingtoneManager.getRingtone(context, notification);
            r.play();
        } catch (final Throwable e) {
            Log.e(TAG, "playNotificationSound exception: ", e);
        }
    }

    // Compress files or directories into a zip archive
    public static boolean zipFiles(final String[] srcPaths, final String dstPath) {
        try{
           Zip.compressFiles(srcPaths, dstPath);
           return true;
        } catch (final Throwable e) {
            Log.e(TAG, "zipFiles exception: ", e);
        }
        return false;
    }

    public static String getBackgroundLocationPermissionLabel(final Context context) {
        if(Build.VERSION.SDK_INT >= 30) { // 11+ Android
            try {
                return String.valueOf(context.getPackageManager().getBackgroundPermissionOptionLabel());
            } catch (final Throwable e) {
                Log.e(TAG, "Can't get PackageManager", e);
                return new String();
            }
        } else {
            return new String();
        }
    }


    private static class ActivityInfo implements Comparable<ActivityInfo> {
        private @NonNull String mPackageName = "";
        private @NonNull String mName = "";

        ActivityInfo(final @NonNull String name, final @NonNull String packageName) {
            mName = name;
            mPackageName = packageName;
        }

        public String getPackageName() {
            return mPackageName;
        }

        public String getName() {
            return mName;
        }

        @Override
        public int compareTo(@NonNull ActivityInfo another) {
            final int packageNameCompare = mPackageName.compareTo(another.mPackageName);

            if (0 == packageNameCompare) {
                return mName.compareTo(another.mName);
            }

            return packageNameCompare;
        }
    }


    private static class IntentResolverInfo {
        private final PackageManager mPackageManager;
        final Set<ActivityInfo> mResolveInfoList = new TreeSet<>();

        IntentResolverInfo(final PackageManager packageManager) {
            mPackageManager = packageManager;
        }

        public boolean isEmpty() {
            return getResolveInfos().isEmpty();
        }

        Set<ActivityInfo> getResolveInfos() {
            return mResolveInfoList;
        }

        void appendResolvers(final Intent intent) {
            final List<ResolveInfo> resolveInfoList = mPackageManager.queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);

            for (final ResolveInfo resolveInfo : resolveInfoList) {
                mResolveInfoList.add(new ActivityInfo(resolveInfo.activityInfo.name, resolveInfo.activityInfo.packageName));
            }
        }

        void removeSamePackages(final Set<ActivityInfo> resolveInfoSet) {
            for (final ActivityInfo resolveInfo : resolveInfoSet) {
                if (mResolveInfoList.isEmpty()) {
                    break;
                }

                for (final Iterator<ActivityInfo> iterator = mResolveInfoList.iterator(); iterator.hasNext();) {
                    final ActivityInfo storedResolveInfo = iterator.next();

                    if (storedResolveInfo.getPackageName().equalsIgnoreCase(resolveInfo.getPackageName())) {
                        iterator.remove();
                    }
                }
            }
        }

        @Override
        public String toString() {
            return super.toString() + "; " + mResolveInfoList.toString();
        }
    }
}
