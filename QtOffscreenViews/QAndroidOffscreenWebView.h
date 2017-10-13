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

#pragma once
#include <QtCore/QMap>
#include "QAndroidOffscreenView.h"

class QAndroidOffscreenWebView
	: public QAndroidOffscreenView
{
	Q_OBJECT
	Q_PROPERTY(bool ignoreSslErrors READ getIgnoreSslErrors WRITE setIgnoreSslErrors)
public:
	QAndroidOffscreenWebView(const QString & object_name, const QSize & def_size, QObject * parent = 0);
	virtual ~QAndroidOffscreenWebView();

	/*!
	 * This function should be called from main() to make sure that it all will work from QML threads.
	 * For Qt4/Grym plugin this is not necessary if you use QAndroidOffscreenView functions from GUI thread.
	 */
	static void preloadJavaClasses();

	/*!
	 * Start loading specified URL.
	 */
	bool loadUrl(const QString & url);

	/*!
	 * Start loading specified URL.
	 * \param additionalHttpHeaders contains the additional headers. The headers should not contain '\n' symbols.
	 */
	bool loadUrl(const QString & url, const QMap<QString, QString> & additionalHttpHeaders);

	/*!
	 * Load document from a string.
	 */
	bool loadData(const QString & text, const QString & mime = QLatin1String("text/html"), const QString & encoding = QString::null);

	/*!
	 * Start loading specified URL.
	 */
	bool loadDataWithBaseURL(const QString & baseUrl, const QString & data, const QString & mimeType= QLatin1String("text/html"), const QString & encoding = QString::null, const QString & historyUrl = QString::null);

	//! Will emit contentHeightReceived(int) after done.
	bool requestContentHeight();

	//! Gets whether this WebView has a back history item.
	void requestCanGoBack();
	void goBack();

	//! Gets whether the page can go back or forward the given number of steps.
	void requestCanGoBackOrForward(int steps);
	void goBackOrForward(int steps);

	//! Gets whether this WebView has a forward history item.
	void requestCanGoForward();
	void goForward();

	bool getIgnoreSslErrors() const { return ignore_ssl_errors_; }
	void setIgnoreSslErrors(bool ignore) { ignore_ssl_errors_ = ignore; }
	void setWebContentsDebuggingEnabled(bool enabled);

	/*
	Unimplemented WebView functions:

	void addJavascriptInterface(Object object, String name) // Injects the supplied Java object into this WebView.
	void 	clearCache(boolean includeDiskFiles) // Clears the resource cache.
	void 	clearFormData() // Removes the autocomplete popup from the currently focused form field, if present.
	void 	clearHistory() // Tells this WebView to clear its internal back/forward list.
	void 	clearMatches() // Clears the highlighting surrounding text matches created by findAllAsync(String).
	void 	clearSslPreferences() // Clears the SSL preferences table stored in response to proceeding with SSL certificate errors.
	void 	computeScroll() // Called by a parent to request that a child update its values for mScrollX and mScrollY if necessary.
	WebBackForwardList copyBackForwardList() // Gets the WebBackForwardList for this WebView.
	PrintDocumentAdapter createPrintDocumentAdapter() // Creates a PrintDocumentAdapter that provides the content of this Webview for printing.
	boolean dispatchKeyEvent(KeyEvent event) // Dispatch a key event to the next view on the focus path.
	void 	documentHasImages(Message response) // Queries the document to see if it contains any image references.
	abstract void dumpViewHierarchyWithProperties(BufferedWriter out, int level) // Dumps custom children to hierarchy viewer.
	void 	evaluateJavascript(String script, ValueCallback<String> resultCallback) // Asynchronously evaluates JavaScript in the context of the currently displayed page.
	static String findAddress(String addr) // Gets the first substring consisting of the address of a physical location.
	int 	findAll(String find) // This method was deprecated in API level 16. findAllAsync(String) is preferred.
	void 	findAllAsync(String find) // Finds all instances of find on the page and highlights them, asynchronously.
	abstract View findHierarchyView(String className, int hashCode) // Returns a View to enable grabbing screenshots from custom children returned in dumpViewHierarchyWithProperties.
	void 	findNext(boolean forward) // Highlights and scrolls to the next match found by findAllAsync(String), wrapping around page boundaries as necessary.
	void 	flingScroll(int vx, int vy) //
	AccessibilityNodeProvider 	getAccessibilityNodeProvider() // Gets the provider for managing a virtual view hierarchy rooted at this View and reported to AccessibilityServices that explore the window content.
	SslCertificate 	getCertificate() // Gets the SSL certificate for the main top-level page or null if there is no certificate (the site is not secure).
	int 	getContentHeight() // Gets the height of the HTML content.
	Bitmap 	getFavicon() // Gets the favicon for the current page.
	WebView.HitTestResult 	getHitTestResult() // Gets a HitTestResult based on the current cursor node.
	String[] 	getHttpAuthUsernamePassword(String host, String realm) // Retrieves HTTP authentication credentials for a given host and realm.
	String 	getOriginalUrl() // Gets the original URL for the current page.
	int 	getProgress() // Gets the progress for the current page.
	WebSettings getSettings() // Gets the WebSettings object used to control the settings for this WebView.
	String 	getTitle() // Gets the title for the current page.
	String 	getUrl() // Gets the URL for the current page.
	void 	invokeZoomPicker() // Invokes the graphical zoom picker widget for this WebView.
	boolean isPrivateBrowsingEnabled() // Gets whether private browsing is enabled in this WebView.
	InputConnection onCreateInputConnection(EditorInfo outAttrs) // Create a new InputConnection for an InputMethod to interact with the view.
	boolean onGenericMotionEvent(MotionEvent event) // Implement this method to handle generic motion events.
	void 	onGlobalFocusChanged(View oldFocus, View newFocus) // This method was deprecated in API level 3. WebView should not have implemented ViewTreeObserver.OnGlobalFocusChangeListener. This method does nothing now.
	boolean onHoverEvent(MotionEvent event) // Implement this method to handle hover events.
	void 	onInitializeAccessibilityEvent(AccessibilityEvent event) //	Initializes an AccessibilityEvent with information about this View which is the event source.
	void 	onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) // Initializes an AccessibilityNodeInfo with information about this view.
	boolean onKeyDown(int keyCode, KeyEvent event) // Default implementation of KeyEvent.Callback.onKeyDown(): perform press of the view when KEYCODE_DPAD_CENTER or KEYCODE_ENTER is released, if the view is enabled and clickable.
	boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) // Default implementation of KeyEvent.Callback.onKeyMultiple(): always returns false (doesn't handle the event).
	boolean onKeyUp(int keyCode, KeyEvent event) // Default implementation of KeyEvent.Callback.onKeyUp(): perform clicking of the view when KEYCODE_DPAD_CENTER or KEYCODE_ENTER is released.
	void 	onPause() // Pauses any extra processing associated with this WebView and its associated DOM, plugins, JavaScript etc.
	void 	onResume() // Resumes a WebView after a previous call to onPause().
	boolean onTouchEvent(MotionEvent event) // Implement this method to handle touch screen motion events.
	boolean onTrackballEvent(MotionEvent event) // Implement this method to handle trackball motion events.
	void 	onWindowFocusChanged(boolean hasWindowFocus) // Called when the window containing this view gains or loses focus.
	boolean overlayHorizontalScrollbar() // Gets whether horizontal scrollbar has overlay style.
	boolean overlayVerticalScrollbar() // Gets whether vertical scrollbar has overlay style.
	boolean pageDown(boolean bottom) // Scrolls the contents of this WebView down by half the page size.
	boolean pageUp(boolean top) // Scrolls the contents of this WebView up by half the view size.
	void 	pauseTimers() // Pauses all layout, parsing, and JavaScript timers for all WebViews.
	boolean performAccessibilityAction(int action, Bundle arguments) // Performs the specified accessibility action on the view.
	boolean performLongClick() // Call this view's OnLongClickListener, if it is defined.
	void 	postUrl(String url, byte[] postData) // Loads the URL with postData using "POST" method into this WebView.
	void 	reload() // Reloads the current URL.
	void 	removeJavascriptInterface(String name) // Removes a previously injected Java object from this WebView.
	boolean requestChildRectangleOnScreen(View child, Rect rect, boolean immediate) // Called when a child of this group wants a particular rectangle to be positioned onto the screen.
	boolean requestFocus(int direction, Rect previouslyFocusedRect) // Call this to try to give focus to a specific view or to one of its descendants and give it hints about the direction and a specific rectangle that the focus is coming from. Looks for a view to give focus to respecting the setting specified by getDescendantFocusability().
	void 	requestFocusNodeHref(Message hrefMsg) // Requests the anchor or image element URL at the last tapped point.
	void 	requestImageRef(Message msg) // Requests the URL of the image last touched by the user.
	WebBackForwardList 	restoreState(Bundle inState) // Restores the state of this WebView from the given Bundle.
	void 	resumeTimers() // Resumes all layout, parsing, and JavaScript timers for all WebViews.
	void 	savePassword(String host, String username, String password) // This method was deprecated in API level 18. Saving passwords in WebView will not be supported in future versions.
	WebBackForwardList 	saveState(Bundle outState) // Saves the state of this WebView used in onSaveInstanceState(Bundle).
	void 	saveWebArchive(String filename) // Saves the current view as a web archive.
	void 	saveWebArchive(String basename, boolean autoname, ValueCallback<String> callback) // Saves the current view as a web archive.
	void 	setBackgroundColor(int color) // Sets the background color for this view.
	void 	setCertificate(SslCertificate certificate) // This method was deprecated in API level 17. Calling this function has no useful effect, and will be ignored in future releases.
	void 	setDownloadListener(DownloadListener listener) // Registers the interface to be used when content can not be handled by the rendering engine, and should be downloaded instead.
	void 	setFindListener(WebView.FindListener listener) // Registers the listener to be notified as find-on-page operations progress.
	void 	setHorizontalScrollbarOverlay(boolean overlay) // Specifies whether the horizontal scrollbar has overlay style.
	void 	setHttpAuthUsernamePassword(String host, String realm, String username, String password) // Stores HTTP authentication credentials for a given host and realm.
	void 	setInitialScale(int scaleInPercent) // Sets the initial scale for this WebView.
	void 	setLayerType(int layerType, Paint paint) // Specifies the type of layer backing this view.
	void 	setLayoutParams(ViewGroup.LayoutParams params) // Set the layout parameters associated with this view.
	void 	setMapTrackballToArrowKeys(boolean setMap) // This method was deprecated in API level 17. Only the default case, true, will be supported in a future version.
	void 	setNetworkAvailable(boolean networkUp) // Informs WebView of the network state.
	void 	setOverScrollMode(int mode) // Set the over-scroll mode for this view.
	void 	setScrollBarStyle(int style) // Specify the style of the scrollbars.
	void 	setVerticalScrollbarOverlay(boolean overlay) // Specifies whether the vertical scrollbar has overlay style.
	void 	setWebChromeClient(WebChromeClient client) // Sets the chrome handler.
	static void setWebContentsDebuggingEnabled(boolean enabled) // Enables debugging of web contents (HTML / CSS / JavaScript) loaded into any WebViews of this application.
	void 	setWebViewClient(WebViewClient client) // Sets the WebViewClient that will receive various notifications and requests.
	boolean shouldDelayChildPressedState() // Return true if the pressed state should be delayed for children or descendants of this ViewGroup.
	boolean showFindDialog(String text, boolean showIme) // This method was deprecated in API level 18. This method does not work reliably on all Android versions; implementing a custom find dialog using WebView.findAllAsync() provides a more robust solution.
	void 	stopLoading() // Stops the current load.
	boolean zoomIn() // Performs zoom in in this WebView.
	boolean zoomOut() // Performs zoom out in this WebView.
	*/

	/*
	Unimplemented WebChromeClient callbacks:

	Bitmap 	getDefaultVideoPoster() // When not playing, video elements are represented by a 'poster' image.
	View 	getVideoLoadingProgressView() // When the user starts to playback a video element, it may take time for enough data to be buffered before the first frames can be rendered.
	void 	getVisitedHistory(ValueCallback<String[]> callback) // Obtains a list of all visited history items, used for link coloring
	void 	onCloseWindow(WebView window) // Notify the host application to close the given WebView and remove it from the view system if necessary.
	boolean onConsoleMessage(ConsoleMessage consoleMessage) // Report a JavaScript console message to the host application.
	boolean onCreateWindow(WebView view, boolean isDialog, boolean isUserGesture, Message resultMsg) // Request the host application to create a new window.
	void 	onExceededDatabaseQuota(String url, String databaseIdentifier, long quota, long estimatedDatabaseSize, long totalQuota, WebStorage.QuotaUpdater quotaUpdater) // This method was deprecated in API level 19. This method is no longer called; WebView now uses the HTML5 / JavaScript Quota Management API.
	void 	onGeolocationPermissionsHidePrompt() // Notify the host application that a request for Geolocation permissions, made with a previous call to onGeolocationPermissionsShowPrompt() has been canceled.
	void 	onGeolocationPermissionsShowPrompt(String origin, GeolocationPermissions.Callback callback) // Notify the host application that web content from the specified origin is attempting to use the Geolocation API, but no permission state is currently set for that origin.
	void 	onHideCustomView() // Notify the host application that the current page would like to hide its custom view.
	boolean onJsAlert(WebView view, String url, String message, JsResult result) // Tell the client to display a javascript alert dialog.
	boolean onJsBeforeUnload(WebView view, String url, String message, JsResult result) // Tell the client to display a dialog to confirm navigation away from the current page.
	boolean onJsConfirm(WebView view, String url, String message, JsResult result) // Tell the client to display a confirm dialog to the user.
	boolean onJsPrompt(WebView view, String url, String message, String defaultValue, JsPromptResult result) // Tell the client to display a prompt dialog to the user.
	boolean onJsTimeout() // This method was deprecated in API level 17. This method is no longer supported and will not be invoked.
	void 	onReceivedIcon(WebView view, Bitmap icon) // Notify the host application of a new favicon for the current page.
	void 	onReceivedTitle(WebView view, String title) // Notify the host application of a change in the document title.
	void 	onReceivedTouchIconUrl(WebView view, String url, boolean precomposed) // Notify the host application of the url for an apple-touch-icon.
	void 	onRequestFocus(WebView view) // Request display and focus for this WebView.
	void 	onShowCustomView(View view, int requestedOrientation, WebChromeClient.CustomViewCallback callback) // This method was deprecated in API level 18. This method supports the obsolete plugin mechanism, and will not be invoked in future
	void 	onShowCustomView(View view, WebChromeClient.CustomViewCallback callback) // Notify the host application that the current page would like to show a custom View.
	*/

signals:
	void pageStarted();
	void pageStarted(const QString & url);
	void pageFinished();
	void pageFinished(const QString & url);
	void receivedError(int errorCode, const QString & description, const QString & failingUrl);
	void receivedSslError(int primaryError, const QString & failingUrl);
	void contentHeightReceived(int height);
	void canGoBackReceived(bool can);
	void canGoForwardReceived(bool can);
	void canGoBackOrForwardReceived(bool can, int steps);

	void progressChanged(int percent);

protected:
	//
	// WebViewClient functions.
	// Please note that all these functions are called in Android UI thread, so they can
	// safely call WebView's functions.
	//
	virtual void doUpdateVisitedHistory(JNIEnv *, jobject, jobject url, jboolean isReload);
	virtual void onFormResubmission(JNIEnv *, jobject, jobject dontResend, jobject resend);
	virtual void onLoadResource(JNIEnv *, jobject, jobject url);
	virtual void onPageFinished(JNIEnv *, jobject, jobject url);
	virtual void onPageStarted(JNIEnv *, jobject, jobject url, jobject favicon);
	virtual void onReceivedError(JNIEnv *, jobject, int errorCode, jobject description, jobject failingUrl);
	virtual void onReceivedHttpAuthRequest(JNIEnv *, jobject, jobject handler, jobject host, jobject realm);
	virtual void onReceivedLoginRequest(JNIEnv *, jobject, jobject realm, jobject account, jobject args);
	virtual void onReceivedSslError(JNIEnv *, jobject, jobject handler, jobject error);
	virtual void onScaleChanged(JNIEnv *, jobject, float oldScale, float newScale);
	virtual void onTooManyRedirects(JNIEnv *, jobject, jobject cancelMsg, jobject continueMsg);
	virtual void onUnhandledKeyEvent(JNIEnv *, jobject, jobject event);
	virtual jobject shouldInterceptRequest(JNIEnv *, jobject, jobject url);
	virtual jboolean shouldOverrideKeyEvent(JNIEnv *, jobject, jobject event);
	virtual jboolean shouldOverrideUrlLoading(JNIEnv *, jobject, jobject url);

	//
	// WebViewClient JNI wrappers
	//
	friend Q_DECL_EXPORT void JNICALL Java_doUpdateVisitedHistory(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jboolean isReload);
	friend Q_DECL_EXPORT void JNICALL Java_onFormResubmission(JNIEnv * env, jobject jo, jlong nativeptr, jobject dontResend, jobject resend);
	friend Q_DECL_EXPORT void JNICALL Java_onLoadResource(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT void JNICALL Java_onPageFinished(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT void JNICALL Java_onPageStarted(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jobject favicon);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedError(JNIEnv * env, jobject jo, jlong nativeptr, jint errorCode, jobject description, jobject failingUrl);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedHttpAuthRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject host, jobject realm);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedLoginRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject realm, jobject account, jobject args);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedSslError(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject error);
	friend Q_DECL_EXPORT void JNICALL Java_onScaleChanged(JNIEnv * env, jobject jo, jlong nativeptr, jfloat oldScale, jfloat newScale);
	friend Q_DECL_EXPORT void JNICALL Java_onTooManyRedirects(JNIEnv * env, jobject jo, jlong nativeptr, jobject cancelMsg, jobject continueMsg);
	friend Q_DECL_EXPORT void JNICALL Java_onUnhandledKeyEvent(JNIEnv * env, jobject j, jlong nativeptr, jobject event);
	friend Q_DECL_EXPORT jobject JNICALL Java_shouldInterceptRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideKeyEvent(JNIEnv * env, jobject jo, jlong nativeptr, jobject event);
	friend Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideUrlLoading(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);


	//
	// WebChromiumClient functions.
	//
	virtual void onProgressChanged(JNIEnv *, jobject, jobject webview, jint newProgress);

	//
	// WebChromiumClient JNI wrappers.
	//
	friend Q_DECL_EXPORT void JNICALL Java_onProgressChanged(JNIEnv * env, jobject j, jlong nativeptr, jobject webview, jint newProgress);


	//
	// Own callbacks
	//
	virtual void onContentHeightReceived(int height);
	virtual void onCanGoBackReceived(bool can);
	virtual void onCanGoForwardReceived(bool can);
	virtual void onCanGoBackOrForwardReceived(bool can, int steps);

	friend Q_DECL_EXPORT void JNICALL Java_onContentHeightReceived(JNIEnv * env, jobject jo, jlong nativeptr, jint height);
	friend Q_DECL_EXPORT void JNICALL Java_onCanGoBackReceived(JNIEnv * env, jobject jo, jlong nativeptr, jboolean can);
	friend Q_DECL_EXPORT void JNICALL Java_onCanGoForwardReceived(JNIEnv * env, jobject jo, jlong nativeptr, jboolean can);
	friend Q_DECL_EXPORT void JNICALL Java_onCanGoBackOrForwardReceived(JNIEnv * env, jobject jo, jlong nativeptr, jboolean can, jint steps);


private:
	bool ignore_ssl_errors_;
};
