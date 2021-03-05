#include "QAndroidPowerManagerHelper.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_Provider_onInteractiveChanged(JNIEnv * env, jobject, jlong inst)
{
    Q_UNUSED(env);

    JNI_LINKER_OBJECT(QAndroidPowerManagerHelper, inst, proxy)
    proxy->onInteractiveChanged();
}


static const JNINativeMethod methods[] = {
    {"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
    {"onInteractiveChanged", "(J)V", reinterpret_cast<void *>(Java_Provider_onInteractiveChanged)},
};

JNI_LINKER_IMPL(QAndroidPowerManagerHelper, "ru/dublgis/androidhelpers/PowerManagerHelper", methods)

QAndroidPowerManagerHelper::QAndroidPowerManagerHelper(QObject * parent)
    : QObject(parent)
    , jniLinker_(new JniObjectLinker(this))
{
}

QAndroidPowerManagerHelper::~QAndroidPowerManagerHelper()
{
}

bool QAndroidPowerManagerHelper::isInteractive()
{
    if (isJniReady())
    {
        try
        {
            return jni()->callBool("isInteractive");
        }
        catch(const std::exception & ex)
        {
            qCritical() << "JNI exception in QAndroidPowerManagerHelper: " << ex.what();
        }
    }
    return false;
}

void QAndroidPowerManagerHelper::onInteractiveChanged()
{
    emit interactiveChanged();
}
