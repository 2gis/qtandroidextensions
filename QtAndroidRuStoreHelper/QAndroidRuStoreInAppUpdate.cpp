#include "QAndroidRuStoreInAppUpdate.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_RuStoreInAppUpdate_nativeSetUpdateAvailability(JNIEnv *, jobject, jlong native_ptr, jint updateAvailability, jlong versionCode)
{
	JNI_LINKER_OBJECT(QAndroidRuStoreInAppUpdate, native_ptr, proxy)
	proxy->setUpdateAvailability(updateAvailability, versionCode);
}


Q_DECL_EXPORT void JNICALL Java_RuStoreInAppUpdate_nativeSetInstallStatus(JNIEnv *, jobject, jlong native_ptr, jint installStatus)
{
	JNI_LINKER_OBJECT(QAndroidRuStoreInAppUpdate, native_ptr, proxy)
	proxy->setInstallStatus(installStatus);
}


Q_DECL_EXPORT void JNICALL Java_RuStoreInAppUpdate_nativeSetDownloadProgress(JNIEnv *, jobject, jlong native_ptr, jlong bytesDownloaded, jlong totalBytesToDownload)
{
	JNI_LINKER_OBJECT(QAndroidRuStoreInAppUpdate, native_ptr, proxy)
	proxy->setDownloadProgress(bytesDownloaded, totalBytesToDownload);
}


static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"nativeSetUpdateAvailability", "(JIJ)V", reinterpret_cast<void*>(Java_RuStoreInAppUpdate_nativeSetUpdateAvailability)},
	{"nativeSetInstallStatus", "(JI)V", reinterpret_cast<void*>(Java_RuStoreInAppUpdate_nativeSetInstallStatus)},
	{"nativeSetDownloadProgress", "(JJJ)V", reinterpret_cast<void*>(Java_RuStoreInAppUpdate_nativeSetDownloadProgress)},
};


JNI_LINKER_IMPL(QAndroidRuStoreInAppUpdate, "ru/dublgis/rustorehelper/RuStoreInAppUpdate", methods)


QAndroidRuStoreInAppUpdate::QAndroidRuStoreInAppUpdate(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
	, updateAvailability_(UpdateAvailability::UNKNOWN)
	, installStatus_(InstallStatus::UNKNOWN)
	, versionCode_(0)
	, bytesDownloaded_(0)
	, totalBytesToDownload_(0)
{
}


void QAndroidRuStoreInAppUpdate::checkForUpdate()
{
	try
	{
		if (isJniReady())
		{
			jni()->callVoid("checkForUpdate");
		}
	}
	catch(std::exception & e)
	{
		qCritical() << "Unhandled exception while checkForUpdate: " << e.what();
	}
}


void QAndroidRuStoreInAppUpdate::setUpdateAvailability(jint updateAvailability, jlong versionCode)
{
	auto updateAvailabilityNew = static_cast<UpdateAvailability>(updateAvailability);

	QWriteLocker locker(&mutex_);

	if (updateAvailability_ != updateAvailabilityNew)
	{
		if (UpdateAvailability::UPDATE_AVAILABLE == updateAvailability_)
		{
			versionCode_ = versionCode;
		}

		updateAvailability_ = updateAvailabilityNew;
		qInfo() << "New RuStore InAppUpdate update availability is " << updateAvailability;
		emit updateAvailabilityChanged(updateAvailability_);
	}
}


int64_t QAndroidRuStoreInAppUpdate::getAvailabileVersionCode()
{
	QReadLocker locker(&mutex_);
	return versionCode_;
}


void QAndroidRuStoreInAppUpdate::setInstallStatus(jint installStatus)
{
	QWriteLocker locker(&mutex_);

	auto installStatusNew = static_cast<InstallStatus>(installStatus);

	if (installStatus_ != installStatusNew)
	{
		installStatus_ = installStatusNew;
		qInfo() << "New RuStore InAppUpdate install status is " << installStatus;
		emit installStatusChanged(installStatus_);
	}
}


void QAndroidRuStoreInAppUpdate::setDownloadProgress(jint bytesDownloaded, jint totalBytesToDownload)
{
	QWriteLocker locker(&mutex_);
	bytesDownloaded_ = bytesDownloaded;
	totalBytesToDownload_ = totalBytesToDownload;
}


bool QAndroidRuStoreInAppUpdate::IsReadytoDownload()
{
	QReadLocker locker(&mutex_);

	switch(installStatus_)
	{
		case InstallStatus::DOWNLOADED:
		case InstallStatus::DOWNLOADING:
		case InstallStatus::INSTALLING:
		case InstallStatus::PENDING:
			return false;
		default:
			break;
	}

	return UpdateAvailability::UPDATE_AVAILABLE == updateAvailability_;
}


void QAndroidRuStoreInAppUpdate::startDownload()
{
	try
	{
		if (isJniReady())
		{
			jni()->callVoid("startDownload");
		}
	}
	catch(std::exception & e)
	{
		qCritical() << "Unhandled exception while startDownload: " << e.what();
	}
}


bool QAndroidRuStoreInAppUpdate::IsReadytoInstall()
{
	QReadLocker locker(&mutex_);

	return InstallStatus::DOWNLOADED == installStatus_ && UpdateAvailability::UPDATE_AVAILABLE == updateAvailability_;
}


void QAndroidRuStoreInAppUpdate::startInstall()
{
	try
	{
		if (isJniReady())
		{
			jni()->callVoid("startInstall");
		}
	}
	catch(std::exception & e)
	{
		qCritical() << "Unhandled exception while startInstall: " << e.what();
	}
}
