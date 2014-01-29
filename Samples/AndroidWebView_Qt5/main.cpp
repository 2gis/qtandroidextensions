#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QAndroidOffscreenWebView.h>
#include <QAndroidOffscreenEditText.h>
#include "QQuickAndroidOffscreenView.h"
#include <QtAndroidExtras>

int main(int argc, char **argv)
{
	QAndroidOffscreenWebView::preloadJavaClass();
	QAndroidOffscreenEditText::preloadJavaClass();

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

