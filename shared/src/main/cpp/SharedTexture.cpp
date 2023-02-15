/*
 * SharedTexture
 * @author 	: keith@robot9.me
 *
 */

#include "SharedTexture.h"
#include "JniLog.h"

#include <dlfcn.h>
#include <mutex>
#include <unistd.h>

namespace robot9 {

namespace glext {

PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID = nullptr;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR = nullptr;
PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR = nullptr;
PFNEGLWAITSYNCKHRPROC eglWaitSyncKHR = nullptr;
PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID = nullptr;

}
using namespace glext;

std::once_flag glProcOnceFlag;
static bool initGLExtProc() noexcept {
  std::call_once(glProcOnceFlag, []() {
    glext::eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) eglGetProcAddress("eglGetNativeClientBufferANDROID");
    glext::glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    glext::eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    glext::eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
    glext::eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    glext::eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    glext::eglWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC) eglGetProcAddress("eglWaitSyncKHR");
    glext::eglDupNativeFenceFDANDROID = (PFNEGLDUPNATIVEFENCEFDANDROIDPROC) eglGetProcAddress("eglDupNativeFenceFDANDROID");
  });
  return glext::eglGetNativeClientBufferANDROID
      && glext::glEGLImageTargetTexture2DOES
      && glext::eglCreateImageKHR
      && glext::eglDestroyImageKHR
      && glext::eglCreateSyncKHR
      && glext::eglDestroySyncKHR
      && glext::eglWaitSyncKHR
      && glext::eglDupNativeFenceFDANDROID;
}

typedef int (*Func_AHardwareBuffer_allocate)(const AHardwareBuffer_Desc *, AHardwareBuffer **);
typedef void (*Func_AHardwareBuffer_release)(AHardwareBuffer *);
typedef int (*Func_AHardwareBuffer_lock)(AHardwareBuffer *buffer, uint64_t usage, int32_t fence, const ARect *rect, void **outVirtualAddress);
typedef int (*Func_AHardwareBuffer_unlock)(AHardwareBuffer *buffer, int32_t *fence);
typedef void (*Func_AHardwareBuffer_describe)(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc);
typedef void (*Func_AHardwareBuffer_acquire)(AHardwareBuffer *buffer);

typedef AHardwareBuffer *(*Func_AHardwareBuffer_fromHardwareBuffer)(JNIEnv *env, jobject hardwareBufferObj);
typedef jobject (*Func_AHardwareBuffer_toHardwareBuffer)(JNIEnv *env, AHardwareBuffer *hardwareBuffer);

class HWDriver {
 public:
  static Func_AHardwareBuffer_allocate AHardwareBuffer_allocate;
  static Func_AHardwareBuffer_release AHardwareBuffer_release;
  static Func_AHardwareBuffer_lock AHardwareBuffer_lock;
  static Func_AHardwareBuffer_unlock AHardwareBuffer_unlock;
  static Func_AHardwareBuffer_describe AHardwareBuffer_describe;
  static Func_AHardwareBuffer_acquire AHardwareBuffer_acquire;
  static Func_AHardwareBuffer_fromHardwareBuffer AHardwareBuffer_fromHardwareBuffer;
  static Func_AHardwareBuffer_toHardwareBuffer AHardwareBuffer_toHardwareBuffer;

  template<typename T>
  static void loadSymbol(T *&pfn, const char *symbol) {
    pfn = (T *) dlsym(RTLD_DEFAULT, symbol);
  }

  static bool initFunctions() noexcept {
    loadSymbol(AHardwareBuffer_allocate, "AHardwareBuffer_allocate");
    loadSymbol(AHardwareBuffer_release, "AHardwareBuffer_release");
    loadSymbol(AHardwareBuffer_lock, "AHardwareBuffer_lock");
    loadSymbol(AHardwareBuffer_unlock, "AHardwareBuffer_unlock");
    loadSymbol(AHardwareBuffer_describe, "AHardwareBuffer_describe");
    loadSymbol(AHardwareBuffer_acquire, "AHardwareBuffer_acquire");
    loadSymbol(AHardwareBuffer_fromHardwareBuffer, "AHardwareBuffer_fromHardwareBuffer");
    loadSymbol(AHardwareBuffer_toHardwareBuffer, "AHardwareBuffer_toHardwareBuffer");

    return AHardwareBuffer_allocate && AHardwareBuffer_release
        && AHardwareBuffer_lock && AHardwareBuffer_unlock
        && AHardwareBuffer_describe && AHardwareBuffer_acquire
        && AHardwareBuffer_fromHardwareBuffer && AHardwareBuffer_toHardwareBuffer;
  }
};

Func_AHardwareBuffer_allocate HWDriver::AHardwareBuffer_allocate = nullptr;
Func_AHardwareBuffer_release HWDriver::AHardwareBuffer_release = nullptr;
Func_AHardwareBuffer_lock HWDriver::AHardwareBuffer_lock = nullptr;
Func_AHardwareBuffer_unlock HWDriver::AHardwareBuffer_unlock = nullptr;
Func_AHardwareBuffer_describe HWDriver::AHardwareBuffer_describe = nullptr;
Func_AHardwareBuffer_acquire HWDriver::AHardwareBuffer_acquire = nullptr;
Func_AHardwareBuffer_fromHardwareBuffer HWDriver::AHardwareBuffer_fromHardwareBuffer = nullptr;
Func_AHardwareBuffer_toHardwareBuffer HWDriver::AHardwareBuffer_toHardwareBuffer = nullptr;

static const char *getEGLError() {
  switch (eglGetError()) {
    case EGL_SUCCESS:return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONTEXT:return "EGL_BAD_CONTEXT";
    case EGL_BAD_CONFIG:return "EGL_BAD_CONFIG";
    case EGL_BAD_CURRENT_SURFACE:return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:return "EGL_BAD_DISPLAY";
    case EGL_BAD_SURFACE:return "EGL_BAD_SURFACE";
    case EGL_BAD_MATCH:return "EGL_BAD_MATCH";
    case EGL_BAD_PARAMETER:return "EGL_BAD_PARAMETER";
    case EGL_BAD_NATIVE_PIXMAP:return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:return "EGL_BAD_NATIVE_WINDOW";
    case EGL_CONTEXT_LOST:return "EGL_CONTEXT_LOST";
    default:return "Unknown error";
  }
}

bool SharedTexture::AVAILABLE = initGLExtProc() && HWDriver::initFunctions();

bool SharedTexture::available() {
  return AVAILABLE;
}

std::shared_ptr<SharedTexture> SharedTexture::Make(int width, int height) {
  if (!AVAILABLE) {
    LOGE("Make failed: not AVAILABLE");
    return nullptr;
  }
  AHardwareBuffer *buffer = nullptr;
  AHardwareBuffer_Desc desc = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height),
      1,
      AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
      AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
          AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT,
      0,
      0,
      0};
  int errCode = HWDriver::AHardwareBuffer_allocate(&desc, &buffer);
  if (errCode != 0 || !buffer) {
    LOGE("Make failed: AHardwareBuffer_allocate error: %d", errCode);
    return nullptr;
  }
  return std::shared_ptr<SharedTexture>(new SharedTexture(buffer, static_cast<int>(desc.width), static_cast<int>(desc.height)));
}

std::shared_ptr<SharedTexture> SharedTexture::MakeAdopted(AHardwareBuffer *buffer) {
  if (!AVAILABLE) {
    LOGE("MakeAdopted failed: not AVAILABLE");
    return nullptr;
  }
  if (!buffer) {
    LOGE("MakeAdopted failed: buffer null");
    return nullptr;
  }
  AHardwareBuffer_Desc desc;
  HWDriver::AHardwareBuffer_describe(buffer, &desc);
  HWDriver::AHardwareBuffer_acquire(buffer);
  return std::shared_ptr<SharedTexture>(new SharedTexture(buffer, static_cast<int>(desc.width), static_cast<int>(desc.height)));
}

std::shared_ptr<SharedTexture> SharedTexture::MakeAdoptedJObject(JNIEnv *env, jobject buffer) {
  if (!AVAILABLE) {
    LOGE("MakeAdoptedJObject failed: not AVAILABLE");
    return nullptr;
  }
  if (!buffer) {
    LOGE("MakeAdoptedJObject failed: buffer null");
    return nullptr;
  }
  AHardwareBuffer *hwBuffer = HWDriver::AHardwareBuffer_fromHardwareBuffer(env, buffer);
  return MakeAdopted(hwBuffer);
}

SharedTexture::SharedTexture(AHardwareBuffer *buffer, int width, int height)
    : buffer_(buffer), width_(width), height_(height) {
  LOGD("SharedTexture(%d, %d)", width, height);
}

SharedTexture::~SharedTexture() {
  LOGD("~SharedTexture");
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (eglImage_ != EGL_NO_IMAGE_KHR) {
    glext::eglDestroyImageKHR(display, eglImage_);
    eglImage_ = EGL_NO_IMAGE_KHR;
  }
  if (buffer_) {
    HWDriver::AHardwareBuffer_release(buffer_);
  }
}

bool SharedTexture::bindTexture(unsigned texId) {
  if (!AVAILABLE) {
    LOGE("bindTexture failed: not AVAILABLE");
    return false;
  }

  if (!buffer_ || texId == 0) {
    LOGE("bindTexture failed: buffer_ or texId null");
    return false;
  }

  EGLClientBuffer clientBuffer = glext::eglGetNativeClientBufferANDROID(buffer_);
  if (!clientBuffer) {
    LOGE("bindTexture failed: clientBuffer null");
    return false;
  }

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (eglImage_ != EGL_NO_IMAGE_KHR) {
    glext::eglDestroyImageKHR(display, eglImage_);
  }

  EGLint eglImageAttributes[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
  eglImage_ = glext::eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                       clientBuffer, eglImageAttributes);
  if (eglImage_ == EGL_NO_IMAGE_KHR) {
    LOGE("bindTexture failed: eglCreateImageKHR null: %s", getEGLError());
    return false;
  }

  glBindTexture(GL_TEXTURE_2D, texId);
  glext::glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) eglImage_);

  bindTexId_ = texId;
  return true;
}

AHardwareBuffer *SharedTexture::getBuffer() const {
  return buffer_;
}

jobject SharedTexture::getBufferJObject(JNIEnv *env) const {
  if (!AVAILABLE) {
    LOGE("getBufferJObject null: not AVAILABLE");
    return nullptr;
  }
  if (!buffer_) {
    LOGE("getBufferJObject null: buffer_ null");
    return nullptr;
  }
  return HWDriver::AHardwareBuffer_toHardwareBuffer(env, buffer_);
}

int SharedTexture::getWidth() const {
  return width_;
}

int SharedTexture::getHeight() const {
  return height_;
}

unsigned SharedTexture::getBindTexture() const {
  return bindTexId_;
}

int SharedTexture::createEGLFence() {
  if (!AVAILABLE) {
    LOGE("createEGLFence null: not AVAILABLE");
    return EGL_NO_NATIVE_FENCE_FD_ANDROID;
  }

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLSyncKHR eglSync = glext::eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
  if (eglSync == EGL_NO_SYNC_KHR) {
    LOGE("createEGLFence null: eglCreateSyncKHR null: %s", getEGLError());
    return EGL_NO_NATIVE_FENCE_FD_ANDROID;
  }

  // need flush before wait
  glFlush();

  int fenceFd = glext::eglDupNativeFenceFDANDROID(display, eglSync);
  glext::eglDestroySyncKHR(display, eglSync);

  if (fenceFd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
    LOGE("createEGLFence null: eglDupNativeFenceFDANDROID error: %s", getEGLError());
  }

  return fenceFd;
}

bool SharedTexture::waitEGLFence(int fenceFd) {
  if (!AVAILABLE) {
    LOGE("waitEGLFence failed: not AVAILABLE");
    return false;
  }

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint attribs[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fenceFd, EGL_NONE};
  EGLSyncKHR eglSync = glext::eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, attribs);
  if (eglSync == EGL_NO_SYNC_KHR) {
    LOGE("waitEGLFence failed: eglCreateSyncKHR null: %s", getEGLError());
    close(fenceFd);
    return false;
  }

  EGLint success = glext::eglWaitSyncKHR(display, eglSync, 0);
  glext::eglDestroySyncKHR(display, eglSync);

  if (success == EGL_FALSE) {
    LOGE("waitEGLFence failed: eglWaitSyncKHR fail: %s", getEGLError());
    return false;
  }

  return true;
}

}
