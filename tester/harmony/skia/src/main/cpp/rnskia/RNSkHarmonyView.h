#pragma once

#include <memory>
#include <string>
#include <vector>

#include "RNSkOpenGLCanvasProvider.h"
#include "egl_core.h"

namespace RNSkia {

class RNSkBaseHarmonyView {
public:
    virtual void surfaceAvailable(OHNativeWindow *surface, int width, int height) = 0;

    virtual void surfaceDestroyed() = 0;

    virtual void surfaceSizeChanged(int width, int height) = 0;

    virtual float getPixelDensity() = 0;

    virtual void updateTouchPoints(std::vector<RNSkTouchInfo> touches) = 0;

    virtual void setMode(std::string mode) = 0;

    virtual void setShowDebugInfo(bool show) = 0;

    virtual void viewDidUnmount() = 0;

    virtual std::shared_ptr<RNSkView> getSkiaView() = 0;

    virtual void init(void *window, int width, int height) = 0;
    virtual void render() = 0;
};

template <typename T> class RNSkHarmonyView : public T, public RNSkBaseHarmonyView {
public:
    explicit RNSkHarmonyView(std::shared_ptr<RNSkia::RNSkPlatformContext> context)
        : T(context,
            std::make_shared<RNSkOpenGLCanvasProvider>(std::bind(&RNSkia::RNSkView::requestRedraw, this), context)) {}

    void surfaceAvailable(OHNativeWindow *surface, int width, int height) override {
        std::static_pointer_cast<RNSkOpenGLCanvasProvider>(T::getCanvasProvider())
            ->surfaceAvailable(surface, width, height);

        // Try to render directly when the surface has been set so that
        // we don't have to wait until the draw loop returns.
        DLOG(INFO) << "surfaceAvailable to renderImmediate";
        RNSkView::renderImmediate();
        DLOG(INFO) << "surfaceAvailable to renderImmediate finish";
    }

    void surfaceDestroyed() override {
        std::static_pointer_cast<RNSkOpenGLCanvasProvider>(T::getCanvasProvider())->surfaceDestroyed();
    }

    void surfaceSizeChanged(int width, int height) override {
        std::static_pointer_cast<RNSkOpenGLCanvasProvider>(T::getCanvasProvider())->surfaceSizeChanged(width, height);
        // This is only need for the first time to frame, this renderImmediate call
        // will invoke updateTexImage for the previous frame
        RNSkView::renderImmediate();
    }

    float getPixelDensity() override { return T::getPlatformContext()->getPixelDensity(); }

    void setMode(std::string mode) override {
        if (mode.compare("continuous") == 0) {
            T::setDrawingMode(RNSkDrawingMode::Continuous);
        } else {
            T::setDrawingMode(RNSkDrawingMode::Default);
        }
    }

    void setShowDebugInfo(bool show) override { T::setShowDebugOverlays(show); }

    void viewDidUnmount() override { T::endDrawingLoop(); }

    void updateTouchPoints(std::vector<RNSkTouchInfo> touches) override {
        auto scale = getPixelDensity();
        for (auto &touch : touches) {
            touch.x /= scale;
            touch.y /= scale;
        }
        T::updateTouchState(touches);
    }

    std::shared_ptr<RNSkView> getSkiaView() override { return T::shared_from_this(); }

    EGLCore eGLCore;
    void init(void *window, int width, int height) override {
        if (eGLCore.EglContextInit(window, width, height)) {
            eGLCore.Background();
        }
        int32_t has = 0;
        eGLCore.Draw(has);
    }

    void render() override {
        DLOG(INFO) << "render to renderImmediate";
        RNSkView::renderImmediate();
        DLOG(INFO) << "render to renderImmediate finish";
    }
};
} // namespace RNSkia
