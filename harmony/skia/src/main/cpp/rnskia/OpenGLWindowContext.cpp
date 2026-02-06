/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "OpenGLWindowContext.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

#pragma clang diagnostic pop

namespace RNSkia {


static bool makeCurrent(EGLContext glContext, EGLSurface glSurface) {
    // We don't need to call make current if we already are current:
    if (eglGetCurrentSurface(EGL_DRAW) != glSurface || eglGetCurrentSurface(EGL_READ) != glSurface ||
        eglGetCurrentContext() != glContext) {
        auto curr = eglMakeCurrent(OpenGLResourceHolder::getInstance().glDisplay, glSurface, glSurface, glContext);
        // Make current!
        if (curr != EGL_TRUE) {
            DLOG(ERROR) << "eglMakeCurrent failed: " << curr;
            return false;
        }
    }
    return true;
}

sk_sp<SkSurface> OpenGLWindowContext::getSurface() {
    if (_skSurface == nullptr) {
        makeCurrent(_glContext, _glSurface);
        GLint stencil;
        glGetIntegerv(GL_STENCIL_BITS, &stencil);

        GLint samples;
        glGetIntegerv(GL_SAMPLES, &samples);

        auto colorType = kRGBA_8888_SkColorType;

        auto maxSamples = _directContext->maxSurfaceSampleCountForColorType(colorType);

        if (samples > maxSamples) {
            samples = maxSamples;
        }

        GrGLFramebufferInfo fbInfo;
        fbInfo.fFBOID = 0;
        fbInfo.fFormat = GR_GL_RGBA8;
        int width;
        int height;
        int32_t ret = OH_NativeWindow_NativeWindowHandleOpt(_window, GET_BUFFER_GEOMETRY, &width, &height);
        auto backendRT = GrBackendRenderTargets::MakeGL(width, height, samples, stencil, fbInfo);
        sk_sp<SkColorSpace> colorSpace(nullptr);
        SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);
        _skSurface = SkSurfaces::WrapBackendRenderTarget(_directContext, backendRT, kBottomLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType, colorSpace, &surfaceProps);
    }
    return _skSurface;
}

void OpenGLWindowContext::present() {
    makeCurrent(_glContext, _glSurface);
    _directContext->flushAndSubmit();
    if (eglSwapBuffers(OpenGLResourceHolder::getInstance().glDisplay, _glSurface) != EGL_TRUE) {
        DLOG(INFO) << "eglSwapBuffers failed:" << eglGetError();
    }
}

} // namespace RNSkia
