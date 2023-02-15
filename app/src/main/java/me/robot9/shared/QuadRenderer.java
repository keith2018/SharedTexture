/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

package me.robot9.shared;

import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class QuadRenderer {
    static float squareCoords[] = {
            -1, -1, -1, 1, 1, 1, -1, -1, 1, 1, 1, -1
    };
    static float textureVertices[] = {
            0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0
    };
    static float textureVerticesFlipY[] = {
            0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1
    };
    private final String vertexShaderCode = "" +
            "precision mediump float;\n" +
            "attribute vec4 vPosition;\n" +
            "attribute vec4 inputTexCoordinate;\n" +
            "varying vec4 texCoordinate;\n" +
            "void main() {\n" +
            "  gl_Position = vPosition;\n" +
            "  texCoordinate = inputTexCoordinate;\n" +
            "}";
    private final String fragmentShaderCode = "" +
            "precision mediump float;\n" +
            "varying vec4 texCoordinate;\n" +
            "uniform sampler2D s_texture;\n" +
            "void main() {\n" +
            "  gl_FragColor = texture2D(s_texture, vec2(texCoordinate.x,texCoordinate.y));\n" +
            "}";
    private FloatBuffer vertexBuffer, textureVerticesBuffer;
    private int mProgram;
    private int mPositionHandle;
    private int mTexCoordHandle;
    private int mTextureLocation;
    private int mFrameBuffer;

    public QuadRenderer() {
        this(false);
    }

    public QuadRenderer(boolean filpY) {
        // vertex
        ByteBuffer bb = ByteBuffer.allocateDirect(squareCoords.length * 4);
        bb.order(ByteOrder.nativeOrder());
        vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(squareCoords);
        vertexBuffer.position(0);

        // texture
        float[] targetTextureVertices = filpY ? textureVerticesFlipY : textureVertices;
        ByteBuffer bb2 = ByteBuffer.allocateDirect(targetTextureVertices.length * 4);
        bb2.order(ByteOrder.nativeOrder());
        textureVerticesBuffer = bb2.asFloatBuffer();
        textureVerticesBuffer.put(targetTextureVertices);
        textureVerticesBuffer.position(0);

        // shader
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexShaderCode);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentShaderCode);
        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);
        GLES20.glDeleteShader(vertexShader);
        GLES20.glDeleteShader(fragmentShader);

        GLES20.glUseProgram(mProgram);
        mTextureLocation = GLES20.glGetUniformLocation(mProgram, "s_texture");
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        mTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "inputTexCoordinate");

        // fbo
        int[] frameBuffers = new int[1];
        GLES20.glGenFramebuffers(1, frameBuffers, 0);
        mFrameBuffer = frameBuffers[0];
    }

    public void drawTexture(int inputTexture, int width, int height) {
        GLES20.glUseProgram(mProgram);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, inputTexture);
        GLES20.glUniform1i(mTextureLocation, 0);

        GLES20.glEnableVertexAttribArray(mPositionHandle);
        GLES20.glVertexAttribPointer(mPositionHandle, 2, GLES20.GL_FLOAT, false, 0, vertexBuffer);

        GLES20.glEnableVertexAttribArray(mTexCoordHandle);
        GLES20.glVertexAttribPointer(mTexCoordHandle, 2, GLES20.GL_FLOAT, false, 0, textureVerticesBuffer);

        // draw
        GLES20.glViewport(0, 0, width, height);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 6);

        GLES20.glDisableVertexAttribArray(mPositionHandle);
        GLES20.glDisableVertexAttribArray(mTexCoordHandle);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glUseProgram(0);
    }

    private int loadShader(int type, String shaderCode) {
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);
        return shader;
    }

    public void release() {
        int[] frameBuffers = new int[]{mFrameBuffer};
        GLES20.glDeleteFramebuffers(frameBuffers.length, frameBuffers, 0);
        GLES20.glDeleteProgram(mProgram);
    }
}
