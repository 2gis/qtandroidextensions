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

#pragma once
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QJniHelpers.h>


// SpeechRecognizer wrapper.
// Note: this class is QML-singleton friendly.
class QAndroidSpeechRecognizer
	: public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool listening READ listening NOTIFY listeningChanged)
	Q_PROPERTY(float rmsdB READ rmsdB NOTIFY rmsdBChanged)
public:
	QAndroidSpeechRecognizer(QObject * p = 0);
	virtual ~QAndroidSpeechRecognizer();

	static void preloadJavaClasses();
	static bool isRecognitionAvailableStatic();

	bool listening() const { return listening_; }
	float rmsdB() const { return rmsdB_; }

	static const int
		ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK_TIMEOUT = 1,
		ANDROID_SPEECHRECOGNIZER_ERROR_NETWORK = 2,
		ANDROID_SPEECHRECOGNIZER_ERROR_AUDIO = 3,
		ANDROID_SPEECHRECOGNIZER_ERROR_SERVER = 4,
		ANDROID_SPEECHRECOGNIZER_ERROR_CLIENT = 5,
		ANDROID_SPEECHRECOGNIZER_ERROR_SPEECH_TIMEOUT = 6,
		ANDROID_SPEECHRECOGNIZER_ERROR_NO_MATCH = 7,
		ANDROID_SPEECHRECOGNIZER_ERROR_RECOGNIZER_BUSY = 8,
		ANDROID_SPEECHRECOGNIZER_ERROR_INSUFFICIENT_PERMISSIONS = 9;

	static const int
		ANDROID_RECOGNIZERINTENT_RESULT_NO_MATCH = 1,
		ANDROID_RECOGNIZERINTENT_RESULT_CLIENT_ERROR = 2,
		ANDROID_RECOGNIZERINTENT_RESULT_SERVER_ERROR = 3,
		ANDROID_RECOGNIZERINTENT_RESULT_NETWORK_ERROR = 4,
		ANDROID_RECOGNIZERINTENT_RESULT_AUDIO_ERROR = 5;

	static const QString
		ANDROID_RECOGNIZERINTENT_ACTION_GET_LANGUAGE_DETAILS,
		ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH,
		ANDROID_RECOGNIZERINTENT_ACTION_VOICE_SEARCH_HANDS_FREE,
		ANDROID_RECOGNIZERINTENT_ACTION_WEB_SEARCH,

		ANDROID_RECOGNIZERINTENT_DETAILS_META_DATA,

		ANDROID_RECOGNIZERINTENT_EXTRA_CALLING_PACKAGE,
		ANDROID_RECOGNIZERINTENT_EXTRA_CONFIDENCE_SCORES,
		ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE,
		ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL,
		ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_PREFERENCE,
		ANDROID_RECOGNIZERINTENT_EXTRA_MAX_RESULTS,
		ANDROID_RECOGNIZERINTENT_EXTRA_ONLY_RETURN_LANGUAGE_PREFERENCE,
		ANDROID_RECOGNIZERINTENT_EXTRA_ORIGIN,
		ANDROID_RECOGNIZERINTENT_EXTRA_PARTIAL_RESULTS,
		ANDROID_RECOGNIZERINTENT_EXTRA_PREFER_OFFLINE,
		ANDROID_RECOGNIZERINTENT_EXTRA_PROMPT,
		ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS,
		ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS_PENDINGINTENT,
		ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS_PENDINGINTENT_BUNDLE,
		ANDROID_RECOGNIZERINTENT_EXTRA_SECURE,
		ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS,
		ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_MINIMUM_LENGTH_MILLIS,
		ANDROID_RECOGNIZERINTENT_EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS,
		ANDROID_RECOGNIZERINTENT_EXTRA_SUPPORTED_LANGUAGES,
		ANDROID_RECOGNIZERINTENT_EXTRA_WEB_SEARCH_ONLY,

		ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_FREE_FORM,
		ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH;

public slots:
	// SpeechRecognier functions
	bool isRecognitionAvailable() const;
	void startListening(const QString & action);
	void stopListening();
	void cancel();

	// Filling in extra parameters
	void clearExtras() { string_extras_.clear(); bool_extras_.clear(); int_extras_.clear(); }
	void addStringExtra(const QString & key, const QString & value) { string_extras_.insert(key,value); }
	void addBoolExtra(const QString & key, bool value) { bool_extras_.insert(key,value); }
	void addIntExtra(const QString & key, int value) { int_extras_.insert(key,value); }


	// Higher-level functions

	void startListeningFreeForm() {
		addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL, ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_FREE_FORM);
		startListening(ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH);
	}

	void startListeningWebSearch() {
		addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE_MODEL, ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH);
		startListening(ANDROID_RECOGNIZERINTENT_ACTION_RECOGNIZE_SPEECH);
	}

	void startListeningHandsFree() {
		startListening(ANDROID_RECOGNIZERINTENT_ACTION_VOICE_SEARCH_HANDS_FREE);
	}

	void extraSetPrompt(const QString & prompt) { addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_PROMPT, prompt); }
	void extraSetLanguage(const QString & ietf_language) { addStringExtra(ANDROID_RECOGNIZERINTENT_EXTRA_LANGUAGE, ietf_language); }
	void extraSetMaxResults(int results) { addIntExtra(ANDROID_RECOGNIZERINTENT_EXTRA_RESULTS, results); }
	void extraSetPartialResults() { addBoolExtra(ANDROID_RECOGNIZERINTENT_EXTRA_PARTIAL_RESULTS, true); }

signals:
	void listeningChanged(bool listening);
	void beginningOfSpeech();
	void endOfSpeech();
	void error(int code, QString message);
	void partialResults(const QStringList & results);
	void partialResult(const QString & voice_input);
	void readyForSpeech();
	void results(const QStringList & results, bool secure);
	void result(const QString & voice_input, bool secure);
	void rmsdBChanged(float rmsdb);

private slots:
	void javaOnBeginningOfSpeech();
	void javaOnEndOfSpeech();
	void javaOnError(int code);
	void javaOnPartialResults(const QStringList & results);
	void javaOnReadyForSpeech();
	void javaOnResults(const QStringList & results, bool secure);
	void javaOnRmsdBChanged(float rmsdb);

private:
	QString errorCodeToMessage(int code);

private:
	bool listening_;
	float rmsdB_;
	QMap<QString, QString> string_extras_;
	QMap<QString, bool> bool_extras_;
	QMap<QString, int> int_extras_;
	QScopedPointer<QJniObject> listener_;
};


