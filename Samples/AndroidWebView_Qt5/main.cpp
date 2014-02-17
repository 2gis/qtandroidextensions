#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include "QQuickViews/QQuickOffscreenWebView.h"
#include "QQuickViews/QQuickOffscreenEditText.h"

int main(int argc, char **argv)
{
	QJniClassUnloader unloader;
	QAndroidOffscreenWebView::preloadJavaClasses();
	QAndroidOffscreenEditText::preloadJavaClasses();

	QGuiApplication app(argc, argv);

	qmlRegisterType<QQuickAndroidOffscreenEditText>("SceneGraphRendering", 1, 0, "OffscreenEditText");
	qmlRegisterType<QQuickAndroidOffscreenWebView>("SceneGraphRendering", 1, 0, "OffscreenWebView");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
	view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

	return app.exec();
}

