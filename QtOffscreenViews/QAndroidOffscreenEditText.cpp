/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
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

#include <QAndroidQPAPluginGap.h>
#include "QAndroidOffscreenEditText.h"

QAndroidOffscreenEditText::QAndroidOffscreenEditText(const QString & object_name, const QSize & def_size, QObject * parent)
	: QAndroidOffscreenView(QLatin1String("OffscreenEditText"), object_name, true, def_size, parent)
{
	setAttachingMode(true);
}

QAndroidOffscreenEditText::~QAndroidOffscreenEditText()
{
}

void QAndroidOffscreenEditText::preloadJavaClasses()
{
	QAndroidOffscreenView::preloadJavaClasses();
	QAndroidQPAPluginGap::preloadJavaClass((getDefaultJavaClassPath() + QLatin1String("OffscreenEditText")).toLatin1());
}

void QAndroidOffscreenEditText::setText(const QString & text)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setText", text);
	}
}

QString QAndroidOffscreenEditText::getText() const
{
	if (QJniObject * view = const_cast<QJniObject*>(offscreenView()))
	{
		return view->callString("getText");
	}
	return QString::null;
}

void QAndroidOffscreenEditText::setTextSize(float size, int unit)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setTextSize", "FI", jfloat(size), jint(unit));
	}
}

void QAndroidOffscreenEditText::setTypeface(const QString & name, int style)
{
	if (QJniObject * view = offscreenView())
	{
		QJniLocalRef jstr(QJniEnvPtr().JStringFromQString(name));
		view->callParamVoid("setTypeface", "Ljava/lang/string;I", jstr.jObject(), jint(style));
	}
}

void QAndroidOffscreenEditText::setCursorVisible(bool visible)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setCursorVisible", jboolean(visible));
	}
}

void QAndroidOffscreenEditText::setInputType(int type)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setInputType", jint(type));
	}
}

void QAndroidOffscreenEditText::setMarqueeRepeatLimit(int marqueeLimit)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMarqueeRepeatLimit", jint(marqueeLimit));
	}
}

void QAndroidOffscreenEditText::setMaxEms(int maxems)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMaxEms", jint(maxems));
	}
}

void QAndroidOffscreenEditText::setMinEms(int minems)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMinEms", jint(minems));
	}
}

void QAndroidOffscreenEditText::setMaxHeight(int maxHeight)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMaxHeight", jint(maxHeight));
	}
}

void QAndroidOffscreenEditText::setMinHeight(int minHeight)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMinHeight", jint(minHeight));
	}
}

void QAndroidOffscreenEditText::setMaxLines(int maxlines)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMaxLines", jint(maxlines));
	}
}

void QAndroidOffscreenEditText::setMinLines(int minlines)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMinLines", jint(minlines));
	}
}

void QAndroidOffscreenEditText::setMaxWidth(int maxpixels)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMaxWidth", jint(maxpixels));
	}
}

void QAndroidOffscreenEditText::setMinWidth(int minpixels)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMinWidth", jint(minpixels));
	}
}

void QAndroidOffscreenEditText::setPadding(int left, int top, int right, int bottom)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setPadding", "IIII", jint(left), jint(top), jint(right), jint(bottom));
	}
}

void QAndroidOffscreenEditText::setPaintFlags(int flags)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setPaintFlags", jint(flags));
	}
}

void QAndroidOffscreenEditText::setSelectAllOnFocus(bool selectAllOnFocus)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setSelectAllOnFocus", jint(selectAllOnFocus));
	}
}

void QAndroidOffscreenEditText::setSingleLine(bool singleLine)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setSingleLine", jboolean(singleLine));
	}
}

void QAndroidOffscreenEditText::setTextColor(int color)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setTextColor", jint(color));
	}
}

void QAndroidOffscreenEditText::setTextScaleX(float size)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setTextScaleX", jfloat(size));
	}
}

void QAndroidOffscreenEditText::setTextIsSelectable(bool selectable)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setTextIsSelectable", jboolean(selectable));
	}
}

void QAndroidOffscreenEditText::setGravity(int gravity)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setGravity", jint(gravity));
	}
}

void QAndroidOffscreenEditText::setHeight(int pixels)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHeight", jint(pixels));
	}
}

void QAndroidOffscreenEditText::setHighlightColor(int color)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHighlightColor", jint(color));
	}
}

void QAndroidOffscreenEditText::setHint(const QString & hint)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHint", hint);
	}
}

void QAndroidOffscreenEditText::setHintTextColor(int color)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHintTextColor", jint(color));
	}
}

void QAndroidOffscreenEditText::setWidth(int pixels)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setWidth", jint(pixels));
	}
}

void QAndroidOffscreenEditText::setLineSpacing(float add, float mult)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setLineSpacing", "FF", jfloat(add), jfloat(mult));
	}
}

void QAndroidOffscreenEditText::setLines(int lines)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setLines", jint(lines));
	}
}

void QAndroidOffscreenEditText::setHorizontallyScrolling(bool whether)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHorizontallyScrolling", jboolean(whether));
	}
}

void QAndroidOffscreenEditText::setAllCaps(bool allCaps)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setAllCaps", jboolean(allCaps));
	}
}

void QAndroidOffscreenEditText::selectAll()
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("selectAll");
	}
}

void QAndroidOffscreenEditText::setSelection(int index)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setSelection", jint(index));
	}
}

void QAndroidOffscreenEditText::setSelection(int start, int stop)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setSelection", "II", jint(start), jint(stop));
	}
}


