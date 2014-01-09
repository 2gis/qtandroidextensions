#include "stdafx.h"
#include <JNIUtils/JclassPtr.h>
#include <JNIUtils/JniEnvPtr.h>

namespace jni_utils {

JclassPtr::JclassPtr()
	: env_(0)
	, instance_(0)
{
}

JclassPtr::JclassPtr(const jclass & instance)
	: env_(0)
	, instance_(0)
{
	setInstance(instance);
}

JclassPtr::~JclassPtr()
{
	freeInstance();
}

JclassPtr& JclassPtr::operator= (const jclass & instance)
{
	freeInstance();
	setInstance(instance);

	return *this;
}

void JclassPtr::setInstance(const jclass & instance)
{
	if (instance)
	{
		Q_ASSERT(!env_ && !instance_);

		JniEnvPtr jep;
		env_ = jep.env();
		Q_ASSERT(env_);

		instance_ = static_cast<jclass>(env_->NewGlobalRef(instance));
		Q_ASSERT(instance_);
	}
}

void JclassPtr::freeInstance()
{
	if (env_ && instance_)
	{
		env_->DeleteGlobalRef(instance_);
		env_ = 0;
		instance_ = 0;
	}
}

} // namespace jni_utils

