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
	QAndroidSpeechRecognizer::ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH = QLatin1String("web_search"),

	QAndroidSpeechRecognizer::ANDROID_SPEECHRECOGNIZER_RESULTS_RECOGNITION = QLatin1String("results_recognition");


static const char * const c_recognition_listener_class_name_ = "ru/dublgis/androidhelpers/VoiceRecognitionListener";
static const char * const c_speech_recognizer_class_name_ = "android/speech/SpeechRecognizer";


static QStringList bundleResultsToQStringList(jobject jobundle)
{
	try
	{
		QJniObject bundle(jobundle, false);
		QScopedPointer<QJniObject> result_array(bundle.callParamObject(
			"getStringArrayList"
			, "java/util/ArrayList"
			, "Ljava/lang/String;"
			, QJniLocalRef(QAndroidSpeechRecognizer::ANDROID_SPEECHRECOGNIZER_RESULTS_RECOGNITION).jObject()));
		QStringList result;
		if (result_array && result_array->jObject())
		{
			jint size = result_array->callInt("size");
			QJniEnvPtr jep;
			for (jint i = 0; i < size; ++i)
			{
				QScopedPointer<QJniObject> str_object(result_array->callParamObject("get", "java/lang/Object", "I", i));
				result << jep.JStringToQString(static_cast<jstring>(str_object->jObject()));
			}
		}
		return result;
	}
	catch (const std::exception & e)
	{
		qCritical() << "QAndroidSpeechRecognizer: exception in bundleResultsToQStringList:" << e.what();
	}
	return QStringList();
}


Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnBeginningOfSpeech(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnBeginningOfSpeech();
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnEndOfSpeech(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnEndOfSpeech();
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnError(JNIEnv *, jobject, jlong param, jint code)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnError(static_cast<int>(code));
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnPartialResults(JNIEnv *, jobject, jlong param, jobject bundle_results)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnPartialResults(bundleResultsToQStringList(bundle_results));
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnReadyForSpeech(JNIEnv *, jobject, jlong param, jobject bundle_params)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			Q_UNUSED(bundle_params)
			myobject->javaOnReadyForSpeech();
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnResults(JNIEnv *, jobject, jlong param, jobject bundle_results, jboolean secure)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnResults(bundleResultsToQStringList(bundle_results), static_cast<bool>(secure));
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_QAndroidSpeechRecognizer_nativeOnRmsChanged(JNIEnv *, jobject, jlong param, jfloat rmsdB)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidSpeechRecognizer * myobject = reinterpret_cast<QAndroidSpeechRecognizer*>(vp);
		if (myobject)
		{
			myobject->javaOnRmsdBChanged(static_cast<float>(rmsdB));
			return;
		}
	}
	qWarning() << __FUNCTION__ <<" Zero param!";
}




QAndroidSpeechRecognizer::QAndroidSpeechRecognizer(QObject * p)
	: QObject(p)
	, listening_(false)
	, rmsdB_(0.0f)
	, enable_timeout_timer_(false)
{
	preloadJavaClasses();

	connect(&timeout_timer_, SIGNAL(timeout()), this, SLOT(onTimeoutTimerTimeout()));
	timeout_timer_.setSingleShot(true);

	if (isRecognitionAvailableStatic())
	{
		try
		{
			listener_.reset(new QJniObject(c_recognition_listener_class_name_));
			listener_->callParamVoid("initialize", "Landroid/app/Activity;", QAndroidQPAPluginGap::Context().jObject());
			listener_->callVoid("setNativePtr", reinterpret_cast<jlong>(this));

			static const JNINativeMethod methods[] = {
				{"nativeOnBeginningOfSpeech", "(J)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnBeginningOfSpeech},
				{"nativeOnEndOfSpeech", "(J)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnEndOfSpeech},
				{"nativeOnError", "(JI)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnError},
				{"nativeOnPartialResults", "(JLandroid/os/Bundle;)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnPartialResults},
				{"nativeOnReadyForSpeech", "(JLandroid/os/Bundle;)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnReadyForSpeech},
				{"nativeOnResults", "(JLandroid/os/Bundle;)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnResults},
				{"nativeOnRmsChanged", "(JF)V", (void*)Java_QAndroidSpeechRecognizer_nativeOnRmsChanged},
			};
			listener_->registerNativeMethods(methods, sizeof(methods));

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
			if (listening_)
			{
				cancel();
			}

			QJniObject intent("android/content/Intent");
			QScopedPointer<QJniObject>(intent.callParamObject(
				"setAction"
				, "android/content/Intent"
				, "Ljava/lang/String;"
				, QJniLocalRef(action).jObject()));

			for (QMap<QString, QString>::const_iterator it = string_extras_.begin(); it != string_extras_.end(); ++it)
			{
				QScopedPointer<QJniObject>(intent.callParamObject(
					"putExtra"
					, "android/content/Intent"
					, "Ljava/lang/String;Ljava/lang/String;"
					, QJniLocalRef(it.key()).jObject()
					, QJniLocalRef(it.value()).jObject()));
			}

			for (QMap<QString, bool>::const_iterator it = bool_extras_.begin(); it != bool_extras_.end(); ++it)
			{
				QScopedPointer<QJniObject>(intent.callParamObject(
					"putExtra"
					, "android/content/Intent"
					, "Ljava/lang/String;Z"
					, QJniLocalRef(it.key()).jObject()
					, static_cast<jboolean>(it.value())));
			}

			for (QMap<QString, int>::const_iterator it = int_extras_.begin(); it != int_extras_.end(); ++it)
			{
				QScopedPointer<QJniObject>(intent.callParamObject(
					"putExtra"
					, "android/content/Intent"
					, "Ljava/lang/String;I"
					, QJniLocalRef(it.key()).jObject()
					, static_cast<jint>(it.value())));
			}

			listener_->callParamVoid("startListening", "Landroid/content/Intent;", intent.jObject());

			listening_ = true;
			emit listeningChanged(listening_);

			if (enable_timeout_timer_)
			{
				timeout_timer_.start();
			}
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
			listeningStopped();
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
			listeningStopped();
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
	listeningStopped();
	emit error(code, message);
}

void QAndroidSpeechRecognizer::javaOnPartialResults(const QStringList & res)
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__ << ":" << res.join(QLatin1String(" | "));
	#endif
	if(!res.isEmpty())
	{
		emit partialResults(res);
		emit partialResult(res.last());
	}
	if (enable_timeout_timer_ && timeout_timer_.isActive())
	{
		timeout_timer_.stop();
		timeout_timer_.start();
	}
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
	listeningStopped();
	if (!res.isEmpty())
	{
		emit results(res, secure);
		emit result(res.last(), secure);
	}
}

void QAndroidSpeechRecognizer::javaOnRmsdBChanged(float rmsdb)
{
	// Too many of these!
	// #if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
	// 	qDebug() << __PRETTY_FUNCTION__ << rmsdb;
	// #endif
	rmsdB_ = rmsdb;
	emit rmsdBChanged(rmsdB_);
}

void QAndroidSpeechRecognizer::onTimeoutTimerTimeout()
{
	#if defined(ANDROIDSPEECHRECOGNIZER_VERBOSE)
		qDebug() << __PRETTY_FUNCTION__;
	#endif
	if (listening_)
	{
		stopListening();
	}
}

void QAndroidSpeechRecognizer::startListeningFreeForm()
{
	addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL, ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_FREE_FORM);
	startListening(ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH);
}

void QAndroidSpeechRecognizer::startListeningWebSearch()
{
	addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL, ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH);
	startListening(ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH);
}

void QAndroidSpeechRecognizer::startListeningHandsFree()
{
	startListening(ANDROID_RECOGNIZERINTENT_ACTION_VOICE_SEARCH_HANDS_FREE);
}

void QAndroidSpeechRecognizer::extraSetPrompt(const QString & prompt)
{
	addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_PROMPT, prompt);
}

void QAndroidSpeechRecognizer::extraSetLanguage(const QString & ietf_language)
{
	QString language = ietf_language;
	language.replace(QLatin1Char('_'), QLatin1Char('-'));
	addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE, language);
}

void QAndroidSpeechRecognizer::extraSetMaxResults(int results)
{
	addIntExtra(ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS, results);
}

void QAndroidSpeechRecognizer::extraSetPartialResults()
{
	addBoolExtra(ANDROID_RECOGNIZERINTENT_EXTRA_PARTIAL_RESULTS, true);
}

void QAndroidSpeechRecognizer::extraSetListeningTimeouts(
	int min_phrase_length_ms
	, int possibly_complete_ms
	, int complete_ms
	, int timer_workaround_ms)
{
	addIntExtra(ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_MINIMUM_LENGTH_MILLIS, min_phrase_length_ms);
	addIntExtra(ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, possibly_complete_ms);
	addIntExtra(ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS, complete_ms);

	if (timer_workaround_ms > 0)
	{
		enable_timeout_timer_ = true;
		extraSetPartialResults();
		timeout_timer_.setInterval(timer_workaround_ms);
	}
	else
	{
		enable_timeout_timer_ = false;
	}
}

void QAndroidSpeechRecognizer::listeningStopped()
{
	timeout_timer_.stop();
	if (listening_)
	{
		listening_ = false;
		emit listeningChanged(listening_);
	}
}

QString QAndroidSpeechRecognizer::errorCodeToMessage(int code)
{
	switch (code)
	{
	case ANDROID_SPEECHRECOGNIZER_ERROR_AUDIO:
		return tr("Audio recording error.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_CLIENT:
		return tr("Client side error.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_INSUFFICIENT_PERMISSIONS:
		return tr("Insufficient permissions.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK:
		return tr("Network error.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK_TIMEOUT:
		return tr("Network timeout.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_NO_MATCH:
		return tr("Sorry, your speech was not recognized. Please try again.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_RECOGNIZER_BUSY:
		return tr("Android voice recognition service is busy");
	case ANDROID_SPEECHRECOGNIZER_ERROR_SERVER:
		return tr("Audio recognition server error.");
	case ANDROID_SPEECHRECOGNIZER_ERROR_SPEECH_TIMEOUT:
		return tr("No speech input.");
	default:
		return tr("An unknown voice recognition error occured.");
	}
}
