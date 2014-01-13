#include "QAndroidOffscreenWebView.h"

QAndroidOffscreenWebView::QAndroidOffscreenWebView(const QString & object_name, const QSize & def_size, QObject * parent)
	: QAndroidOffscreenView(QLatin1String("WebView"), object_name, def_size, parent)
{
}

QAndroidOffscreenWebView::~QAndroidOffscreenWebView()
{

}

bool QAndroidOffscreenWebView::loadUrl(const QString & url)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to load URL when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		view->CallVoid("loadUrl", url);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to load URL when View is null.");
	return false;
}

bool QAndroidOffscreenWebView::loadData(const QString & text, const QString & mime)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to insert HTML when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		view->CallVoid("loadData", text, mime);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to insert URL when View is null.");
	return false;
}
