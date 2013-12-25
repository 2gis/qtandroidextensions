package ru.dublgis.gmbase;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import org.qt.util.SplashScreenPainter;
import org.qt.core.QtApplicationBase;
import org.qt.core.QtActivityBase;

public class GrymSplashPainter implements SplashScreenPainter
{
    private Context m_context = null;
    private final Paint mPaint = new Paint();
    private int last_w = 0, last_h = 0;
    private boolean mPainted = false;
    private Bitmap mSplashImage;

    public GrymSplashPainter()
    {
         Log.d("GrymMobile JAVA", "************************ Create Splash ***************************");
    }

    public GrymSplashPainter(Context ctx)
    {
        m_context = ctx;
    }

    //! Returns true if the splash screen has been rendered on screen at least once
    public boolean Painted()
    {
        return mPainted;
    }

    private int DrawableResourceId(Context my_context, final String name)
    {
        return my_context.getResources().getIdentifier(name, "drawable", my_context.getPackageName());
    }

    // Find the context to use for loading resources.
    private Context findMyContext()
    {
        // If we have a custom-set context, use it.
        // This is the way of GrymMobileActivity.
        if (m_context != null)
        {
           return m_context;
        }
        // If we don't have a context, we can look up one in QtApplicationBase.
        // This is the way of GrymMobileNativeActivity.
        QtApplicationBase qab = QtApplicationBase.getImplementationInstance();
        if (qab == null)
        {
            Log.d("GrymMobile Java", "findMyContext() called when QtApplicationBase doesn't exist.");
            return null;
        }
        QtActivityBase activity = qab.getActivity();
        if (activity == null)
        {
            Log.d("GrymMobile Java", "findMyContext() called when QtApplicationBase doesn't exist.");
            return null;
        }
        return activity;
    }

    private boolean checkBitmap(int w, int h)
    {
        try {
            // Если размер не изменился, то вернём прошлый статус
            if (w == last_w && h == last_h)
                return mSplashImage != null;

            Context ctx = findMyContext();
            if (ctx == null)
            {
                return false;
            }

            int theSize = 0, resource = 0;
            /*int min = (w<h)? w: h;
            
            if (min >= 720)
            {
                theSize = 720;
                resource = DrawableResourceId(ctx, "splash720");
            }
            else if (min >= 480)
            {
                theSize = 480;
                resource = DrawableResourceId(ctx, "splash480");
            }
            else if (min >= 320)
            {
                theSize = 320;
                resource = DrawableResourceId(ctx, "splash320");
            }
            else
            {
                theSize = 240;
                resource = DrawableResourceId(ctx, "splash240");
            }*/
            resource = DrawableResourceId(ctx, "kotik");
            theSize = 256;

            if( mSplashImage != null )
            {
                // Если у нас уже есть загруженная картинка, которая влезает
                // в заданный размер, то используем её.
                if (mSplashImage.getWidth() == theSize)
                {
                    last_w = w;
                    last_h = h;
                    return true;
                }
            }

            // Prevent image scaling - or evil BitmapFactory will resize images "for current screen density":
            BitmapFactory.Options ops = new BitmapFactory.Options();
            ops.inScaled = false;
            ops.inDensity = 0;
            ops.inTargetDensity = 0;
            ops.inScreenDensity = 0;

            mSplashImage = BitmapFactory.decodeResource(ctx.getResources(), resource, ops);
            mSplashImage.setDensity(Bitmap.DENSITY_NONE);

            if (mSplashImage==null){
                Log.e("GrymMobile JAVA", "GrymSplashPainter: ERROR: THE BITMAP IS NULL! REQUESTED SIZE: "+theSize);
                return false;
            }

            Log.d("GrymMobile JAVA", "GrymSplashPainter: loaded bitmap for "+theSize+"p @ w="+w+", h="+h);

            last_w = w;
            last_h = h;
            return true;
        } catch ( Exception e ) {
            Log.e("GrymMobile JAVA", "GrymSplashPainter: Exception: "+e);
        }
        return false;
    }

    @Override
    public void DrawSplash(Canvas canvas, int w, int h)
    {
        // Fill canvas with white.
        // Note: fill screen even if you want black because Android 1.6 has dark gray canvas by default.
        Log.d("GrymMobile JAVA", "************************ Draw Splash ***************************");
        canvas.drawRGB(255, 255, 255);
        if( !checkBitmap(w, h) )
            return;
        if (mSplashImage!=null) {
            int x = (w - mSplashImage.getWidth()) / 2;
            int y = (h - mSplashImage.getHeight()) / 2;
            /*Log.d("GrymMobile JAVA", "############################ DRAWING "+
                    mSplashImage.getWidth()+"x"+mSplashImage.getHeight()+" bitmap in "+
                    w + "x" + h + " area of "+
                    canvas.getWidth()+"x"+canvas.getHeight()+" canvas at "+x+", "+y );*/
            canvas.setDensity(Bitmap.DENSITY_NONE);
            canvas.drawBitmap(mSplashImage, x, y, mPaint);
            mPainted = true;
        }
    }
}
