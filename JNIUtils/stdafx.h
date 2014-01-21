// #define JNIUTILS_VERBOSE_LOG

#include <map>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <sys/types.h>

#if defined(JNIUTILS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif

#include <qconfig.h>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadStorage>
