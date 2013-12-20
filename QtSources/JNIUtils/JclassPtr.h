#pragma once
#include <QString>
#include <jni.h>

namespace jni_utils {

class JclassPtr
{
public:
	JclassPtr();
	explicit JclassPtr(const jclass & instance);
	virtual ~JclassPtr();

	jclass instance() const { return instance_; }
	operator jclass() const { return instance(); }

	JclassPtr& operator= (const jclass & instance);

private:
	void setInstance(const jclass & instance);
	void freeInstance();

	JNIEnv * env_;
	jclass instance_;

	Q_DISABLE_COPY(JclassPtr)
};

} // namespace jni_utils

