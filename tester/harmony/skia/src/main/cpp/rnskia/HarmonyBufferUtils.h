#ifndef HARMONY_BUFFER_UTILS_H
#define HARMONY_BUFFER_UTILS_H

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
// #include "include/gpu/GrTypesPriv.h"


#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "native_buffer/native_buffer.h"

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

class GrDirectContext;
typedef struct OH_NativeBuffer OH_NativeBuffer;

namespace RNSkia {

typedef void *TexImageCtx;
typedef void (*DeleteImageProc)(TexImageCtx);
typedef void (*UpdateImageProc)(TexImageCtx, GrDirectContext *);

typedef EGLClientBuffer (*EGLGetNativeClientBufferHarmonyProc)(const struct OH_NativeBuffer *);
typedef EGLImageKHR (*EGLCreateImageKHRProc)(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLint *);
typedef void (*EGLImageTargetTexture2DOESProc)(EGLenum, void *);

class GLTextureHelper {
public:
    GLTextureHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display, GrGLuint texTarget);
    ~GLTextureHelper();

    void rebind(GrDirectContext *);

    GrBackendFormat GetGLBackendFormat(GrDirectContext *dContext, uint32_t bufferFormat, bool requireKnownFormat);

private:
    GrGLuint fTexID;
    EGLImageKHR fImage;
    EGLDisplay fDisplay;
    GrGLuint fTexTarget;
};

GrBackendTexture MakeGLBackendTexture(GrDirectContext *dContext, OH_NativeBuffer *Buffer, int width, int height,
                                      DeleteImageProc *deleteProc, UpdateImageProc *updateProc, TexImageCtx *imageCtx,
                                      bool isProtectedContent, const GrBackendFormat &backendFormat, bool isRenderable);

} // namespace RNSkia

#endif // HARMONY_BUFFER_UTILS_H