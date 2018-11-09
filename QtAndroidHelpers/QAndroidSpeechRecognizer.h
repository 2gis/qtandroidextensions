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
#include <QtCore/QTimer>
#include <QtCore/QVariantList>
#include <QtCore/QSharedPointer>
#include <QJniHelpers/QJniHelpers.h>


// SpeechRecognizer wrapper.
// Note: this class is QML friendly, it can be registered as a creatable QML object or singleton.
// The object must be created in the main thread.
class QAndroidSpeechRecognizer
	: public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool listening READ listening NOTIFY listeningChanged)
	Q_PROPERTY(float rmsdB READ rmsdB NOTIFY rmsdBChanged)
	Q_PROPERTY(
		int permissionRequestCode
		READ permissionRequestCode
		WRITE setPermissionRequestCode
		NOTIFY permissionRequestCodeChanged)
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
		ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_WEB_SEARCH,

		ANDROID_SPEECHRECOGNIZER_RESULTS_RECOGNITION,
		ANDROID_SPEECHRECOGNIZER_CONFIDENCE_SCORES;

public slots:
	// SpeechRecognier functions
	// Note: see also checkRuntimePermissions(bool) - we may have the voice recognition
	// but no permission to use it.
	bool isRecognitionAvailable() const;
	// Version of isRecognitionAvailable() that only does the check once per session.
	bool isRecognitionAvailableCached() const;
	// Make sure to call setPermissionRequestCode(int) before doing it.
	bool startListening(const QString & action);
	void stopListening();
	void cancel();

	// Check for dynamic run-time permissions on Android 6+.
	// Make sure to call setPermissionRequestCode(int) before doing it.
	// On Android < 6 always returns true.
	// You may want to call to isRecognitionAvailable() first to check if it makes
	// sense to show the permission request dialog.
	// The permissions are requested automatically when calling to startListening().
	bool checkRuntimePermissions(bool request_if_necessary) const;

	// The languages are returned via supportedLanguagesReceived(QStringList) signal.
	void requestSupportedLanguages();

	// Filling in extra parameters of future voice recognition intents (see: RecognizerIntent).
	void clearExtras();
	void addStringExtra(const QString & key, const QString & value);
	void addBoolExtra(const QString & key, bool value);
	void addIntExtra(const QString & key, int value);


	// Higher-level functions

	void startListeningFreeForm();
	void startListeningWebSearch();
	void startListeningHandsFree();

	void extraSetPrompt(const QString & prompt);
	void extraSetLanguage(const QString & ietf_language);
	void extraSetMaxResults(int results);
	void extraSetPartialResults();

	// Set voice input timeout parameters.
	// Passing a value <=0 will leave the timeout on system default.
	// Please note that the first 3 parameters are handled by OS and don't work
	// on certain versions, e.g. on Android 4.3.
	// Note: setting timer_workaround_ms >0 also enables partial results event.
	void extraSetListeningTimeouts(
		int min_phrase_length_ms
		, int possibly_complete_ms
		, int complete_ms
		, int timer_workaround_ms);

	// Set workaround timer which aborts listening after timer_workaround_ms
	// since the last new recognizer result.
	void extraSetWorkaroundTimeout(int timer_workaround_ms);

	int permissionRequestCode() const;
	void setPermissionRequestCode(int code);

	// See startVoiceRecognitionActivity
	bool isVoiceRecognitionActivityAvailable() const;
	// Version of isVoiceRecognitionActivityAvailable() that only does the check once per session.
	bool isVoiceRecognitionActivityAvailableCached() const;

	// This is a function which allows to implement voice recognition without implementing
	// any UI. In this case, you have to implement onActivityResult(int requestCode,
	// int resultCode, Intent data) in your activity and handle result of the activity there.
	// You can use this helper:
	// ru.dublgis.androidhelpers.VoiceRecognitionListener.getVoiceRecognitionActivityResult()
	// If prompt is empty, it is not set for the activity.
	// If language_model is empty, ANDROID_RECOGNIZERINTENT_LANGUAGE_MODEL_FREE_FORM is used.
	bool startVoiceRecognitionActivity(
		int request_code
		, const QString & prompt = QString()
		, const QString & language_model = QString()) const;

signals:
	void listeningChanged(bool listening);
	void beginningOfSpeech();
	void endOfSpeech();
	void error(int code, QString message);
	void partialResults(const QVariantList & results, const QVariantList & scores);
	// "secure" is set to true if device is currently in locked state so no unsafe operations allowed
	// (may happen only when using "hands free" recognition).
	void results(const QVariantList & results, const QVariantList & scores, bool secure);
	void readyForSpeech();
	void rmsdBChanged(float rmsdb);
	void permissionRequestCodeChanged(int code);
	void supportedLanguagesReceived(const QStringList & ietf_languages);

private slots:
	void javaOnBeginningOfSpeech();
	void javaOnEndOfSpeech();
	void javaOnError(int code);
	void javaOnPartialResults(const QVariantList & results, const QVariantList & confidence_scores);
	void javaOnResults(const QVariantList & results, const QVariantList & confidence_scores, bool secure);
	void javaOnReadyForSpeech();
	void javaOnRmsdBChanged(float rmsdb);
	void javaSupportedLanguagesReceived(const QStringList & languages);
	void onTimeoutTimerTimeout();

private:
	void listeningStarted();
	void listeningStopped();
	QString errorCodeToMessage(int code);

private:
	bool listening_;
	float rmsdB_;
	QMap<QString, QString> string_extras_;
	QMap<QString, bool> bool_extras_;
	QMap<QString, int> int_extras_;
	QScopedPointer<QJniObject> listener_;

	QTimer timeout_timer_;
	bool enable_timeout_timer_;
	QVariantList previous_partial_results_;

	int permission_request_code_;
};




