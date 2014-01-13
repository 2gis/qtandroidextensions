#include "QAndroidOffscreenView.h"

class QAndroidOffscreenWebView
	: public QAndroidOffscreenView
{
	Q_OBJECT
public:
	QAndroidOffscreenWebView(const QString & object_name, const QSize & def_size, QObject * parent = 0);
	virtual ~QAndroidOffscreenWebView();

	bool loadUrl(const QString & url);
	bool loadData(const QString & text, const QString & mime = QLatin1String("text/html"));
};
