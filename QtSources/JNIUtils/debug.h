#ifndef JNIUTILS_DEBUG_H_INCLUDED
#define JNIUTILS_DEBUG_H_INCLUDED

//#define DEBUG
//#define DEBUG_VERBOSE
//#define OS_ANDROID

#if defined(DEBUG)
#if defined(OS_ANDROID)
#include <android/log.h>
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG,"Grym/JNIUtils",__VA_ARGS__)
#if defined(DEBUG_VERBOSE)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,"Grym/JNIUtils",__VA_ARGS__)
#else // DEBUG_VERBOSE
#define LOGV(...)
#endif //DEBUG_VERBOSE
#else //OS_ANDROID
#define LOG(...)
#define LOGV(...)
#endif //OS_ANDROID
#else //DEBUG
#define LOG(...)
#define LOGV(...)
#endif //DEBUG

#endif //JNIUTILS_DEBUG_H_INCLUDED
