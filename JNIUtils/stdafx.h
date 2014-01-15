// #define JNIUTILS_VERBOSE_LOG

#include <map>
#include <string>
#include <jni.h>

#if defined(JNIUTILS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif

#if defined(QT_CORE_LIB)
	#include <qconfig.h>
	#include <QString>
#endif
