/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

#include "SharedTexture.h"
#include "JniLog.h"

namespace robot9 {

class SharedTextureContext {
 public:
  std::shared_ptr<SharedTexture> buffer;
};

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_robot9_shared_SharedTexture_available(JNIEnv *env, jclass clazz) {
  return robot9::SharedTexture::available() ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_me_robot9_shared_SharedTexture_create(JNIEnv *env, jobject /* this */,
                                           jint width, jint height) {
  auto SharedTexture = robot9::SharedTexture::Make(width, height);
  if (SharedTexture) {
    auto *ctx = new robot9::SharedTextureContext();
    ctx->buffer = std::move(SharedTexture);
    return reinterpret_cast<jlong>(ctx);
  }
  LOGE("create SharedTexture failed");
  return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_me_robot9_shared_SharedTexture_createFromBuffer(JNIEnv *env, jobject /* this */,
                                                     jobject buffer) {
  auto SharedTexture = robot9::SharedTexture::MakeAdoptedJObject(env, buffer);
  if (SharedTexture) {
    auto *ctx = new robot9::SharedTextureContext();
    ctx->buffer = std::move(SharedTexture);
    return reinterpret_cast<jlong>(ctx);
  }
  LOGE("create SharedTexture failed");
  return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_me_robot9_shared_SharedTexture_getBuffer(JNIEnv *env, jobject /* this */,
                                              jlong ctx) {
  if (ctx == 0) {
    return nullptr;
  }

  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  if (!_ctx->buffer) {
    return nullptr;
  }
  return _ctx->buffer->getBufferJObject(env);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_robot9_shared_SharedTexture_bindTexture(JNIEnv *env, jobject /* this */,
                                                jlong ctx, jint texId) {
  if (ctx == 0) {
    return JNI_FALSE;
  }

  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  if (!_ctx->buffer) {
    return JNI_FALSE;
  }
  return _ctx->buffer->bindTexture(texId) ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_robot9_shared_SharedTexture_getWidth(JNIEnv *env, jobject /* this */,
                                             jlong ctx) {
  if (ctx == 0) {
    return 0;
  }

  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  if (!_ctx->buffer) {
    return 0;
  }
  return _ctx->buffer->getWidth();
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_robot9_shared_SharedTexture_getHeight(JNIEnv *env, jobject /* this */,
                                              jlong ctx) {
  if (ctx == 0) {
    return 0;
  }

  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  if (!_ctx->buffer) {
    return 0;
  }
  return _ctx->buffer->getHeight();
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_robot9_shared_SharedTexture_getBindTexture(JNIEnv *env, jobject /* this */,
                                                   jlong ctx) {
  if (ctx == 0) {
    return 0;
  }

  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  if (!_ctx->buffer) {
    return 0;
  }
  return static_cast<jint>(_ctx->buffer->getBindTexture());
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_robot9_shared_SharedTexture_createEGLFence(JNIEnv *env, jclass clazz) {
  return robot9::SharedTexture::createEGLFence();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_robot9_shared_SharedTexture_waitEGLFence(JNIEnv *env, jclass clazz,
                                                 jint fenceFd) {
  return robot9::SharedTexture::waitEGLFence(fenceFd) ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_me_robot9_shared_SharedTexture_destroy(JNIEnv *env, jobject /* this */,
                                            jlong ctx) {
  if (ctx == 0) {
    return;
  }
  auto *_ctx = reinterpret_cast<robot9::SharedTextureContext *>(ctx);
  delete _ctx;
}
