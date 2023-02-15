/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

package me.robot9.shared;

import android.hardware.HardwareBuffer;
import android.os.ParcelFileDescriptor;
import android.util.Log;


public class SharedTexture {

    private static final String TAG = "SharedTexture";
    private static final int EGL_NO_NATIVE_FENCE_FD_ANDROID = -1;
    private static boolean available = false;

    static {
        System.loadLibrary("shared-texture");
    }

    static {
        try {
            available = available();
        } catch (Throwable t) {
            Log.e(TAG, "SharedTexture check available error: " + t);
        }

        Log.d(TAG, "SharedTexture available: " + available);
        if (!available) {
            Log.e(TAG, "SharedTexture not available");
        }
    }

    private long nativeContext = 0;
    private HardwareBuffer buffer = null;

    public SharedTexture(HardwareBuffer buff) {
        Log.d(TAG, "create SharedTexture from buffer");
        nativeContext = createFromBuffer(buff);
        buffer = buff;
    }

    public SharedTexture(int width, int height) {
        Log.d(TAG, "create new SharedTexture:(" + width + "," + height + ")");
        nativeContext = create(width, height);
    }

    public static boolean isAvailable() {
        return available;
    }

    public HardwareBuffer getHardwareBuffer() {
        if (nativeContext != 0) {
            if (buffer != null) {
                buffer.close();
                buffer = null;
            }
            buffer = getBuffer(nativeContext);
            return buffer;
        }
        return null;
    }

    public boolean bindTexture(int texId) {
        if (nativeContext != 0) {
            return bindTexture(nativeContext, texId);
        }
        return false;
    }

    public int getBufferWidth() {
        if (nativeContext != 0) {
            return getWidth(nativeContext);
        }
        return 0;
    }

    public int getBufferHeight() {
        if (nativeContext != 0) {
            return getHeight(nativeContext);
        }
        return 0;
    }

    public int getBindTexture() {
        if (nativeContext != 0) {
            return getBindTexture(nativeContext);
        }
        return 0;
    }

    public ParcelFileDescriptor createFence() {
        int fd = createEGLFence();
        if (fd != EGL_NO_NATIVE_FENCE_FD_ANDROID) {
            return ParcelFileDescriptor.adoptFd(fd);
        }
        return null;
    }

    public boolean waitFence(ParcelFileDescriptor pfd) {
        if (pfd != null) {
            int fd = pfd.detachFd();
            if (fd != EGL_NO_NATIVE_FENCE_FD_ANDROID) {
                return waitEGLFence(fd);
            }
        }

        return false;
    }

    public void release() {
        if (nativeContext != 0) {
            Log.d(TAG, "destroy");
            if (buffer != null) {
                buffer.close();
                buffer = null;
            }
            destroy(nativeContext);
            nativeContext = 0;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        release();
    }

    private static native boolean available();

    private static native int createEGLFence();

    private static native boolean waitEGLFence(int fenceFd);

    private native long create(int width, int height);

    private native long createFromBuffer(HardwareBuffer buffer);

    private native HardwareBuffer getBuffer(long ctx);

    private native boolean bindTexture(long ctx, int texId);

    private native int getWidth(long ctx);

    private native int getHeight(long ctx);

    private native int getBindTexture(long ctx);

    private native void destroy(long ctx);
}
