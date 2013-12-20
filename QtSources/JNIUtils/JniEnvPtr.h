#pragma once
#include <string>
#include <jni.h>
#ifdef QT_CORE_LIB
	#include <QString>
#endif

//! Basic functionality to get JNIEnv valid for current thread and scope.
class JniEnvPtr
{	
public:
	JniEnvPtr();
	JniEnvPtr(JNIEnv* env);
	~JniEnvPtr();
	JNIEnv* env();

	// preload class by names 
	// required as Android's JNIEnv->FindClass doesn't work in threads created in native code. 
	// (no workarounds with custom ClassLoader were found {yet?})
public:
	int PreloadClass(const char * class_name);
	// last array element == 0
	int PreloadClasses(const char * const * class_list);

	// Возвращает глобальную ссылку на класс
	jclass FindClass(const char * name);

	// unload all preloaded classes for them not to leak
	void UnloadClasses();

#ifdef QT_CORE_LIB
	// don't forget to DeleteLocalRef it
	jstring JStringFromQString(const QString&);
	QString QStringFromJString(jstring);
#endif
	
public:
	//static JavaVM* JavaVM();
	static void SetJavaVM(JavaVM*);
	static void SetJavaVM(JNIEnv*);
	//static std::string PackageName();

private:
	JNIEnv* env_;
	bool was_already_attached_;
};

