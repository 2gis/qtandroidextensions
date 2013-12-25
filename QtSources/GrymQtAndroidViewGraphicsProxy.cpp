#include <jni.h>
#include <unistd.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <QtOpenGL>
#include <QGLWidget>
#include "GrymQtAndroidViewGraphicsProxy.h"

#define ANDROIDVIEWGRAPHICSPROXY_CLEARALL
// #define ANDROIDVIEWGRAPHICSPROXY_TEST_TEXTURE
#define ANDROIDVIEWGRAPHICSPROXY_GREEN_FILL
#define ANDROIDVIEWGRAPHICSPROXY_READ_ONLY_XY_TRANSFORM

static const QString c_class_path_ = QLatin1String("ru/dublgis/offscreenview");

Q_DECL_EXPORT void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		GrymQtAndroidViewGraphicsProxy * proxy = reinterpret_cast<GrymQtAndroidViewGraphicsProxy*>(vp);
		if (proxy)
		{
			QMetaObject::invokeMethod(proxy, "javaUpdate", Qt::QueuedConnection);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, texture_id_(0)
	, texture_type_(GL_TEXTURE_2D)
	, texture_available_(false)
	, texture_transform_set_(false)
	, need_update_texture_(false)
{
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	offscreen_view_factory_.reset(new jcGeneric((c_class_path_+"/OffscreenViewFactory").toAscii(), true));

	qDebug()<<__PRETTY_FUNCTION__<<"Connected to OffscreenViewFactory.";
	qDebug()<<__PRETTY_FUNCTION__<<"Test string:"<<offscreen_view_factory_->CallStaticString("Test");

	qDebug()<<"Done"<<__PRETTY_FUNCTION__;

	memset(texture_transform_, 0, sizeof(texture_transform_));
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
	// Разрушим Java-объекты первым делом
	offscreen_view_.reset();
	offscreen_view_factory_.reset();
	destroyTexture();
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	Q_ASSERT(painter->paintEngine()->type() == QPaintEngine::OpenGL2);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		painter->fillRect(rect(), Qt::green);
	#endif

	painter->beginNativePainting();

	QPaintDevice *device = painter->device();
	QTransform combined_transform = painter->combinedTransform();

	Q_ASSERT(
		(combined_transform.type() == QTransform::TxNone) ||
		(combined_transform.type() == QTransform::TxTranslate) ||
		(combined_transform.type() == QTransform::TxScale));

	QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

	GLint l = static_cast<GLint>(widget_geometry.left() + 0.5);
	GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom() + 0.5));
	GLsizei w = static_cast<GLsizei>(widget_geometry.width()+0.5);
	GLsizei h = static_cast<GLsizei>(widget_geometry.height()+0.5);

	QPoint viewpos = this->scene()->views().at(0)->pos();
	Q_UNUSED(viewpos);

#if 1
	qDebug()
			<<__PRETTY_FUNCTION__
			<<"tid"<<gettid()
			<<"widget_geometry:"<<widget_geometry.left()<<widget_geometry.top()
			<<"("<<widget_geometry.width()<<"x"<<widget_geometry.height()<<")"
			<<"right"<<widget_geometry.right()<<"bottom"<<widget_geometry.bottom()
			<<"//////// l"<<l<<"b"<<b<<"w"<<w<<"h"<<h
			<<"widget_pos"<<widget->pos().x()<<widget->pos().y()
			<<"viewpos"<<viewpos.x()<<viewpos.y();
#endif

	// SGEXP
	l += widget->pos().x();
	b += widget->pos().y();
	//! \todo Учесть положение вьюшки внутри окна!
	w++;
	h++;

	glViewport(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glEnable(GL_SCISSOR_TEST);
		glScissor(l, b, w, h);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#endif

	initTexture();
	if (need_update_texture_)
	{
		updateTexture();
	}

	// Теперь можно просто нарисовать нашу текстурку вот в этих вот GL-координатах.
	doGLPainting(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
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

/*!
 * Рисовалка текстуры из Qt. Подразумевается, что подготовительные операции уже сделаны.
 */
void GrymQtAndroidViewGraphicsProxy::drawTexture(const QRectF &rect, GLuint tex_id, GLenum target, const QSize &texSize, const QRectF &bitmap_rect)
{
	static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
	static const GLuint QT_TEXTURE_COORDS_ATTR = 1;

	// src - исходный прямоугольник (из текстуры)
	QRectF src = bitmap_rect.isEmpty()
		// Исходный регион не задан - используем всю текстуру
		? QRectF(QPointF(), texSize)
		// Исходный регион не задан - используем кусочек текстуры.
		: QRectF(
			 // Поскольку в OpenGL ось Y снизу вверх, то перевернём.
			 QPointF(bitmap_rect.x(), texSize.height() - bitmap_rect.bottom())
			 , bitmap_rect.size());

	// Конвертируем координаты исходного прямоугольника из текстуры в [0..1]
	qreal width = texSize.width();
	qreal height = texSize.height();
	src.setLeft(src.left() / width);
	src.setRight(src.right() / width);
	src.setTop(src.top() / height);
	src.setBottom(src.bottom() / height);

	GLfloat tx1 = src.left();
	GLfloat tx2 = src.right();
	GLfloat ty1 = src.top();
	GLfloat ty2 = src.bottom();

	GLfloat texCoordArray[4*2];

	if (texture_transform_set_)
	{
		GLfloat * const A = texture_transform_;

		#if 0
			qDebug()<<__PRETTY_FUNCTION__<<"src:"<<src.left()<<src.top()<<src.right()<<src.bottom()
				   << " ////////\n"
				   <<A[0]<<A[4]<<A [8]<<A[12]<<"\n"
				   <<A[1]<<A[5]<<A [9]<<A[13]<<"\n"
				   <<A[2]<<A[6]<<A[10]<<A[14]<<"\n"
				   <<A[3]<<A[7]<<A[11]<<A[15];
		#endif
		// Применяем матрицу преобразований. Подразумеваем, что в матрице texture_transform_ нету ничего над Z.
		// Тогда матрица применяется так:
		// (0 4)(x) + (12)
		// (1 5)(y)   (13)
		GLfloat tx1m = A[0] * tx1 + A[4] * ty1 + A[12];
		GLfloat tx2m = A[0] * tx2 + A[4] * ty2 + A[12];
		GLfloat ty1m = A[1] * tx1 + A[5] * ty1 + A[13];
		GLfloat ty2m = A[1] * tx2 + A[5] * ty2 + A[13];
		#if 0
			qDebug()<<__PRETTY_FUNCTION__<<"A:\n"
				<<A[0]<<A[4]<<"\n"
				<<A[1]<<A[5]<<"\n"
				<<"b:"<<A[12]<<A[13]<<"\n"
				<<"("<<tx1m<<ty1m<<"-"<<tx2m<<ty2m<<")";
		#endif
		texCoordArray[0] = tx1m;
		texCoordArray[1] = ty2m;
		texCoordArray[2] = tx2m;
		texCoordArray[3] = ty2m;
		texCoordArray[4] = tx2m;
		texCoordArray[5] = ty1m;
		texCoordArray[6] = tx1m;
		texCoordArray[7] = ty1m;
	}
	else
	{
		texCoordArray[0] = tx1;
		texCoordArray[1] = ty2;
		texCoordArray[2] = tx2;
		texCoordArray[3] = ty2;
		texCoordArray[4] = tx2;
		texCoordArray[5] = ty1;
		texCoordArray[6] = tx1;
		texCoordArray[7] = ty1;
	}

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

#if defined(QT_OPENGL_ES_2)
/*!
 * Шейдерная рисовалка текстуры для ES 2.0.
 * \todo Сделать более человеческий синглтон? Или и так ОК?
 */
static QGLShaderProgram * CreateBlitProgram(GLenum target)
{
	static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
	static const GLuint QT_TEXTURE_COORDS_ATTR = 1;

	static const QLatin1String c_for_external_("#extension GL_OES_EGL_image_external : require \n");

	QString qglslMainWithTexCoordsVertexShader =
		QString((target == GL_TEXTURE_EXTERNAL_OES)? c_for_external_: QLatin1String("")) +
		"attribute highp vec2 textureCoordArray; \n"
		"varying highp vec2 textureCoords; \n"
		"void setPosition(); \n"
		"void main(void) \n"
		"{ \n"
		"  setPosition(); \n"
		"  textureCoords = textureCoordArray; \n"
		"}\n";

	QString qglslUntransformedPositionVertexShader =
		QString((target == GL_TEXTURE_EXTERNAL_OES)? c_for_external_: QLatin1String("")) +
		"attribute highp vec4 vertexCoordsArray; \n"
		"void setPosition(void) \n"
		"{ \n"
		"  gl_Position = vertexCoordsArray; \n"
		"}\n";

	QString qglslMainFragmentShader =
		QString((target == GL_TEXTURE_EXTERNAL_OES)? c_for_external_: QLatin1String("")) +
		"lowp vec4 srcPixel(); \n"
		"void main() \n"
		"{ \n"
		"  gl_FragColor = srcPixel(); \n"
		"}\n";

	QString qglslImageSrcFragmentShader =
		QString((target == GL_TEXTURE_EXTERNAL_OES)? c_for_external_: QLatin1String("")) +
		"varying highp vec2 textureCoords; \n" +
		QString((target == GL_TEXTURE_EXTERNAL_OES)?
			"uniform samplerExternalOES imageTexture; \n" :
			"uniform sampler2D imageTexture; \n") +
		"lowp vec4 srcPixel() \n"
		"{ \n"
		"  return texture2D(imageTexture, textureCoords); \n"
		"}\n";

	static QMap<GLenum, QSharedPointer<QGLShaderProgram> > programs;
	QSharedPointer<QGLShaderProgram> & m_blitProgram = programs[target];
	if (m_blitProgram.isNull())
	{
		m_blitProgram = QSharedPointer<QGLShaderProgram>(new QGLShaderProgram());
		{
			QString source;
			source.append(qglslMainWithTexCoordsVertexShader);
			source.append(qglslUntransformedPositionVertexShader);

			QGLShader *vertexShader = new QGLShader(QGLShader::Vertex, m_blitProgram.data());
			vertexShader->compileSourceCode(source);

			m_blitProgram->addShader(vertexShader);
		}

		{
			QString source;
			source.append(qglslMainFragmentShader);
			source.append(qglslImageSrcFragmentShader);

			QGLShader *fragmentShader = new QGLShader(QGLShader::Fragment, m_blitProgram.data());
			fragmentShader->compileSourceCode(source);

			m_blitProgram->addShader(fragmentShader);
		}

		m_blitProgram->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
		m_blitProgram->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);

		m_blitProgram->link();
	}
	return m_blitProgram.data();
}
#endif

/*!
 * Нарисуем текстуру.
 * \param texSize - размер текстуры
 * \param targetRect - прямоугольник в координатах GL-контекста
 * \todo Пока считаем, что размер вьюпорта равен размеру таргетректа, на понятно, нафига вот это всё?
*/
void GrymQtAndroidViewGraphicsProxy::blitTexture(GLuint texture, GLenum target, const QSize &texSize, const QRect &targetRect, const QRect &sourceRect)
{
	#if defined(QT_OPENGL_ES_2)
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);
		// glDisable(GL_SCISSOR_TEST);
		glDisable(GL_BLEND);

		QGLShaderProgram * blitProgram = CreateBlitProgram(target);
		blitProgram->bind();
		blitProgram->setUniformValue("imageTexture", 0 /*QT_IMAGE_TEXTURE_UNIT*/);

		// The shader manager's blit program does not multiply the
		// vertices by the pmv matrix, so we need to do the effect
		// of the orthographic projection here ourselves.
		QRectF r;
		qreal w = targetRect.width();
		qreal h = targetRect.height();
		r.setLeft((targetRect.left() / w) * 2.0f - 1.0f);
		if (targetRect.right() == (targetRect.width() - 1))
		{
			r.setRight(1.0f);
		}
		else
		{
			r.setRight((targetRect.right() / w) * 2.0f - 1.0f);
		}
		r.setBottom((targetRect.top() / h) * 2.0f - 1.0f);
		if (targetRect.bottom() == (targetRect.height() - 1))
		{
			r.setTop(1.0f);
		}
		else
		{
			r.setTop((targetRect.bottom() / w) * 2.0f - 1.0f);
		}
		drawTexture(r, texture, target, texSize, sourceRect);
	#else
		// Не-GL ES реализация, используем не шейдеры

		// Возьмём текстуру и сделаем ей glCopyTexSubImage2D. Поскольку ось Y в OpenGL перевёрнутая,
		// то нужно вычислить правильный bottom.
		glBindTexture(target, texture);
		const uint bottom = targetRect.height() - (sourceRect.y() + sourceRect.height());
		glCopyTexSubImage2D(target, 0, sourceRect.x(), bottom, sourceRect.x(), bottom, sourceRect.width(), sourceRect.height());
		glBindTexture(target, 0);

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
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // На этот цвет умножится текстура
		// Готово!
		drawTexture(targetRect, texture, target, targetRect.size(), sourceRect);
	#endif
}

void GrymQtAndroidViewGraphicsProxy::javaUpdate()
{
	qDebug()<<__PRETTY_FUNCTION__;
	need_update_texture_ = true;
	update();
}

/*!
 * \param Координаты передаются в терминах OpenGL.
 */
void GrymQtAndroidViewGraphicsProxy::doGLPainting(int x, int y, int w, int h)
{
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
	// verbose qDebug()<<"tid"<<gettid()<<"Mapped to parent:"<<target_rect.left()<<","<<target_rect.top()
	// <<"("<<target_rect.width()<<","<<target_rect.height()<<")";
	glViewport(target_rect.x(), target_rect.y(), target_rect.width(), target_rect.height());

	// Debug
	#if 0 // !defined(QT_OPENGL_ES_2)
		GLuint width = 0, height = 0;
		glBindTexture(texture_type_, texture_id_);
		glGetTexLevelParameteriv(texture_type_, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(texture_type_, 0, GL_TEXTURE_HEIGHT, &height);
		glBindTexture(texture_type_, 0);
		qDebug<<__PRETTY_FUNCTION__<<"TRUE TEXTURE SIZE:"<<width<<height;
	#endif

	blitTexture(
		texture_id_ // texture
		, texture_type_ // texture type (2D / external)
		, texture_size_ // texture size
		, QRect(QPoint(0, 0), draw_size) // target rect
		, QRect(QPoint(0, 0), draw_size)); // source rect
}

/*!
 * Тестовая текстура - с котиком.
 */
void GrymQtAndroidViewGraphicsProxy::CreateTestTexture(QSize * out_texture_size_)
{
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	need_update_texture_ = false;
	QImage img;
	if (!img.load(":/images/kotik.png"))
	{
		qFatal("Failed to load PNG image for test texture.");
	}
	QImage GL_formatted_image = QGLWidget::convertToGLFormat(img);
	if (GL_formatted_image.isNull())
	{
		qFatal("Result of conversion to GL format is null.");
	}
	// Создаём текстуру
	glGenTextures(1, &texture_id_);
	static const GLenum target = GL_TEXTURE_2D;
	texture_type_ = target;

	// Выберем эту текстуру для работы
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	// Загрузим данные текстуры (собственно картинку)
	glTexImage2D(
		target, 0, GL_RGBA
		, GL_formatted_image.width(), GL_formatted_image.height()
		, 0, GL_RGBA, GL_UNSIGNED_BYTE, GL_formatted_image.bits());
	// Параметры текстуры
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(target, 0);

	*out_texture_size_ = GL_formatted_image.size();
	qDebug()<<"Done"<<__PRETTY_FUNCTION__<<"Texture size:"<<GL_formatted_image.width()<<"x"<<GL_formatted_image.height();
}

void GrymQtAndroidViewGraphicsProxy::CreateEmptyExternalTexture()
{
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();	
	glGenTextures(1, &texture_id_);
	static const GLenum target = GL_TEXTURE_EXTERNAL_OES; // Требование SurfaceTexture
	texture_type_ = target;
	glBindTexture(target, texture_id_);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(target, 0);
	need_update_texture_ = true;
	qDebug()<<"Done"<<__PRETTY_FUNCTION__;
}

void GrymQtAndroidViewGraphicsProxy::initTexture()
{
	//qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	if (!texture_available_)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid()<<"Have to create a texture...";
		texture_available_ = true;
		texture_transform_set_ = false;

		//! \todo Клипнуть запрошенные размеры по максимальным?

		#if defined(ANDROIDVIEWGRAPHICSPROXY_TEST_TEXTURE)
			CreateTestTexture(&texture_size_);
		#else
			//! \todo задание размеров
			CreateEmptyExternalTexture();

			// Определяем максимальные размеры текстуры
			GLint maxdims[2];
			GLint maxtextsz;
			glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxdims);
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtextsz);
			int max_x = qMin(maxdims[0], maxtextsz);
			int max_y = qMin(maxdims[1], maxtextsz);
			QSize mysize = size().toSize();
			texture_size_ = QSize(qMin(max_x, mysize.width()), qMin(max_y, mysize.height()));
			// texture_size_ = QSize(512, 512);
			qDebug()<<__PRETTY_FUNCTION__<<"GL_MAX_VIEWPORT_DIMS"<<maxdims[0]<<maxdims[1]
				<<"GL_MAX_TEXTURE_SIZE"<<maxtextsz
				<<"My size:"<<mysize.width()<<"x"<<mysize.height()
				<<"Resuling size:"<<texture_size_.width()<<"x"<<texture_size_.height();

			// Мы можем создать Java-вьюху только имея текстуру, а текстуру мы можем создать
			// только в OpenGL-контексте, поэтому конструирование сделаем при первой отрисовке.
			// Если вьюха тяжёлая, то её создание можно отложить на стороне Java.
			if (offscreen_view_factory_)
			{
				//! \todo Мы задаём размер вьюшки, а не текстур!
				offscreen_view_factory_->CallParamVoid("SetTexture", "I", jint(texture_id_));
				offscreen_view_factory_->CallParamVoid("SetTextureWidth",	"I", jint(texture_size_.width()));
				offscreen_view_factory_->CallParamVoid("SetTextureHeight", "I", jint(texture_size_.height()));
				offscreen_view_factory_->CallParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));
				offscreen_view_.reset(
					offscreen_view_factory_->CallObject("DoCreateView"
					, (c_class_path_+"/OffscreenView").toAscii()));
				offscreen_view_->RegisterNativeMethod(
					"nativeUpdate"
					, "(J)V"
					, (void*)Java_OffscreenView_nativeUpdate);
			}
		#endif
	}
}

void GrymQtAndroidViewGraphicsProxy::updateTexture()
{
	if (offscreen_view_)
	{
		offscreen_view_->CallVoid("drawViewOnTexture");

		// Заберём нарисованное изображение в текстуру
		offscreen_view_->CallVoid("updateTexture");

		// Заберём матрицу преобразований текстуры
		#if defined(ANDROIDVIEWGRAPHICSPROXY_READ_ONLY_XY_TRANSFORM)
			texture_transform_[0] = offscreen_view_->CallFloat("getTextureTransformMatrix", 0);
			texture_transform_[1] = offscreen_view_->CallFloat("getTextureTransformMatrix", 1);
			texture_transform_[4] = offscreen_view_->CallFloat("getTextureTransformMatrix", 4);
			texture_transform_[5] = offscreen_view_->CallFloat("getTextureTransformMatrix", 5);
			texture_transform_[12] = offscreen_view_->CallFloat("getTextureTransformMatrix", 12);
			texture_transform_[13] = offscreen_view_->CallFloat("getTextureTransformMatrix", 13);
		#else
			for (int i = 0; i < 16; ++i)
			{
				texture_transform_[i] = offscreen_view_->CallFloat("getTextureTransformMatrix", i);
			}
		#endif
		texture_transform_set_ = true;
		#if 0
			QString msg;
			for (int i = 0; i < 16; ++i)
			{
				msg += QString("%1 ").arg(texture_transform_[i]);
			}
			qDebug()<<__PRETTY_FUNCTION__<<"Transform matrix:"<<msg;
		#endif
		need_update_texture_ = false;
	}
}

void GrymQtAndroidViewGraphicsProxy::destroyTexture()
{
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	if (texture_available_)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Deleting texture...";
		glDeleteTextures(1, &texture_id_);
		texture_available_ = false;
		texture_id_ = 0;
	}
	qDebug()<<"Done"<<__PRETTY_FUNCTION__;
}

QSize GrymQtAndroidViewGraphicsProxy::getDrawableSize() const
{
	QSize mysize = size().toSize();
	return QSize(qMin(mysize.width(), texture_size_.width()), qMin(mysize.height(), texture_size_.height()));
}


