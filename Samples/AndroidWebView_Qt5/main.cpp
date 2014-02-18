#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include "QQuickViews/QQuickOffscreenWebView.h"
#include "QQuickViews/QQuickOffscreenEditText.h"

int main(int argc, char **argv)
{
	// Preload Java classes, so they can be instantiated in other threads.
	QAndroidOffscreenWebView::preloadJavaClasses();
	QAndroidOffscreenEditText::preloadJavaClasses();

	QGuiApplication app(argc, argv);
	QApplicationActivityObserver::installQApplicationEventFilter();

	qmlRegisterType<QQuickAndroidOffscreenEditText>("SceneGraphRendering", 1, 0, "OffscreenEditText");
	qmlRegisterType<QQuickAndroidOffscreenWebView>("SceneGraphRendering", 1, 0, "OffscreenWebView");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
	view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

	return app.exec();
}

