#include <QtOpenGL>
#include <EGL/egl.h>
#include "GrymQtAndroidViewGraphicsProxy.h"

GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, texture_id_(0)
{
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// painter->fillRect(rect(), Qt::green);
	painter->beginNativePainting();
	{
		QPaintDevice *device = painter->device();
		{
			QTransform combined_transform = painter->combinedTransform();

			/*BOOST_ASSERT(
				(combined_transform.type() == QTransform::TxNone) ||
				(combined_transform.type() == QTransform::TxTranslate) ||
				(combined_transform.type() == QTransform::TxScale));*/

			QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

			GLint l = static_cast<GLint>(widget_geometry.left());
			GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom()));
			GLsizei w = static_cast<GLsizei>(widget_geometry.width());
			GLsizei h = static_cast<GLsizei>(widget_geometry.height());
			glViewport(l, b, w, h);

			glEnable(GL_SCISSOR_TEST);
			glScissor(l, b, w, h);
		}

		doGLPainting();

		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);

		// Возвращаем viewport в исходное состояние
		glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	}
	painter->endNativePainting();
}

void GrymQtAndroidViewGraphicsProxy::doGLPainting()
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


