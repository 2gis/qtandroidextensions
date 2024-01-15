package ru.dublgis.rustorehelper;

import android.app.Activity;
import android.content.Context;

import androidx.annotation.NonNull;

import java.util.concurrent.atomic.AtomicReference;

import ru.rustore.sdk.appupdate.listener.InstallStateUpdateListener;
import ru.rustore.sdk.appupdate.manager.RuStoreAppUpdateManager;
import ru.rustore.sdk.appupdate.manager.factory.RuStoreAppUpdateManagerFactory;
import ru.rustore.sdk.appupdate.model.AppUpdateInfo;
import ru.rustore.sdk.appupdate.model.AppUpdateOptions;
import ru.rustore.sdk.appupdate.model.AppUpdateType;
import ru.rustore.sdk.appupdate.model.InstallState;
import ru.rustore.sdk.appupdate.model.InstallStatus;

import ru.dublgis.androidhelpers.Log;



public class RuStoreInAppUpdate {
	private static final String TAG = "Grym/RuStoreInAppUpdate";
	private Long mNativePtr = 0L;
	RuStoreAppUpdateManager mRuStoreAppUpdateManager = null;
	InstallStateUpdateListener mInstallStateUpdateListener = null;
	AtomicReference<AppUpdateInfo> mAppUpdateInfo = null;


	public RuStoreInAppUpdate(final long native_ptr)
	{
		mNativePtr = native_ptr;

		try {
			mRuStoreAppUpdateManager = RuStoreAppUpdateManagerFactory.INSTANCE.create(getContext());
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to create RuStoreAppUpdateManager", ex);
		}
	}


	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed()
	{
		unregisterListener();

		synchronized(this) {
			mNativePtr = 0L;
		}
	}


	public void checkForUpdate()
	{
		mAppUpdateInfo = new AtomicReference<AppUpdateInfo>();

		try {
			mRuStoreAppUpdateManager
				.getAppUpdateInfo()
				.addOnSuccessListener(
					info -> {
						mAppUpdateInfo.set(info);

						Log.d(TAG, "availableVersionCode: " + info.getAvailableVersionCode());
						Log.d(TAG, "installStatus: " + info.getInstallStatus());
						Log.d(TAG, "packageName: " + info.getPackageName());
						Log.d(TAG, "updateAvailability: " + info.getUpdateAvailability());
						Log.d(TAG, "updatePriority: " + info.getUpdatePriority());
						Log.d(TAG, "updatedAt: " + info.getUpdatedAt());

						synchronized(this) {
							nativeSetUpdateAvailability(mNativePtr, info.getUpdateAvailability(), info.getAvailableVersionCode());
							nativeSetInstallStatus(mNativePtr, info.getInstallStatus());
						}
					})
				.addOnFailureListener(
					throwable -> {
						Log.e(TAG, "Failed to get update info", throwable);
					});
		} catch (NullPointerException ex) {
			Log.e(TAG, "Failed to getAppUpdateInfo with NullPointerException", ex);
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to getAppUpdateInfo with Throwable", ex);
		}
	}


	public void registerListener()
	{
		try {
			mInstallStateUpdateListener = new InstallStateUpdateListener() {
				@Override
				public void onStateUpdated(@NonNull InstallState installState) {
					nativeSetInstallStatus(mNativePtr, installState.getInstallStatus());
					nativeSetDownloadProgress(mNativePtr, installState.getBytesDownloaded(), installState.getTotalBytesToDownload());
				}
			};

			mRuStoreAppUpdateManager.registerListener(mInstallStateUpdateListener);
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to registerListener", ex);
		}
	}


	public void unregisterListener()
	{
		try {
			if (mInstallStateUpdateListener != null) {
				mRuStoreAppUpdateManager.unregisterListener(mInstallStateUpdateListener);
				mInstallStateUpdateListener = null;
			}
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to unregisterListener", ex);
		}
	}


	public void startDownload()
	{
		try {
			Log.i(TAG, "Start download");
			unregisterListener();

			if (mAppUpdateInfo.get() == null) {
				checkForUpdate();
				return;
			}

			registerListener();

			int type = AppUpdateType.FLEXIBLE;
			AppUpdateOptions appUpdateOptions = new AppUpdateOptions.Builder().appUpdateType(type).build();

			mRuStoreAppUpdateManager
					.startUpdateFlow(mAppUpdateInfo.get(), appUpdateOptions)
					.addOnSuccessListener(resultCode -> {
						Log.i(TAG, "Download success " + resultCode);

					})
					.addOnFailureListener(throwable -> {
						Log.e(TAG, "Download failure", throwable);
					});

			mAppUpdateInfo.set(null);
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to startDownload", ex);
		}
	}


	public void startInstall()
	{
		try {
			Log.i(TAG, "Start install");
			mRuStoreAppUpdateManager
					.completeUpdate()
					.addOnFailureListener(throwable -> {
						Log.e(TAG, "Failed to install", throwable);
					});
		} catch (Throwable ex) {
			Log.e(TAG, "Failed to startInstall", ex);
		}
	}


	public native Context getContext();
	public native void nativeSetUpdateAvailability(long nativeptr, int updateAvailability, long versionCode);
	public native void nativeSetInstallStatus(long nativeptr, int installStatus);
	public native void nativeSetDownloadProgress(long nativeptr, long bytesDownloaded, long totalBytesToDownload);
}
