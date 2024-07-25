#include "HarmonyBufferUtils.h"
#include "HarmonyOpenGLHelper.h"
#include "native_window/external_window.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include <GLES2/gl2ext.h>
#include <sys/stat.h>

namespace RNSkia {

GLTextureHelper::GLTextureHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display, GrGLuint texTarget)
    : fTexID(texID), fImage(image), fDisplay(display), fTexTarget(texTarget) {}

PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
    (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

GLTextureHelper::~GLTextureHelper() {
    glDeleteTextures(1, &fTexID);
    // eglDestroyImageKHR will remove a ref from the OH_NativeBuffer Buffer

    eglDestroyImageKHR(fDisplay, fImage);
}

void GLTextureHelper::rebind(GrDirectContext *dContext) {
    glBindTexture(fTexTarget, fTexID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture(%#x, %d) failed (%#x)", static_cast<int>(fTexTarget), static_cast<int>(fTexID),
                 static_cast<int>(status));
        return;
    }

    glEGLImageTargetTexture2DOES(fTexTarget, fImage);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", static_cast<int>(status));
        return;
    }
    dContext->resetContext(kTextureBinding_GrGLBackendState);
}

GrBackendFormat GLTextureHelper::GetGLBackendFormat(GrDirectContext *dContext, uint32_t bufferFormat,
                                                    bool requireKnownFormat) {
    GrBackendApi backend = dContext->backend();
    if (backend != GrBackendApi::kOpenGL) {
        return GrBackendFormat();
    }
    switch (bufferFormat) {
    case NATIVEBUFFER_PIXEL_FMT_RGBA_8888:
        return GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
    case NATIVEBUFFER_PIXEL_FMT_RGB_565:
        return GrBackendFormats::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_EXTERNAL);
    case NATIVEBUFFER_PIXEL_FMT_RGB_888:
        return GrBackendFormats::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_EXTERNAL);
    default:
        if (requireKnownFormat) {
            return GrBackendFormat();
        } else {
            return GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
        }
    }
    SkUNREACHABLE;
}

void delete_gl_texture(void *context) {
    GLTextureHelper *cleanupHelper = static_cast<GLTextureHelper *>(context);
    delete cleanupHelper;
}

void update_gl_texture(void *context, GrDirectContext *dContext) {
    GLTextureHelper *cleanupHelper = static_cast<GLTextureHelper *>(context);
    cleanupHelper->rebind(dContext);
}

static GrBackendTexture make_gl_backend_texture(GrDirectContext *dContext, OH_NativeBuffer *Buffer, int width,
                                                int height, DeleteImageProc *deleteProc, UpdateImageProc *updateProc,
                                                TexImageCtx *imageCtx, bool isProtectedContent,
                                                const GrBackendFormat &backendFormat, bool isRenderable) {
    EGLClientBuffer clientBuffer = OH_NativeWindow_CreateNativeWindowBufferFromNativeBuffer(Buffer);
    EGLint attribs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, isProtectedContent ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
                        isProtectedContent ? EGL_TRUE : EGL_NONE, EGL_NONE};
    EGLDisplay display = eglGetCurrentDisplay();
    // eglCreateImageKHR will add a ref to the OH_NativeBuffer

    PFNEGLCREATEIMAGEKHRPROC EGLCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("EGLCreateImageKHR");
    if (!EGLCreateImageKHR) {
        DLOG(INFO) << "无法获取 EGLCreateImageKHR 函数指针";
        return GrBackendTexture();
    }

    EGLImageKHR image = EGLCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_OHOS, clientBuffer, attribs);
    if (EGL_NO_IMAGE_KHR == image) {
        SkDebugf("Could not create EGL image, err = (%#x)", static_cast<int>(eglGetError()));
        return GrBackendTexture();
    }

    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }

    GrGLuint target = isRenderable ? GR_GL_TEXTURE_2D : GR_GL_TEXTURE_EXTERNAL;

    glBindTexture(target, texID);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glBindTexture failed (%#x)", static_cast<int>(status));
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    glEGLImageTargetTexture2DOES(target, image);
    if ((status = glGetError()) != GL_NO_ERROR) {
        SkDebugf("glEGLImageTargetTexture2DOES failed (%#x)", static_cast<int>(status));
        glDeleteTextures(1, &texID);
        eglDestroyImageKHR(display, image);
        return GrBackendTexture();
    }
    dContext->resetContext(kTextureBinding_GrGLBackendState);

    GrGLTextureInfo textureInfo;
    textureInfo.fID = texID;
    SkASSERT(backendFormat.isValid());
    textureInfo.fTarget = target;
    textureInfo.fFormat = GrBackendFormats::AsGLFormatEnum(backendFormat);
    textureInfo.fProtected = skgpu::Protected(isProtectedContent);

    *deleteProc = delete_gl_texture;
    *updateProc = update_gl_texture;
    *imageCtx = new GLTextureHelper(texID, image, display, target);

    return GrBackendTextures::MakeGL(width, height, skgpu::Mipmapped::kNo, textureInfo);
}

static bool can_import_protected_content_eglimpl() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    const char *exts = eglQueryString(dpy, EGL_EXTENSIONS);
    size_t cropExtLen = strlen(PROT_CONTENT_EXT_STR);
    size_t extsLen = strlen(exts);
    bool equal = !strcmp(PROT_CONTENT_EXT_STR, exts);
    bool atStart = !strncmp(PROT_CONTENT_EXT_STR " ", exts, cropExtLen + 1);
    bool atEnd = (cropExtLen + 1) < extsLen && !strcmp(" " PROT_CONTENT_EXT_STR, exts + extsLen - (cropExtLen + 1));
    bool inMiddle = strstr(exts, " " PROT_CONTENT_EXT_STR " ");
    return equal || atStart || atEnd || inMiddle;
}

static bool can_import_protected_content(GrDirectContext *dContext) {
    SkASSERT(GrBackendApi::kOpenGL == dContext->backend());
    // Only compute whether the extension is present once the first time this
    // function is called.
    static bool hasIt = can_import_protected_content_eglimpl();
    return hasIt;
}

GrBackendTexture MakeGLBackendTexture(GrDirectContext *dContext, OH_NativeBuffer *Buffer, int width, int height,
                                      DeleteImageProc *deleteProc, UpdateImageProc *updateProc, TexImageCtx *imageCtx,
                                      bool isProtectedContent, const GrBackendFormat &backendFormat,
                                      bool isRenderable) {
    SkASSERT(dContext);
    if (!dContext || dContext->abandoned()) {
        return GrBackendTexture();
    }

    if (GrBackendApi::kOpenGL != dContext->backend()) {
        return GrBackendTexture();
    }

    if (isProtectedContent && !can_import_protected_content(dContext)) {
        return GrBackendTexture();
    }

    return make_gl_backend_texture(dContext, Buffer, width, height, deleteProc, updateProc, imageCtx,
                                   isProtectedContent, backendFormat, isRenderable);
}

} // namespace RNSkia
