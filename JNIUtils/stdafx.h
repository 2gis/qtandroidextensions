#include <map>
#include <string>
#include <jni.h>
#if defined(OS_ANDROID)
	#include <android/log.h>
#endif

// #define JNIUTILS_VERBOSE_LOG

#if defined(JNIUTILS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif
