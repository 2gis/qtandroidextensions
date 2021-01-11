#include "QAndroidScreenInteractiveObserver.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_Provider_onInteractiveChanged(JNIEnv * env, jobject, jlong inst)
{
        Q_UNUSED(env);

        JNI_LINKER_OBJECT(QAndroidScreenInteractiveObserver, inst, proxy)
        proxy->onInteractiveChanged();
}



static const JNINativeMethod methods[] = {
    {"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
    {"getActivity",
     "()Landroid/app/Activity;",
     reinterpret_cast<void *>(QAndroidQPAPluginGap::getActivityNoThrow)},
    {"onInteractiveChanged", "(J)V", reinterpret_cast<void *>(Java_Provider_onInteractiveChanged)},
};

JNI_LINKER_IMPL(QAndroidScreenInteractiveObserver, "ru/dublgis/androidhelpers/ScreenInteractiveObserver", methods)

QAndroidScreenInteractiveObserver::QAndroidScreenInteractiveObserver(QObject * parent)
    : QObject(parent),
      jniLinker_(new JniObjectLinker(this))
{
}

QAndroidScreenInteractiveObserver::~QAndroidScreenInteractiveObserver()
{
}

bool QAndroidScreenInteractiveObserver::isInteractive()
{
    if (isJniReady())
    {
        try
        {
            return jni()->callBool("isInteractive");
        }
        catch(const std::exception & ex)
        {
                qCritical() << "JNI exception in ScreenInteractiveObserver: " << ex.what();
        }
        catch(...)
        {
                qCritical() << "Unknown exception";
        }
    }
    return false;
}

void QAndroidScreenInteractiveObserver::onInteractiveChanged()
{
    emit interactiveChanged();
}
