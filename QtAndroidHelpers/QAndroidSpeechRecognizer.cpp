/*
	Lightweight access to various Android APIs for Qt

	Author:
	Sergey A. Galin <sergey.galin@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2016, DoubleGIS, LLC.
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

#include <QtCore/QDebug>
#include <QAndroidQPAPluginGap.h>
#include "QAndroidSpeechRecognizer.h"


#define ANDROIDSPEECHRECOGNIZER_VERBOSE


const QString
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_ACTION_GET_LANGUAGE_DETAILS = QLatin1String("android.speech.action.GET_LANGUAGE_DETAILS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH = QLatin1String("android.speech.action.RECOGNIZE_SPEECH"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_ACTION_VOICE_SEARCH_HANDS_FREE = QLatin1String("android.speech.action.VOICE_SEARCH_HANDS_FREE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_ACTION_WEB_SEARCH = QLatin1String("android.speech.action.WEB_SEARCH"),

	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_DETAILS_META_DATA = QLatin1String("android.speech.DETAILS"),

	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_CALLING_PACKAGE = QLatin1String("calling_package"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_CONFIDENCE_SCORES = QLatin1String("android.speech.extra.CONFIDENCE_SCORES"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE = QLatin1String("android.speech.extra.LANGUAGE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL = QLatin1String("android.speech.extra.LANGUAGE_MODEL"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_PREFERENCE = QLatin1String("android.speech.extra.LANGUAGE_PREFERENCE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_MAX_RESULTS = QLatin1String("android.speech.extra.MAX_RESULTS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_ONLY_RETURN_LANGUAGE_PREFERENCE = QLatin1String("android.speech.extra.ONLY_RETURN_LANGUAGE_PREFERENCE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_ORIGIN = QLatin1String("android.speech.extra.ORIGIN"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_PARTIAL_RESULTS = QLatin1String("android.speech.extra.PARTIAL_RESULTS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_PREFER_OFFLINE = QLatin1String("android.speech.extra.PREFER_OFFLINE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_PROMPT = QLatin1String("android.speech.extra.PROMPT"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS = QLatin1String("android.speech.extra.RESULTS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS_PENDINGINTENT = QLatin1String("android.speech.extra.RESULTS_PENDINGINTENT"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS_PENDINGINTENT_BUNDLE = QLatin1String("android.speech.extra.RESULTS_PENDINGINTENT_BUNDLE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_SECURE = QLatin1String("android.speech.extras.EXTRA_SECURE"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS = QLatin1String("android.speech.extras.SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_MINIMUM_LENGTH_MILLIS = QLatin1String("android.speech.extras.SPEECH_INPUT_MINIMUM_LENGTH_MILLIS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS = QLatin1String("android.speech.extras.SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_SUPPORTED_LANGUAGES = QLatin1String("android.speech.extra.SUPPORTED_LANGUAGES"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_EXTRA_WEB_SEARCH_ONLY = QLatin1String("android.speech.extra.WEB_SEARCH_ONLY"),

	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_FREE_FORM = QLatin1String("free_form"),
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH = QLatin1String("web_search");


static const char * const c_recognition_listener_class_name_ = "ru/dublgis/androidhelpers/VoiceRecognitionListener";
static const char * const c_speech_recognizer_class_name_ = "android/speech/SpeechRecognizer";


QAndroidSpeechRecognizer::QAndroidSpeechRecognizer(QObject * p)
	: QObject(p)
	, listening_(false)
	, rmsdB_(0.0f)
{
	preloadJavaClasses();
	if (isRecognitionAvailableStatic())
	{
		try
		{
			listener_.reset(new QJniObject(c_recognition_listener_class_name_));
			listener_->callParamVoid("initialize", "Landroid/app/Activity;", QAndroidQPAPluginGap::Context().jObject());
			listener_->callVoid("setNativePtr", reinterpret_cast<jlong>(this));

			/*    public native void nativeOnBeginningOfSpeech(long ptr);
			public native void nativeOnEndOfSpeech(long ptr);
			public native void nativeOnError(long ptr, int error);
			public native void nativeOnPartialResults(long ptr, Bundle partialResults);
			public native void nativeOnReadyForSpeech(long ptr, Bundle params);
			public native void nativeOnResults(long ptr, Bundle results);
			public native void nativeOnRmsChanged(long ptr, float rmsdB);*/
			qDebug() << "SpeechRecognizer initialized successfully.";
		}
		catch(const std::exception & e)
		{
			qCritical() << "QAndroidSpeechRecognizer: Exception while creating Speech Recognizer:" << e.what();
			listener_.reset();
		}
	}
	else
	{
		qDebug() << "SpeechRecognizer not intialized because the interface is not available.";
	}
}

QAndroidSpeechRecognizer::~QAndroidSpeechRecognizer()
{
	try
	{
		if (listener_)
		{
			listener_->callVoid("setNativePtr", jlong(0));
		}
	}
	catch(const std::exception & e)
	{
		qCritical() << "QAndroidSpeechRecognizer: exception in the destructor:" << e.what();
	}
}

void QAndroidSpeechRecognizer::preloadJavaClasses()
{
	static bool s_preloaded = false;
	if (!s_preloaded)
	{
		s_preloaded = true;
		QAndroidQPAPluginGap::preloadJavaClass(c_speech_recognizer_class_name_);
		QAndroidQPAPluginGap::preloadJavaClass("android/content/Intent");
		QAndroidQPAPluginGap::preloadJavaClass(c_recognition_listener_class_name_);
	}
}

bool QAndroidSpeechRecognizer::isRecognitionAvailableStatic()
{
	preloadJavaClasses();
	jboolean result = QJniClass(c_speech_recognizer_class_name_)
		.callStaticParamBoolean("isRecognitionAvailable"
		, "Landroid/content/Context;"
		, QAndroidQPAPluginGap::Context().jObject());
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << result;
	#endif
	return static_cast<bool>(result);
}

bool QAndroidSpeechRecognizer::isRecognitionAvailable() const
{
	bool result = listener_ && isRecognitionAvailableStatic();
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << result;
	#endif
	return result;
}

void QAndroidSpeechRecognizer::startListening(const QString & action)
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << action;
	#endif
	try
	{
		if (listener_)
		{
			QJniObject intent("android/content/Intent");
			intent.callVoid("setAction", action);

			for (QMap<QString, QString>::const_iterator it = string_extras_.begin(); it != string_extras_.end(); ++it)
			{
				intent.callParamVoid(
					"putExtra"
					, "Ljava/lang/String;Ljava/lang/String;"
					, QJniLocalRef(it.key()).jObject()
					, QJniLocalRef(it.value()).jObject());
			}

			for (QMap<QString, bool>::const_iterator it = bool_extras_.begin(); it != bool_extras_.end(); ++it)
			{
				intent.callParamVoid(
					"putExtra"
					, "Ljava/lang/String;Z"
					, QJniLocalRef(it.key()).jObject()
					, static_cast<jboolean>(it.value()));
			}

			for (QMap<QString, int>::const_iterator it = int_extras_.begin(); it != int_extras_.end(); ++it)
			{
				intent.callParamVoid(
					"putExtra"
					, "Ljava/lang/String;I"
					, QJniLocalRef(it.key()).jObject()
					, static_cast<jint>(it.value()));
			}

			listener_->callParamVoid("startListening", "Landroid/content/Intent;", intent.jObject());
			listening_ = true;
			emit listeningChanged(listening_);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in QAndroidSpeechRecognizer::startListening:" << e.what();
	}
}

void QAndroidSpeechRecognizer::stopListening()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	try
	{
		if (listener_)
		{
			listener_->callVoid("stopListening");
			listening_ = false;
			emit listeningChanged(listening_);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in QAndroidSpeechRecognizer::stopListening:" << e.what();
	}
}

void QAndroidSpeechRecognizer::cancel()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	try
	{
		if (listener_)
		{
			listener_->callVoid("cancel");
			listening_ = false;
			emit listeningChanged(listening_);
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "Exception in QAndroidSpeechRecognizer::cancel:" << e.what();
	}
}

void QAndroidSpeechRecognizer::javaOnBeginningOfSpeech()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	emit beginningOfSpeech();
}

void QAndroidSpeechRecognizer::javaOnEndOfSpeech()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	emit endOfSpeech();
}

void QAndroidSpeechRecognizer::javaOnError(int code)
{
	QString message = errorCodeToMessage(code);
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << code << ":" << message;
	#endif
	emit error(code, message);
}

void QAndroidSpeechRecognizer::javaOnPartialResults(const QStringList & res)
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << ":" << res.join(QLatin1String(" | "));
	#endif
	emit partialResults(res);
	emit partialResult(res.join(QLatin1String(" ")));
}

void QAndroidSpeechRecognizer::javaOnReadyForSpeech()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	emit readyForSpeech();
}

void QAndroidSpeechRecognizer::javaOnResults(const QStringList & res, bool secure)
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << ", secure =" << secure << ":" << res.join(QLatin1String(" | "));
	#endif
	emit results(res, secure);
	emit result(res.join(QLatin1String(" ")), secure);
}

void QAndroidSpeechRecognizer::javaOnRmsdBChanged(float rmsdb)
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << rmsdb;
	#endif
	rmsdB_ = rmsdb;
	emit rmsdBChanged(rmsdB_);
}

QString QAndroidSpeechRecognizer::errorCodeToMessage(int code)
{
	switch (code)
	{
	case ANDROID_SPEECHRECOGNIZER_ERROR_AUDIO:
		return tr("Audio recording error");
	case ANDROID_SPEECHRECOGNIZER_ERROR_CLIENT:
		return tr("Client side error");
	case ANDROID_SPEECHRECOGNIZER_ERROR_INSUFFICIENT_PERMISSIONS:
		return tr("Insufficient permissions");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK:
		return tr("Network error");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK_TIMEOUT:
		return tr("Network timeout");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NO_MATCH:
		return tr("No recognition result matched");
	case ANDROID_SPEECHRECOGNIZER_ERROR_RECOGNIZER_BUSY:
		return tr("Android voice recognition service is busy");
	case ANDROID_SPEECHRECOGNIZER_ERROR_SERVER:
		return tr("Audio recognition server error");
	case ANDROID_SPEECHRECOGNIZER_ERROR_SPEECH_TIMEOUT:
		return tr("No speech input");
	default:
		return tr("An unknown voice recognition error occured.");
	}
}
