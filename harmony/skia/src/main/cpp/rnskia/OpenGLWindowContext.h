/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "RNSkLog.h"

#include <EGL/egl.h>
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <condition_variable>
#include <memory>
#include <thread>
#include <unordered_map>

#include "RNWindowContext.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "HarmonyOpenGLHelper.h"
#pragma clang diagnostic pop

namespace RNSkia {

class OpenGLWindowContext : public WindowContext {
public:
  OpenGLWindowContext(GrDirectContext *directContext, EGLDisplay *display,
                      EGLContext *glContext, OHNativeWindow *window,
                      EGLConfig config)
      : _directContext(directContext), _glContext(glContext), _window(window) {
    _glSurface = SkiaOpenGLHelper::createWindowedSurface(_window);
   int32_t ret = OH_NativeWindow_NativeWindowHandleOpt(
        window,
        GET_BUFFER_GEOMETRY,
        &_width,
        &_height
    );     
  }

  ~OpenGLWindowContext() override {
    _skSurface = nullptr;
    _glSurface = nullptr;
  }

  sk_sp<SkSurface> getSurface() override;

  void present() override;

  int getWidth() override { return _width; };

  int getHeight() override { return _height; };

  void resize(int width, int height) override {
    if (_skSurface != nullptr) {
      // Let's make sure there is no pending work
      _width = width;
      _height = height;
      _skSurface = nullptr;
    }
  }

private:
  GrDirectContext *_directContext;
  EGLContext *_glContext = nullptr;
  OHNativeWindow *_window;
  sk_sp<SkSurface> _skSurface = nullptr;
  EGLSurface _glSurface = nullptr;
  int _width;  
  int _height;  
};

} // namespace RNSkia
