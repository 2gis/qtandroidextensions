#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QAndroidOffscreenWebView.h>
#include <QAndroidOffscreenEditText.h>
#include "QQuickViews/QQuickOffscreenWebView.h"
#include "QQuickViews/QQuickOffscreenEditText.h"
#include <QtAndroidExtras>

int main(int argc, char **argv)
{
	QAndroidOffscreenWebView::preloadJavaClasses();
	QAndroidOffscreenEditText::preloadJavaClasses();

	QGuiApplication app(argc, argv);

	qmlRegisterType<QQuickAndroidOffscreenEditText>("SceneGraphRendering", 1, 0, "OffscreenEditText");
	qmlRegisterType<QQuickAndroidOffscreenWebView>("SceneGraphRendering", 1, 0, "OffscreenWebView");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
	view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

	bool result = app.exec();

	QJniEnvPtr().unloadAllClasses();

	return result;
}

