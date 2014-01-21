/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the DoubleGIS, LLC nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <EGL/egl.h>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSharedPointer>
#include <QtOpenGL/QGLShaderProgram>

/*!
 * This class:
 * 1. Holds OpenGL texture id, allocates and deallocates it when needed.
 * 2. Holds _supposed_ texture size. Important note: it's up to you to keep
 *    the size in sync with real texture size, as GL ES doesn't have provision
 *    to read dimensions from a texture. The correct supposed texture size is
 *    necessary for blitTexture() to work correctly.
 * 3. Paints GL texture on some GL context.
 * 4. It supports GL_TEXTURE_2D and GL_TEXTURE_EXTERNAL_OES textures.
 * 5. It is graphics-system agnostic and can be used as a helper in QWidget,
 *    QGLWidget, QGraphicsWidget, QML, and etc.
 */
class QOpenGLTextureHolder
{
public:
	/*!
	 * Allocates a texture and sets its supposed size.
	 * \note This function should be called with correct OpenGL context.
	 * \param type - GL_TEXTURE_EXTERNAL_OES or GL_TEXTURE_2D.
	 * \param size - Expected size of the texture (used by blitTexture()).
	 */
	QOpenGLTextureHolder(GLenum type, const QSize & size);
	QOpenGLTextureHolder();
	~QOpenGLTextureHolder();

	bool isAllocated() const { return texture_id_ != 0; }
	const QSize & getTextureSize() const { return texture_size_; }
	void setTextureSize(const QSize & size) { texture_size_ = size; }

	GLuint getTexture() const { return texture_id_; }
	GLenum getTextureType() const { return texture_type_; }

	/*!
	 * Set texture transformation matrix ('a' is 2D matrix and 'b' is a 2D vector).
	 * To set the coeffients from a uniform matrix represened as array 'x',
	 * do: setTransformation(x[0], x[1], x[4], x[5], x[12], x[13]).
	 * (All coefficients for Z axis are discarded).
	 * This is necessary addition to support texture transformation from Android SurfaceTexture.
	 * By default, the object is initialized with identity transformation.
	 */
	inline void setTransformation(GLfloat a11, GLfloat a12, GLfloat a21, GLfloat a22, GLfloat b1, GLfloat b2);

	/*!
	 * Draw the texture in current GL context.
	 * This function is intended to draw the texture on screen without resampling.
	 * Texture size should be set correctly for this to work.
	 * \param targetRect - output coordinates in OpenGL terms.
	 * \param sourceRect - used to calculate source rectangle in the GL texture.
	 */
	void blitTexture(const QRect & targetRect, const QRect & sourceRect, bool reverse_y = false);

	//! Allocate a texture of the current texture type.
	void allocateTexture();

	//! Set texture type and allocated a texture.
	void allocateTexture(GLenum type);

	//! Deallocate texture, if it has been allocated.
	void deallocateTexture();

	/*!
	 * This function may be called during initialization of GL to prevent shader compilation
	 * during first blitTexture() call.
	 */
	static void initializeGL();

private:
	//! Helper for blitTexture().
	void drawTexture(const QRectF & rect, const QRectF & bitmap_rect, bool reverse_y);

	//! Shader programs for drawTexture().
	static QGLShaderProgram * GetBlitProgram(GLenum target);

protected:
	GLuint texture_id_;
	GLenum texture_type_;
	QSize texture_size_;
	// Texture transformation: (v) = (A)*(b).
	GLfloat a11_, a12_, a21_, a22_, b1_, b2_;
	static QMap<GLenum, QSharedPointer<QGLShaderProgram> > blit_programs_;
private:
	Q_DISABLE_COPY(QOpenGLTextureHolder)
};

void QOpenGLTextureHolder::setTransformation(GLfloat a11, GLfloat a12, GLfloat a21, GLfloat a22, GLfloat b1, GLfloat b2)
{
	a11_ = a11;
	a12_ = a12;
	a21_ = a21;
	a22_ = a22;
	b1_ = b1;
	b2_ = b2;
}


