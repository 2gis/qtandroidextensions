/*
    QJniHelpers library for Qt

    Author:
    Vyacheslav O. Koscheev <vok1980@gmail.com>

    Distrbuted under The BSD License

    Copyright (c) 2015, DoubleGIS, LLC.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the DoubleGIS, LLC nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once


#include "QJniHelpers.h"


class IJniObjectLinker
{
public:
	virtual ~IJniObjectLinker() {};
	virtual QJniObject * handler() const = 0;
};


template <typename TNative> class TJniObjectLinker;


#define JNI_LINKER_DECL(nativeClass)                                                                                                     \
public:                                                                                                                                  \
	static void preloadJavaClasses();                                                                                                    \
private:                                                                                                                                 \
	bool isJniReady() const;                                                                                                             \
	QJniObject * jni() const;                                                                                                            \
	static void getNativeMethods(QByteArray & javaFullClassName, const JNINativeMethod ** methods_list, size_t & sizeof_methods_list);   \
	friend class TJniObjectLinker<nativeClass>;                                                                                          \
	typedef TJniObjectLinker<nativeClass> JniObjectLinker;                                                                               \
	QScopedPointer<IJniObjectLinker> jniLinker_;                                                                                         \

