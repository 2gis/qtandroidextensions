#pragma once

#include <QtCore/QObject>

#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/IJniObjectLinker.h>

class QAndroidPowerManagerHelper : public QObject
{
	Q_OBJECT
        JNI_LINKER_DECL(QAndroidPowerManagerHelper)

public:
        QAndroidPowerManagerHelper(QObject * parent = 0);
        virtual ~QAndroidPowerManagerHelper();

	bool isInteractive();

signals:
	void interactiveChanged();

private:
	void onInteractiveChanged();
	friend void JNICALL Java_Provider_onInteractiveChanged(JNIEnv * env, jobject, jlong inst);
};

