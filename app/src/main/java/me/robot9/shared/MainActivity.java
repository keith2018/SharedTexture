/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

package me.robot9.shared;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.opengl.GLES20;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.util.concurrent.atomic.AtomicBoolean;

import me.robot9.shared.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private ActivityMainBinding binding;
    private OnscreenRenderView surfaceView;

    private ServiceConnection aidlConnection;
    private IOffscreenInterface aidlInterface;

    private QuadRenderer quadRenderer;
    private SharedTexture sharedTexture;
    private int renderWidth = 0;
    private int renderHeight = 0;
    private int texId = 0;

    private final AtomicBoolean aidlReady = new AtomicBoolean(false);
    private final AtomicBoolean renderReady = new AtomicBoolean(false);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initGLSurfaceView();

        if (!SharedTexture.isAvailable()) {
            Toast.makeText(this, "shared texture not available !", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();

        if (SharedTexture.isAvailable()) {
            startOffscreenService();
        }
        surfaceView.onResume();
    }

    @Override
    protected void onStop() {
        super.onStop();

        if (SharedTexture.isAvailable()) {
            stopOffscreenService();
        }
        surfaceView.onPause();
    }

    private void initGLSurfaceView() {
        surfaceView = binding.surfaceView;
        surfaceView.setSurfaceViewCallback(new OnscreenRenderView.SurfaceViewCallback() {
            @Override
            public void onCreated(int width, int height) {
                renderWidth = width;
                renderHeight = height;
            }

            @Override
            public void onDrawFrame() {
                GLES20.glClearColor(0, 0, 0, 0);
                GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

                if (aidlReady.get()) {
                    // init render context
                    if (!renderReady.get()) {
                        createRenderResources();
                        offscreenRenderInit();
                    }

                    // draw frame
                    offscreenRenderDrawFrame();

                    // draw shared texture
                    quadRenderer.drawTexture(texId, renderWidth, renderHeight);
                }
            }

            @Override
            public void onDestroy() {
                destroyRenderResources();
            }
        });
    }

    private void createRenderResources() {
        Log.d(TAG, "createRenderResources");

        quadRenderer = new QuadRenderer();

        texId = EGLCore.createTexture(GLES20.GL_TEXTURE_2D);
        sharedTexture = new SharedTexture(renderWidth, renderHeight);
        sharedTexture.bindTexture(texId);

        renderReady.set(true);
    }

    private void destroyRenderResources() {
        Log.d(TAG, "destroyRenderResources");

        renderReady.set(false);

        if (quadRenderer != null) {
            quadRenderer.release();
            quadRenderer = null;
        }

        EGLCore.deleteTexture(texId);
        if (sharedTexture != null) {
            sharedTexture.release();
            sharedTexture = null;
        }
    }

    private void offscreenRenderInit() {
        try {
            aidlInterface.init(sharedTexture.getHardwareBuffer(), renderWidth, renderHeight);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void offscreenRenderDrawFrame() {
        try {
            ParcelFileDescriptor fence = aidlInterface.drawFrame();
            if (fence != null) {
                sharedTexture.waitFence(fence);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void offscreenRenderDestroy() {
        try {
            aidlInterface.destroy();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void startOffscreenService() {
        Log.d(TAG, "startOffscreenService");
        if (aidlConnection == null) {
            aidlConnection = new ServiceConnection() {
                @Override
                public void onServiceConnected(ComponentName name, IBinder service) {
                    Log.d(TAG, "onServiceConnected");
                    aidlInterface = IOffscreenInterface.Stub.asInterface(service);
                    aidlReady.set(true);
                }

                @Override
                public void onServiceDisconnected(ComponentName name) {
                    Log.d(TAG, "onServiceDisconnected");
                }
            };
        }

        Intent it = new Intent(this, OffscreenService.class);
        bindService(it, aidlConnection, BIND_AUTO_CREATE | BIND_IMPORTANT);
    }

    private void stopOffscreenService() {
        Log.d(TAG, "stopOffscreenService");

        aidlReady.set(false);

        if (aidlInterface != null) {
            offscreenRenderDestroy();
            aidlInterface = null;
        }

        if (aidlConnection != null) {
            unbindService(aidlConnection);
            aidlConnection = null;
        }
    }
}
