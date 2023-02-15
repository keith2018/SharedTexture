/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

package me.robot9.shared;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OnscreenRenderView extends GLSurfaceView {

    private static final String TAG = "OnscreenRenderView";

    private int renderWidth = 0;
    private int renderHeight = 0;

    private boolean firstFrame = true;
    private SurfaceViewCallback callback;

    public OnscreenRenderView(Context context) {
        super(context);
        init();
    }

    public OnscreenRenderView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        setEGLContextClientVersion(3);
        setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
                firstFrame = true;
            }

            @Override
            public void onSurfaceChanged(GL10 gl10, int width, int height) {
                renderWidth = width;
                renderHeight = height;
            }

            @Override
            public void onDrawFrame(GL10 gl10) {
                if (firstFrame) {
                    firstFrame = false;
                    if (callback != null) {
                        callback.onCreated(renderWidth, renderHeight);
                    }
                }

                if (callback != null) {
                    callback.onDrawFrame();
                }
            }
        });
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated");
        super.surfaceCreated(holder);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "surfaceDestroyed");
        if (callback != null) {
            queueEvent(new Runnable() {
                @Override
                public void run() {
                    callback.onDestroy();
                }
            });
        }
        super.surfaceDestroyed(holder);
    }

    public void setSurfaceViewCallback(SurfaceViewCallback callback) {
        this.callback = callback;
    }

    // all call back run in GLThread
    interface SurfaceViewCallback {
        void onCreated(int width, int height);

        void onDrawFrame();

        void onDestroy();
    }
}
