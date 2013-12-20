#include <QtOpenGL>
#include <QGLWidget>
#include "GrymQtAndroidViewGraphicsProxy.h"

GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, texture_id_(0)
	, texture_available_(false)
{
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
	destroyTexture();
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_ASSERT(painter->paintEngine()->type() == QPaintEngine::OpenGL2);

	// NB: это обязательно, если делаем glClear
	#define SCISSOR_WORKAROUND

	// SGEXP
	painter->fillRect(rect(), Qt::green);

	painter->beginNativePainting();

	QPaintDevice *device = painter->device();

	QTransform combined_transform = painter->combinedTransform();

	Q_ASSERT(
		(combined_transform.type() == QTransform::TxNone) ||
		(combined_transform.type() == QTransform::TxTranslate) ||
		(combined_transform.type() == QTransform::TxScale));

	QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

	GLint l = static_cast<GLint>(widget_geometry.left());
	GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom()));
	GLsizei w = static_cast<GLsizei>(widget_geometry.width());
	GLsizei h = static_cast<GLsizei>(widget_geometry.height());

	QPoint viewpos = this->scene()->views().at(0)->pos();
	Q_UNUSED(viewpos);

#if 0

	qDebug()
			<<"widget_geometry:"<<widget_geometry.left()<<widget_geometry.top()
			<<"("<<widget_geometry.width()<<"x"<<widget_geometry.height()<<")"
			<<"right"<<widget_geometry.right()<<"bottom"<<widget_geometry.bottom()
			<<"//////// l"<<l<<"b"<<b<<"w"<<w<<"h"<<h
			<<"widget_pos"<<widget->pos().x()<<widget->pos().y()
			<<"viewpos"<<viewpos.x()<<viewpos.y();
#endif

	// SGEXP
	/*l += widget->pos().x();
	b -= widget->pos().y();
	l += widget_geometry.left();
	b -= viewpos.y();*/

	glViewport(l, b, w, h);

	#if defined(SCISSOR_WORKAROUND)
		glEnable(GL_SCISSOR_TEST);
		glScissor(l, b, w, h);
	#endif

	doGLPainting(l, b, w, h);

	#if defined(SCISSOR_WORKAROUND)
		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);
	#endif

	// Возвращаем viewport в исходное состояние
	glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	painter->endNativePainting();
}

static inline void QRectFToVertexArray(const QRectF &r, GLfloat *array)
{
	qreal left = r.left();
	qreal right = r.right();
	qreal top = r.top();
	qreal bottom = r.bottom();
	array[0] = left;
	array[1] = top;
	array[2] = right;
	array[3] = top;
	array[4] = right;
	array[5] = bottom;
	array[6] = left;
	array[7] = bottom;
}

static void drawTexture(const QRectF &rect, GLuint tex_id, const QSize &texSize, const QRectF &bitmap_rect = QRectF())
{
	static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
	static const GLuint QT_TEXTURE_COORDS_ATTR = 1;
	static const GLenum target = GL_TEXTURE_2D;

	// src - исходный прямоугольник (из текстуры)
	QRectF src = bitmap_rect.isEmpty()
		// Исходный регион не задан - используем всю текстуру
		? QRectF(QPointF(), texSize)
		// Исходный регион не задан - используем кусочек текстуры.
		: QRectF(
			 // Поскольку в OpenGL ось Y снизу вверх, то перевернём.
			 QPointF(bitmap_rect.x(), texSize.height() - bitmap_rect.bottom())
			 , bitmap_rect.size());

	if (target == GL_TEXTURE_2D)
	{
		// Конвертируем координаты исходного прямоугольника из текстуры в [0..1]
		qreal width = texSize.width();
		qreal height = texSize.height();
		src.setLeft(src.left() / width);
		src.setRight(src.right() / width);
		src.setTop(src.top() / height);
		src.setBottom(src.bottom() / height);
	}

	const GLfloat tx1 = src.left();
	const GLfloat tx2 = src.right();
	const GLfloat ty1 = src.top();
	const GLfloat ty2 = src.bottom();

	GLfloat texCoordArray[4*2] =
	{
		tx1, ty2, tx2, ty2, tx2, ty1, tx1, ty1
	};

	// Запихнём координаты выхлопного прямоугольника в vertexArray
	GLfloat vertexArray[4*2];
	QRectFToVertexArray(rect, vertexArray);

#if !defined(QT_OPENGL_ES_2)
	glVertexPointer(2, GL_FLOAT, 0, vertexArray);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

	glBindTexture(target, tex_id);
	glEnable(target);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(target);
	glBindTexture(target, 0);
#else
	// Установим координаты выхлопных вершин и соответствующие им координаты в исходной текстуре
	glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexArray);
	glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, texCoordArray);

	// Выберем текстуру
	glBindTexture(target, tex_id);

	glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
	glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);
	// Нарисуем вот это всё
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Отвалим
	glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
	glDisableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);
	glBindTexture(target, 0);
#endif
}

static QGLShaderProgram * GrymBlitProgram()
{
	static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
	static const GLuint QT_TEXTURE_COORDS_ATTR = 1;

	static const char * const qglslMainWithTexCoordsVertexShader =
		"attribute highp   vec2      textureCoordArray; \n"
		"varying   highp   vec2      textureCoords; \n"
		"void setPosition(); \n"
		"void main(void) \n"
		"{ \n"
		"  setPosition(); \n"
		"  textureCoords = textureCoordArray; \n"
		"}\n";

	static const char* const qglslUntransformedPositionVertexShader =
		"attribute highp   vec4      vertexCoordsArray; \n"
		"void setPosition(void) \n"
		"{ \n"
		"  gl_Position = vertexCoordsArray; \n"
		"}\n";

	static const char* const qglslMainFragmentShader =
		"lowp vec4 srcPixel(); \n"
		"void main() \n"
		"{ \n"
		"  gl_FragColor = srcPixel(); \n"
		"}\n";

	static const char* const qglslImageSrcFragmentShader =
		"varying   highp   vec2      textureCoords; \n"
		"uniform           sampler2D imageTexture; \n"
		"lowp vec4 srcPixel() \n"
		"{ \n"
		"  return texture2D(imageTexture, textureCoords); \n"
		"}\n";

	static QGLShaderProgram * m_blitProgram = 0;
	if (m_blitProgram == 0)
	{
		m_blitProgram = new QGLShaderProgram();
		{
			QString source;
			source.append(QLatin1String(qglslMainWithTexCoordsVertexShader));
			source.append(QLatin1String(qglslUntransformedPositionVertexShader));

			QGLShader *vertexShader = new QGLShader(QGLShader::Vertex, m_blitProgram);
			vertexShader->compileSourceCode(source);

			m_blitProgram->addShader(vertexShader);
		}

		{
			QString source;
			source.append(QLatin1String(qglslMainFragmentShader));
			source.append(QLatin1String(qglslImageSrcFragmentShader));

			QGLShader *fragmentShader = new QGLShader(QGLShader::Fragment, m_blitProgram);
			fragmentShader->compileSourceCode(source);

			m_blitProgram->addShader(fragmentShader);
		}

		m_blitProgram->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
		m_blitProgram->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);

		m_blitProgram->link();
	}
	return m_blitProgram;
}


/*!
	\param texSize - размер текстуры
	\param targetRect - прямоугольник в координатах GL-контекста
	\todo Пока считаем, что размер вьюпорта равен размеру таргетректа, на понятно, нафига вот это всё?
*/
static void blitTexture(GLuint texture, const QSize &viewport, const QSize &texSize, const QRect &targetRect, const QRect &sourceRect)
{
	#if defined(QT_OPENGL_ES_2)
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);
		// glDisable(GL_SCISSOR_TEST); // WTF
		glDisable(GL_BLEND);

		// glViewport(0, 0, viewport.width(), viewport.height());

		QGLShaderProgram *blitProgram =	GrymBlitProgram();
		blitProgram->bind();
		blitProgram->setUniformValue("imageTexture", 0 /*QT_IMAGE_TEXTURE_UNIT*/);

		// The shader manager's blit program does not multiply the
		// vertices by the pmv matrix, so we need to do the effect
		// of the orthographic projection here ourselves.
		QRectF r;
		qreal w = viewport.width();
		qreal h = viewport.height();
		r.setLeft((targetRect.left() / w) * 2.0f - 1.0f);
		if (targetRect.right() == (viewport.width() - 1))
		{
			r.setRight(1.0f);
		}
		else
		{
			r.setRight((targetRect.right() / w) * 2.0f - 1.0f);
		}
		r.setBottom((targetRect.top() / h) * 2.0f - 1.0f);
		if (targetRect.bottom() == (viewport.height() - 1))
		{
			r.setTop(1.0f);
		}
		else
		{
			r.setTop((targetRect.bottom() / w) * 2.0f - 1.0f);
		}

		drawTexture(r, texture, texSize, sourceRect);
	#else
		// Не-GL ES реализация, используем не шейдеры

		// Возьмём текстуру и сделаем ей glCopyTexSubImage2D. Поскольку ось Y в OpenGL перевёрнутая,
		// то нужно вычислить правильный bottom.
		glBindTexture(GL_TEXTURE_2D, texture);
		const uint bottom = targetRect.height() - (sourceRect.y() + sourceRect.height());
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, sourceRect.x(), bottom, sourceRect.x(), bottom, sourceRect.width(), sourceRect.height());
		glBindTexture(GL_TEXTURE_2D, 0);

		// Включим правильный набор трансформаций
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); // Интересно, нафига это второй раз?
		#ifndef QT_OPENGL_ES
			glOrtho(0, targetRect.width(), targetRect.height(), 0, -999999, 999999);
		#else
			glOrthof(0, targetRect.width(), targetRect.height(), 0, -999999, 999999);
		#endif

		// Спозиционируем view port на нужный регион
		// glViewport(0, 0, viewport.width(), viewport.height());

		// На этот цвет умножится текстура
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// Готово!
		drawTexture(targetRect, texture, targetRect.size(), sourceRect);
	#endif
}

void GrymQtAndroidViewGraphicsProxy::doGLPainting(int x, int y, int w, int h)
{
	// verbose qDebug()<<__PRETTY_FUNCTION__;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!texture_available_)
	{
		initTexture();
	}

	// (GLuint texture, const QSize &viewport, const QSize &texSize, const QRect &targetRect, const QRect &sourceRect)
	QSize draw_size = getDrawableSize();

	// Пока считаем, что w, h у нас совпадают с размерами виджета, значит, в getDrawableSize всё учтено
	Q_UNUSED(w);
	Q_UNUSED(h);
	// Прямоугольник в координатах OpenGL
	QRect target_rect(
		x
		, y + h - draw_size.height()
		, draw_size.width()
		, draw_size.height());

	// verbose qDebug()<<"Mapped to parent:"<<target_rect.left()<<","<<target_rect.top()<<"("<<target_rect.width()<<","<<target_rect.height()<<")";

	glViewport(target_rect.x(), target_rect.y(), target_rect.width(), target_rect.height());

	blitTexture(
		texture_id_ // texture
		, draw_size // viewport
		, texture_size_ // texture size
		, QRect(QPoint(0, 0), draw_size) // target rect
		, QRect(QPoint(0, 0), draw_size)); // source rect
}

static void GrymCreateTestTexture(GLuint * texture_id_, QSize * texture_size_)
{
	qDebug()<<__PRETTY_FUNCTION__;
	QImage img;
	if (!img.load(":/images/kotik.png"))
	{
		qFatal("ERROR LOADING IMAGE");
	}

	QImage GL_formatted_image = QGLWidget::convertToGLFormat(img);
	if (GL_formatted_image.isNull())
	{
		qFatal("GL IMAGE IS NULL");
	}

	glGenTextures(1, texture_id_);

	// Выберем эту текстуру для работы
	glBindTexture(GL_TEXTURE_2D, *texture_id_);

	// Создадим данные текстуры (собственно картинку)
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA
		, GL_formatted_image.width(), GL_formatted_image.height()
		, 0, GL_RGBA, GL_UNSIGNED_BYTE, GL_formatted_image.bits());

	// Параметры текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	*texture_size_ = GL_formatted_image.size();

	qDebug()<<"Done"<<__PRETTY_FUNCTION__<<"Texture size:"<<GL_formatted_image.width()<<"x"<<GL_formatted_image.height();
}

void GrymQtAndroidViewGraphicsProxy::initTexture()
{
	qDebug()<<__PRETTY_FUNCTION__;
	if (!texture_available_)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Have to create a texture...";
		texture_available_ = true;
		GrymCreateTestTexture(&texture_id_, &texture_size_);
	}
}

void GrymQtAndroidViewGraphicsProxy::destroyTexture()
{
	qDebug()<<__PRETTY_FUNCTION__;
	if (texture_available_)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Deleting texture...";
		glDeleteTextures( 1, &texture_id_ );
	}
	qDebug()<<"Done"<<__PRETTY_FUNCTION__;
}

QSize GrymQtAndroidViewGraphicsProxy::getDrawableSize() const
{
	QSize mysize = size().toSize();
	return QSize(qMin(mysize.width(), texture_size_.width()), qMin(mysize.height(), texture_size_.height()));
}

