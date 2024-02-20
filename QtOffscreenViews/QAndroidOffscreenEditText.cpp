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

#include <QtCore/QMetaObject>
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
			try
			{
				edit->javaOnTextChanged(QJniEnvPtr().toQString(str), start, before, count);
			}
			catch (const std::exception & e)
			{
				qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
			}
			return;
		}
	}
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
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
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
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
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_AndroidOffscreenEditText_nativeSetSelectionInfo(JNIEnv *, jobject, jlong param, jint top, jint bottom)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			QMetaObject::invokeMethod(edit, "javaSetSelectionInfo", Q_ARG(int, top),  Q_ARG(int, bottom));
			return;
		}
	}
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_AndroidOffscreenEditText_nativeOnContentHeightChanged(JNIEnv *, jobject, jlong param, jint height)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			QMetaObject::invokeMethod(edit, "javaOnContentHeightChanged", Q_ARG(int, height));
			return;
		}
	}
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_AndroidOffscreenEditText_nativeOnHasAcceptableInputChanged(JNIEnv *, jobject, jlong param, jboolean hasAcceptableInput)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenEditText * edit = qobject_cast<QAndroidOffscreenEditText*>(reinterpret_cast<QAndroidOffscreenView*>(vp));
		if (edit)
		{
			QMetaObject::invokeMethod(edit, "javaOnHasAcceptableInputChanged", Q_ARG(bool, hasAcceptableInput ? true : false));
			return;
		}
	}
	qWarning() << __PRETTY_FUNCTION__ << "Zero param!";
}

QAndroidOffscreenEditText::QAndroidOffscreenEditText(
		const QString & object_name,
		const QSize & def_size,
		QObject * parent)
	: QAndroidOffscreenView(QLatin1String("OffscreenEditText"), object_name, def_size, parent)
	, paint_flags_(ANDROID_PAINT_DEV_KERN_TEXT_FLAG | ANDROID_PAINT_ANTI_ALIAS_FLAG)
{
	setAttachingMode(true);
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.registerNativeMethods({
				{"nativeOnTextChanged", "(JLjava/lang/String;III)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnTextChanged)},
				{"nativeOnKey", "(JZI)Z", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnKey)},
				{"nativeOnEditorAction", "(JI)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnEditorAction)},
				{"nativeSetSelectionInfo", "(JII)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeSetSelectionInfo)},
				{"nativeOnContentHeightChanged", "(JI)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnContentHeightChanged)},
				{"nativeOnHasAcceptableInputChanged", "(JZ)V", reinterpret_cast<void*>(Java_AndroidOffscreenEditText_nativeOnHasAcceptableInputChanged)},
			});
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

QAndroidOffscreenEditText::~QAndroidOffscreenEditText()
{
}

void QAndroidOffscreenEditText::preloadJavaClasses()
{
	try
	{
		QAndroidOffscreenView::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass(
			(getDefaultJavaClassPath() + QLatin1String("OffscreenEditText")).toLatin1());
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::javaOnTextChanged(const QString & str, int start, int before, int count)
{
	emit onTextChanged(str, start, before, count);
	emit onTextChanged();
}

bool QAndroidOffscreenEditText::javaOnKey(bool down, int androidKey)
{
	switch(androidKey)
	{
	case 0x00000004: // KEYCODE_BACK
		emit onKeyBack(down);
		return true;
	case 0x00000017: // KEYCODE_DPAD_CENTER
	case 0x00000042: // KEYCODE_ENTER
		if (down)
		{
			emit onEnter();
			emit onEnterOrPositiveAction();
		}
		break;
	default:
		break;
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
		emit onPositiveAction();
	}
}

void QAndroidOffscreenEditText::javaSetSelectionInfo(int top, int bottom)
{
	selection_top_ = top;
	selection_bottom_ = bottom;
	emit selectionChanged();
}

void QAndroidOffscreenEditText::setText(const QString & text)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setText", text);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

QString QAndroidOffscreenEditText::getText() const
{
	try
	{
		if (QJniObject & view = const_cast<QAndroidOffscreenEditText*>(this)->offscreenView())
		{
			return view.callString("getText");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
	return QString();
}

void QAndroidOffscreenEditText::setTextSize(float size, int unit)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setTextSize", "FI", jfloat(size), jint(unit));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTypeface(const QString & name, int style)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setTypeface", "Ljava/lang/String;I", QJniLocalRef(name).jObject(), jint(style));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTypefaceFromFile(const QString & filename, int style)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setTypefaceFromFile", "Ljava/lang/String;I", QJniLocalRef(filename).jObject(), style);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTypefaceFromAsset(const QString & filename, int style)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setTypefaceFromAsset", "Ljava/lang/String;I", QJniLocalRef(filename).jObject(), style);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setCursorVisible(bool visible)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setCursorVisible", jboolean(visible));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setAutofillType(int type)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setAutofillType", type);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHintType(const QString & type)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHintType", type);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setInputType(int type)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setInputType", jint(type));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setInputType(int type_and, int type_or)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setInputType", "II", jint(type_and), jint(type_or));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMarqueeRepeatLimit(int marqueeLimit)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMarqueeRepeatLimit", jint(marqueeLimit));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMaxEms(int maxems)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMaxEms", jint(maxems));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMinEms(int minems)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMinEms", jint(minems));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMaxHeight(int maxHeight)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMaxHeight", jint(maxHeight));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMinHeight(int minHeight)
{
	if (QJniObject & view = offscreenView())
	{
		view.callVoid("setMinHeight", jint(minHeight));
	}
}

void QAndroidOffscreenEditText::setMaxLines(int maxlines)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMaxLines", jint(maxlines));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMinLines(int minlines)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMinLines", jint(minlines));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMaxWidth(int maxpixels)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMaxWidth", jint(maxpixels));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMinWidth(int minpixels)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMinWidth", jint(minpixels));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setPadding(int left, int top, int right, int bottom)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setPadding", "IIII", jint(left), jint(top), jint(right), jint(bottom));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setIncludeFontPadding(bool enabled)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setIncludeFontPadding", jboolean(enabled));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setPaintFlags(int flags)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			paint_flags_ = flags;
			view.callVoid("setPaintFlags", jint(flags));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

int QAndroidOffscreenEditText::getPaintFlags()
{
	return paint_flags_;
}

void QAndroidOffscreenEditText::setSelectAllOnFocus(bool selectAllOnFocus)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setSelectAllOnFocus", jint(selectAllOnFocus));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setSingleLine(bool singleLine)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setSingleLine", jboolean(singleLine));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTextColor(int color)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setTextColor", jint(color));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTextScaleX(float size)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setTextScaleX", jfloat(size));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setTextIsSelectable(bool selectable)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setTextIsSelectable", jboolean(selectable));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setGravity(int gravity)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setGravity", jint(gravity));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHeight(int pixels)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHeight", jint(pixels));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHighlightColor(int color)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHighlightColor", jint(color));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHint(const QString & hint)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHint", hint);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHintTextColor(int color)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHintTextColor", jint(color));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setWidth(int pixels)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setWidth", jint(pixels));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setLineSpacing(float add, float mult)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setLineSpacing", "FF", jfloat(add), jfloat(mult));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setLines(int lines)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setLines", jint(lines));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setHorizontallyScrolling(bool whether)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHorizontallyScrolling", jboolean(whether));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setVerticallyScrolling(bool whether)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setVerticallyScrolling", jboolean(whether));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setAllCaps(bool allCaps)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setAllCaps", jboolean(allCaps));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setPasswordMode(bool enable)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setPasswordMode", jboolean(enable));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setPasswordModeWithDefaultTypeface(bool enable)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setPasswordModeWithDefaultTypeface", jboolean(enable));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setImeOptions(int and_mask, int or_mask)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setImeOptions", "II", static_cast<jint>(and_mask), static_cast<jint>(or_mask));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setEllipsize(AndroidTruncateAt ellipsis)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setEllipsize", jint(ellipsis));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::selectAll()
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("selectAll");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setSelection(int index)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setSelection", jint(index));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setSelection(int start, int stop)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callParamVoid("setSelection", "II", jint(start), jint(stop));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

int QAndroidOffscreenEditText::getSelectionStart()
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			return view.callInt("getSelectionStart");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
	return 0; // No selection start
}

void QAndroidOffscreenEditText::setHorizontalScrollBarEnabled(bool horizontalScrollBarEnabled)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setHorizontalScrollBarEnabled", jboolean(horizontalScrollBarEnabled));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setVerticalScrollBarEnabled(bool verticalScrollBarEnabled)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setVerticalScrollBarEnabled", jboolean(verticalScrollBarEnabled));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

int QAndroidOffscreenEditText::getSelectionEnd()
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			return view.callInt("getSelectionEnd");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
	return 0; // No selection end
}

int QAndroidOffscreenEditText::getSelectionTop() const
{
	return selection_top_;
}

int QAndroidOffscreenEditText::getSelectionBottom() const
{
	return selection_bottom_;
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
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setAllowFullscreenKeyboard", jboolean(allow));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setCursorColorToTextColor()
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setCursorColorToTextColor");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setCursorByDrawableName(const QString & name)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setCursorByDrawableName", name);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setMaxLength(int length)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setMaxLength", jint(length));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::setRichTextMode(bool enabled)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setRichTextMode", jboolean(enabled));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

int QAndroidOffscreenEditText::getSystemDrawMode()
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			return static_cast<int>(view.callInt("getSystemDrawMode"));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
	return 0;
}

void QAndroidOffscreenEditText::setSystemDrawMode(int mode)
{
	try
	{
		if (QJniObject & view = offscreenView())
		{
			view.callVoid("setSystemDrawMode", static_cast<jint>(mode));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
	}
}

void QAndroidOffscreenEditText::javaOnContentHeightChanged(int height)
{
	if (content_height_ != height)
	{
		content_height_ = height;
		emit contentHeightChanged(height);
	}
}

void QAndroidOffscreenEditText::setInputMask(const QString & inputMask)
{
	if (QJniObject & view = offscreenView())
	{
		try 
		{
			view.callVoid("setInputMask", inputMask);
		}
		catch (const std::exception & e)
		{
			qCritical() << "JNI exception in" << __PRETTY_FUNCTION__ << ":" << e.what();
		}
	}
}

bool QAndroidOffscreenEditText::hasAcceptableInput() const
{
	return hasAcceptableInput_;
}

void QAndroidOffscreenEditText::javaOnHasAcceptableInputChanged(bool hasAcceptableInput)
{

	if (hasAcceptableInput_ != hasAcceptableInput) 
	{
		hasAcceptableInput_ = hasAcceptableInput;
		emit hasAcceptableInputChanged(hasAcceptableInput_);
	}
}
