#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "QOpenGLTextureHolder.h"

QMap<GLenum, QSharedPointer<QGLShaderProgram> > QOpenGLTextureHolder::blit_programs_;

QOpenGLTextureHolder::QOpenGLTextureHolder(GLenum type, const QSize & size)
	: texture_id_(0)
	, texture_type_(type)
	, texture_size_(size)
	, a11_(1.0f), a12_(0), a21_(0), a22_(1.0f), b1_(0), b2_(0)
{
	Q_ASSERT(type == GL_TEXTURE_EXTERNAL_OES || type == GL_TEXTURE_2D);
	allocateTexture();
}

QOpenGLTextureHolder::QOpenGLTextureHolder()
	: texture_id_(0)
	, texture_type_(GL_TEXTURE_2D)
	, texture_size_(64, 64)
	, a11_(1.0f), a12_(0), a21_(0), a22_(1.0f), b1_(0), b2_(0)
{
}

QOpenGLTextureHolder::~QOpenGLTextureHolder()
{
	deallocateTexture();
}

void QOpenGLTextureHolder::deallocateTexture()
{
	if (texture_id_ != 0)
	{
		glDeleteTextures(1, &texture_id_);
		texture_id_ = 0;
	}
}

void QOpenGLTextureHolder::setTransformation(GLfloat a11, GLfloat a12, GLfloat a21, GLfloat a22, GLfloat b1, GLfloat b2)
{
	a11_ = a11;
	a12_ = a12;
	a21_ = a21;
	a22_ = a22;
	b1_ = b1;
	b2_ = b2;
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

QGLShaderProgram * QOpenGLTextureHolder::GetBlitProgram(GLenum target)
{
	#if !defined(QT_OPENGL_ES_2)
		qFatal("GetBlitProgram should not be called if not using OpenGL ES 2.");
		Q_UNUSED(target);
		return 0;
	#else
		QSharedPointer<QGLShaderProgram> & blit_program_ptr = blit_programs_[target];
		if (!blit_program_ptr.isNull())
		{
			return blit_program_ptr.data();
		}

		qDebug()<<"Creating blit shaders for tetxure type"<<target;

		static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
		static const GLuint QT_TEXTURE_COORDS_ATTR = 1;
		static const QLatin1String c_for_external_("#extension GL_OES_EGL_image_external : require \n");

		QString qglslMainWithTexCoordsVertexShader =
			"attribute highp vec2 textureCoordArray; \n"
			"varying highp vec2 textureCoords; \n"
			"void setPosition(); \n"
			"void main(void) \n"
			"{ \n"
			"  setPosition(); \n"
			"  textureCoords = textureCoordArray; \n"
			"}\n";

		QString qglslUntransformedPositionVertexShader =
			"attribute highp vec4 vertexCoordsArray; \n"
			"void setPosition(void) \n"
			"{ \n"
			"  gl_Position = vertexCoordsArray; \n"
			"}\n";

		QString qglslMainFragmentShader =
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

		blit_program_ptr = QSharedPointer<QGLShaderProgram>(new QGLShaderProgram());
		{
			QString source;
			source.append(qglslMainWithTexCoordsVertexShader);
			source.append(qglslUntransformedPositionVertexShader);

			QGLShader *vertexShader = new QGLShader(QGLShader::Vertex, blit_program_ptr.data());
			vertexShader->compileSourceCode(source);

			blit_program_ptr->addShader(vertexShader);
		}

		{
			QString source;
			// Fragment shader may contain #extension directive, and it should be before any other code
			source.append(qglslImageSrcFragmentShader);
			source.append(qglslMainFragmentShader);

			QGLShader *fragmentShader = new QGLShader(QGLShader::Fragment, blit_program_ptr.data());
			fragmentShader->compileSourceCode(source);

			blit_program_ptr->addShader(fragmentShader);
		}

		blit_program_ptr->bindAttributeLocation("vertexCoordsArray", QT_VERTEX_COORDS_ATTR);
		blit_program_ptr->bindAttributeLocation("textureCoordArray", QT_TEXTURE_COORDS_ATTR);

		blit_program_ptr->link();

		return blit_program_ptr.data();
	#endif // ES 2.0
}

void QOpenGLTextureHolder::blitTexture(const QRect & targetRect, const QRect & sourceRect)
{
	if (!isAllocated())
	{
		qWarning()<<"Attempt to blit texture unallocated texture!";
		return;
	}
	if (targetRect.isEmpty() || sourceRect.isEmpty())
	{
		qWarning()<<"Attempt to blit texture with empty source or target rectangle!";
		return;
	}
#if defined(QT_OPENGL_ES_2)
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	// glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);

	QGLShaderProgram * blitProgram = GetBlitProgram(texture_type_);
	if (!blitProgram || !blitProgram->isLinked())
	{
		qWarning()<<"Shader program is not linked, can't blit the texture.";
		return;
	}
	if (!blitProgram->bind())
	{
		qWarning()<<"Failed to bind shader program, can't blit the texture.";
		return;
	}
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
	drawTexture(r, sourceRect);
	blitProgram->release();
#else
	glBindTexture(texture_type_, texture_id_);
	// (In OpenGL coordinates Y is reversed)
	const uint bottom = targetRect.height() - (sourceRect.y() + sourceRect.height());
	glCopyTexSubImage2D(target, 0, sourceRect.x(), bottom, sourceRect.x(), bottom, sourceRect.width(), sourceRect.height());
	glBindTexture(texture_type_, 0);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	#ifndef QT_OPENGL_ES
		glOrtho(0, targetRect.width(), targetRect.height(), 0, -999999, 999999);
	#else
		glOrthof(0, targetRect.width(), targetRect.height(), 0, -999999, 999999);
	#endif
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawTexture(targetRect, targetRect.size(), sourceRect);
#endif
}

void QOpenGLTextureHolder::drawTexture(const QRectF & rect, const QRectF & bitmap_rect)
{
	if (!isAllocated())
	{
		qWarning()<<"Attempt to draw unallocated texture!";
		return;
	}

	static const GLuint QT_VERTEX_COORDS_ATTR  = 0;
	static const GLuint QT_TEXTURE_COORDS_ATTR = 1;

	// src - source rectangle (in the texture)
	QRectF src = bitmap_rect.isEmpty()
		// Source region not specified, will use whole texture
		? QRectF(QPointF(), texture_size_)
		// Using a piece of texture
		: QRectF(
			 // (Reverse Y axis)
			 QPointF(bitmap_rect.x(), texture_size_.height() - bitmap_rect.bottom())
			 , bitmap_rect.size());

	// Convert in-texture coords to [0..1]
	qreal width = texture_size_.width();
	qreal height = texture_size_.height();
	src.setLeft(src.left() / width);
	src.setRight(src.right() / width);
	src.setTop(src.top() / height);
	src.setBottom(src.bottom() / height);

	GLfloat tx1 = src.left();
	GLfloat tx2 = src.right();
	GLfloat ty1 = src.top();
	GLfloat ty2 = src.bottom();

	GLfloat texCoordArray[4*2];

	// Apply texture transformation. This basically reverses Y axis on all
	// tested Android devices, but let's write more generic code.
	GLfloat tx1m = a11_ * tx1 + a12_ * ty1 + b1_;
	GLfloat tx2m = a11_ * tx2 + a12_ * ty2 + b1_;
	GLfloat ty1m = a21_ * tx1 + a22_ * ty1 + b2_;
	GLfloat ty2m = a21_ * tx2 + a22_ * ty2 + b2_;
	texCoordArray[0] = tx1m;
	texCoordArray[1] = ty2m;
	texCoordArray[2] = tx2m;
	texCoordArray[3] = ty2m;
	texCoordArray[4] = tx2m;
	texCoordArray[5] = ty1m;
	texCoordArray[6] = tx1m;
	texCoordArray[7] = ty1m;

	GLfloat vertexArray[4*2];
	QRectFToVertexArray(rect, vertexArray);

#if !defined(QT_OPENGL_ES_2)
	glVertexPointer(2, GL_FLOAT, 0, vertexArray);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

	glBindTexture(texture_type_, texture_id_);
	glEnable(texture_type_);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(texture_type_);
	glBindTexture(texture_type_, 0);
#else
	glVertexAttribPointer(QT_VERTEX_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, vertexArray);
	glVertexAttribPointer(QT_TEXTURE_COORDS_ATTR, 2, GL_FLOAT, GL_FALSE, 0, texCoordArray);

	glBindTexture(texture_type_, texture_id_);

	glEnableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
	glEnableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(QT_VERTEX_COORDS_ATTR);
	glDisableVertexAttribArray(QT_TEXTURE_COORDS_ATTR);
	glBindTexture(texture_type_, 0);
#endif
}

void QOpenGLTextureHolder::allocateTexture()
{
	deallocateTexture();

	glGenTextures(1, &texture_id_);
	glBindTexture(texture_type_, texture_id_);
	glTexParameteri(texture_type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(texture_type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(texture_type_, 0);
}

void QOpenGLTextureHolder::allocateTexture(GLenum type)
{
	Q_ASSERT(type == GL_TEXTURE_EXTERNAL_OES || type == GL_TEXTURE_2D);
	texture_type_ = type;
	allocateTexture();
}


// Transformation

