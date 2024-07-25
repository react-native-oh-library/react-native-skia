#ifndef I_RNSK_HARMONY_VIEW_H
#define I_RNSK_HARMONY_VIEW_H

#include <memory>
#include <string>
#include <vector>
#include "RNSkOpenGLCanvasProvider.h"
#include "native_window/external_window.h"

namespace RNSkia {
class RNSkBaseHarmonyView {
public:
  virtual void surfaceAvailable(OHNativeWindow *surface, int width, int height) = 0;
 
  virtual void surfaceDestroyed() = 0;

  virtual void surfaceSizeChanged(int width, int height) = 0;

  virtual float getPixelDensity() = 0;
 
  virtual void updateTouchPoints(std::vector<double> touches) = 0;

  virtual void setMode(std::string mode) = 0;

  virtual void setShowDebugInfo(bool show) = 0; 

  virtual void viewDidUnmount() = 0;

  virtual std::shared_ptr<RNSkView> getSkiaView() = 0;
};


}// namespace RNSkia
#endif // I_RNSK_HARMONY_VIEW_H