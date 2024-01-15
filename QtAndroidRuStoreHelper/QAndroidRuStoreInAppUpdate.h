#pragma once

#include <QtCore/QObject>
#include <QtCore/QReadLocker>
#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/IJniObjectLinker.h>


// see https://www.rustore.ru/help/sdk/updates/
class QAndroidRuStoreInAppUpdate : public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidRuStoreInAppUpdate)

public:
	enum class UpdateAvailability
	{
		UNKNOWN = 0,
		UPDATE_NOT_AVAILABLE = 1,
		UPDATE_AVAILABLE = 2,
		DEVELOPER_TRIGGERED_UPDATE_IN_PROGRESS = 3
	};

	enum class InstallStatus
	{
		UNKNOWN = 0,
		DOWNLOADED = 1,
		DOWNLOADING = 2,
		FAILED = 3,
		INSTALLING = 4,
		PENDING = 5
	};

public:
	QAndroidRuStoreInAppUpdate(QObject * parent);
	void checkForUpdate();
	bool IsReadytoDownload();
	void startDownload();
	bool IsReadytoInstall();
	void startInstall();
	int64_t getAvailabileVersionCode();

signals:
	void updateAvailabilityChanged(UpdateAvailability);
	void installStatusChanged(InstallStatus);

private:
	friend void JNICALL Java_RuStoreInAppUpdate_nativeSetUpdateAvailability(JNIEnv *, jobject, jlong native_ptr, jint updateAvailability, jlong versionCode);
	friend void JNICALL Java_RuStoreInAppUpdate_nativeSetInstallStatus(JNIEnv *, jobject, jlong native_ptr, jint installStatus);
	friend void JNICALL Java_RuStoreInAppUpdate_nativeSetDownloadProgress(JNIEnv *, jobject, jlong native_ptr, jlong bytesDownloaded, jlong totalBytesToDownload);

private:
	void setUpdateAvailability(jint updateAvailability, jlong versionCode);
	void setInstallStatus(jint installStatus);
	void setDownloadProgress(jint bytesDownloaded, jint totalBytesToDownload);

private:
	UpdateAvailability updateAvailability_;
	InstallStatus installStatus_;
	int64_t versionCode_;
	int64_t bytesDownloaded_;
	int64_t totalBytesToDownload_;
	QReadWriteLock mutex_;
};
