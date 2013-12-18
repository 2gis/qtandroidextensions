#pragma once
#include <QtOpenGL/QGLWidget>

class GrymQtAndroidViewProxy
	: public QGLWidget
{
	Q_OBJECT
public:
	explicit GrymQtAndroidViewProxy(QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	explicit GrymQtAndroidViewProxy(QGLContext *context, QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	explicit GrymQtAndroidViewProxy(const QGLFormat& format, QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	virtual ~GrymQtAndroidViewProxy();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	virtual void paintGL();

private:
	void init();

private:
	int texture_id_;
};

