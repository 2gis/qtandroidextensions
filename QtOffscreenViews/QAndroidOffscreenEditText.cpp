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

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include "QAndroidOffscreenEditText.h"


Q_DECL_EXPORT void JNICALL Java_AndroidOffscreenEditText_nativeOnTextChanged(JNIEnv *, jobject, jlong param, jstring str, jint start, jint before, jint count)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			QString qstr = QJniEnvPtr().JStringToQString(str);
			edit->javaOnTextChanged(qstr, start, before, count);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

Q_DECL_EXPORT jboolean JNICALL Java_AndroidOffscreenEditText_nativeOnKey(JNIEnv *, jobject, jlong param, jboolean down, jint keycode)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			return jboolean(edit->javaOnKey(down? true: false, keycode));
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
	return JNI_FALSE;
}

Q_DECL_EXPORT void JNICALL Java_AndroidOffscreenEditText_nativeOnEditorAction(JNIEnv *, jobject, jlong param, jint action)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			edit->javaOnEditorAction(action);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}




QAndroidOffscreenEditText::QAndroidOffscreenEditText(const QString & object_name, const QSize & def_size, QObject * parent)
	: QAndroidOffscreenView(QLatin1String("OffscreenEditText"), object_name, def_size, parent)
	, paint_flags_(ANDROID_PAINT_DEV_KERN_TEXT_FLAG | ANDROID_PAINT_ANTI_ALIAS_FLAG)
{
	setAttachingMode(true);
	if (QJniObject * view = offscreenView())
	{
		static const JNINativeMethod methods[] = {
			{"nativeOnTextChanged", "(JLjava/lang/String;III)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnTextChanged)},
			{"nativeOnKey", "(JZI)Z", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnKey)},
			{"nativeOnEditorAction", "(JI)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnEditorAction)},
		};
		view->registerNativeMethods(methods, sizeof(methods));
	}
}

QAndroidOffscreenEditText::~QAndroidOffscreenEditText()
{
}

void QAndroidOffscreenEditText::preloadJavaClasses()
{
	QAndroidOffscreenView::preloadJavaClasses();
	QAndroidQPAPluginGap::preloadJavaClass((getDefaultJavaClassPath() + QLatin1String("OffscreenEditText")).toLatin1());
}

void QAndroidOffscreenEditText::javaOnTextChanged(const QString & str, int start, int before, int count)
{
	emit onTextChanged(str, start, before, count);
	emit onTextChanged();
}

bool QAndroidOffscreenEditText::javaOnKey(bool down, int androidKey)
{
	if (down)
	{
		switch(androidKey)
		{
		case 0x00000004: // KEYCODE_BACK
			emit onKeyBack(down);
			return true;
		case 0x00000017: // KEYCODE_DPAD_CENTER
		case 0x00000042: // KEYCODE_ENTER
			emit onEnter();
			emit onEnterOrPositiveAction();
			break;
		default:
			break;
		}
	}
	return false;
}

void QAndroidOffscreenEditText::javaOnEditorAction(int action)
{
	qDebug()<<__FUNCTION__<<action;
	emit onEditorAction(action);
	switch (action)
	{
	case ANDROID_EDITORINFO_IME_ACTION_DONE:
	case ANDROID_EDITORINFO_IME_ACTION_GO:
	case ANDROID_EDITORINFO_IME_ACTION_NEXT:
	case ANDROID_EDITORINFO_IME_ACTION_SEARCH:
	case ANDROID_EDITORINFO_IME_ACTION_SEND:
	case ANDROID_EDITORINFO_IME_ACTION_UNSPECIFIED:
	default:
		emit onEnterOrPositiveAction();
	}
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
		view->callParamVoid("setTypeface", "Ljava/lang/String;I", QJniLocalRef(name).jObject(), jint(style));
	}
}

void QAndroidOffscreenEditText::setTypefaceFromFile(const QString & filename, int style)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setTypefaceFromFile", "Ljava/lang/String;I", QJniLocalRef(filename).jObject(), style);
	}
}

void QAndroidOffscreenEditText::setTypefaceFromAsset(const QString & filename, int style)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setTypefaceFromAsset", "Ljava/lang/String;I", QJniLocalRef(filename).jObject(), style);
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

void QAndroidOffscreenEditText::setInputType(int type_and, int type_or)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setInputType", "II", jint(type_and), jint(type_or));
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
		paint_flags_ = flags;
		view->callVoid("setPaintFlags", jint(flags));
	}
}

int QAndroidOffscreenEditText::getPaintFlags()
{
	return paint_flags_;
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

void QAndroidOffscreenEditText::setPasswordMode()
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setPasswordMode");
	}
}

void QAndroidOffscreenEditText::setPasswordModeWithDefaultTypeface(bool enable)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setPasswordModeWithDefaultTypeface", jboolean(enable));
	}
}

void QAndroidOffscreenEditText::setImeOptions(int and_mask, int or_mask)
{
	if (QJniObject * view = offscreenView())
	{
		view->callParamVoid("setImeOptions", "II", static_cast<jint>(and_mask), static_cast<jint>(or_mask));
	}
}

void QAndroidOffscreenEditText::setEllipsize(AndroidTruncateAt ellipsis)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setEllipsize", jint(ellipsis));
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

int QAndroidOffscreenEditText::getSelectionStart()
{
	if (QJniObject * view = offscreenView())
	{
		return view->callInt("getSelectionStart");
	}
	return 0; // No selection start
}

void QAndroidOffscreenEditText::setHorizontalScrollBarEnabled(bool horizontalScrollBarEnabled)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setHorizontalScrollBarEnabled", jboolean(horizontalScrollBarEnabled));
	}
}

void QAndroidOffscreenEditText::setVerticalScrollBarEnabled(bool verticalScrollBarEnabled)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setVerticalScrollBarEnabled", jboolean(verticalScrollBarEnabled));
	}
}

int QAndroidOffscreenEditText::getSelectionEnd()
{
	if (QJniObject * view = offscreenView())
	{
		return view->callInt("getSelectionEnd");
	}
	return 0; // No selection end
}

void QAndroidOffscreenEditText::setTextColor(const QColor & color)
{
	setTextColor(QColorToAndroidColor(color));
}

void QAndroidOffscreenEditText::setHighlightColor(const QColor & color)
{
	setHighlightColor(QColorToAndroidColor(color));
}

void QAndroidOffscreenEditText::setHintTextColor(const QColor & color)
{
	setHintTextColor(QColorToAndroidColor(color));
}

void QAndroidOffscreenEditText::setAllowFullscreenKeyboard(bool allow)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setAllowFullscreenKeyboard", jboolean(allow));
	}
}

void QAndroidOffscreenEditText::setCursorColorToTextColor()
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setCursorColorToTextColor");
	}
}

void QAndroidOffscreenEditText::setMaxLength(int length)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setMaxLength", jint(length));
	}
}

int QAndroidOffscreenEditText::getSystemDrawMode()
{
	if (QJniObject * view = offscreenView())
	{
		return static_cast<int>(view->callInt("getSystemDrawMode"));
	}
	return 0;
}

void QAndroidOffscreenEditText::setSystemDrawMode(int mode)
{
	if (QJniObject * view = offscreenView())
	{
		view->callVoid("setSystemDrawMode", static_cast<jint>(mode));
	}
}



