/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <cstdint>
#include <hilog/log.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <string>


#include "RNSkDomView.h"
#include "RNSkHarmonyView.h"
#include "SkiaManager.h"
#include "common.h"
#include "plugin_manager.h"
#include "plugin_render.h"
#include <native_window/external_window.h>


namespace RNSkia {
std::unordered_map<std::string, PluginRender *> PluginRender::m_instance;

PluginRender::PluginRender(std::shared_ptr<RNSkia::RNSkPlatformContext> context) {
    _harmonyView = std::make_shared<RNSkHarmonyView<RNSkia::RNSkDomView>>(context);
}

PluginRender *PluginRender::GetInstance(std::string &id) {
    if (m_instance.find(id) == m_instance.end()) {
        PluginRender *instance = new PluginRender(SkiaManager::getInstance().getContext());
        m_instance[id] = instance;
        return instance;
    } else {
        return m_instance[id];
    }
}

void OnSurfaceCreatedCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnSurfaceCreatedCB";
    if ((component == nullptr) || (window == nullptr)) {
        DLOG(ERROR) << "Callback OnSurfaceCreatedCB: component or window is null";
        return;
    }

    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback OnSurfaceCreatedCB: Unable to get XComponent id";
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    uint64_t width;
    uint64_t height;
    int32_t xSize = OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);

    if ((xSize == OH_NATIVEXCOMPONENT_RESULT_SUCCESS) && (render != nullptr)) {
        // render->m_window = window;
        OHNativeWindow *nativeWindow = static_cast<OHNativeWindow *>(window);
        render->m_width = width;
        render->m_height = height;
        
        if (render->_harmonyView) {
            //native 的获取？
            SkiaManager::getInstance().getManager()->registerSkiaView(1000, render->_harmonyView->getSkiaView());

            render->_harmonyView->surfaceAvailable(nativeWindow, width, height);
            // render->_harmonyView->init(nativeWindow, width, height);
        } else {
            DLOG(ERROR) << "Callback OnSurfaceCreatedCB _harmonyView is null";
        }
        DLOG(INFO) << "Callback OnSurfaceCreatedCB surfaceAvailable finish";
    }
}

void OnSurfaceChangedCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnSurfaceChangedCB";
    if ((component == nullptr) || (window == nullptr)) {
        DLOG(ERROR) << "Callback OnSurfaceChangedCB: component or window is null";
        return;
    }

    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback OnSurfaceChangedCB: Unable to get XComponent id";
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnSurfaceChanged(component, window);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "Callback", "surface changed");
    }
}

void OnSurfaceDestroyedCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnSurfaceDestroyedCB";
    if ((component == nullptr) || (window == nullptr)) {
        DLOG(ERROR) << "Callback OnSurfaceDestroyedCB: component or window is null";
        return;
    }

    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback OnSurfaceDestroyedCB: Unable to get XComponent id";
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->_harmonyView->surfaceDestroyed();
        DLOG(INFO) << "Callback surfaceDestroyed";
    }
}

void DispatchTouchEventCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback DispatchTouchEventCB";
    if ((component == nullptr) || (window == nullptr)) {
        DLOG(ERROR) << "Callback DispatchTouchEventCB: component or window is null";
        return;
    }

    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback DispatchTouchEventCB: Unable to get XComponent id";
        return;
    }

    std::string id(idStr);
    PluginRender *render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnTouchEvent(component, window);
    }
}

void DispatchMouseEventCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback DispatchMouseEventCB";
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    int32_t ret = OH_NativeXComponent_GetXComponentId(component, idStr, &idSize);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnMouseEvent(component, window);
    }
}
// 悬停事件
void DispatchHoverEventCB(OH_NativeXComponent *component, bool isHover) {
    DLOG(INFO) << "Callback DispatchHoverEventCB";
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    int32_t ret = OH_NativeXComponent_GetXComponentId(component, idStr, &idSize);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnHoverEvent(component, isHover);
    }
}
// 对焦事件
void OnFocusEventCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnFocusEventCB";
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    int32_t ret = OH_NativeXComponent_GetXComponentId(component, idStr, &idSize);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnFocusEvent(component, window);
    }
}
// 模糊事件
void OnBlurEventCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnBlurEventCB";
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    int32_t ret = OH_NativeXComponent_GetXComponentId(component, idStr, &idSize);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }

    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnBlurEvent(component, window);
    }
}
// 按键事件
void OnKeyEventCB(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "Callback OnKeyEventCB";
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    int32_t ret = OH_NativeXComponent_GetXComponentId(component, idStr, &idSize);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return;
    }
    std::string id(idStr);
    auto render = PluginRender::GetInstance(id);
    if (render != nullptr) {
        render->OnKeyEvent(component, window);
    }
}

void PluginRender::OnSurfaceChanged(OH_NativeXComponent *component, void *window) {
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback OnSurfaceChanged: Unable to get XComponent id";
        return;
    }

    std::string id(idStr);
    PluginRender *render = PluginRender::GetInstance(id);
    double offsetX;
    double offsetY;
    OH_NativeXComponent_GetXComponentOffset(component, window, &offsetX, &offsetY);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "OH_NativeXComponent_GetXComponentOffset",
                 "offsetX = %{public}lf, offsetY = %{public}lf", offsetX, offsetY);
    uint64_t width;
    uint64_t height;
    OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
    if (render != nullptr) {
        render->_harmonyView->surfaceSizeChanged(width, height);
    }
}

void PluginRender::OnTouchEvent(OH_NativeXComponent *component, void *window) {
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        DLOG(ERROR) << "Callback DispatchTouchEventCB: Unable to get XComponent id";
        return;
    }
    OH_NativeXComponent_TouchEvent touchEvent;
    OH_NativeXComponent_GetTouchEvent(component, window, &touchEvent);

    float tiltX = 0.0f;
    float tiltY = 0.0f;
    OH_NativeXComponent_TouchPointToolType toolType =
        OH_NativeXComponent_TouchPointToolType::OH_NATIVEXCOMPONENT_TOOL_TYPE_UNKNOWN;
    OH_NativeXComponent_GetTouchPointToolType(component, 0, &toolType);
    OH_NativeXComponent_GetTouchPointTiltX(component, 0, &tiltX);
    OH_NativeXComponent_GetTouchPointTiltY(component, 0, &tiltY);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "OnTouchEvent",
                 "touch info: toolType = %{public}d, tiltX = %{public}lf, tiltY = %{public}lf", toolType, tiltX, tiltY);

    std::string id(idStr);
    PluginRender *render = PluginRender::GetInstance(id);
    RNSkTouchInfo info;
    std::vector<RNSkTouchInfo> touches;
    if (render != nullptr && touchEvent.type == OH_NativeXComponent_TouchEventType::OH_NATIVEXCOMPONENT_UP) {
        info.x = tiltX;
        info.y = tiltY;
        touches.push_back(info);
        render->_harmonyView->updateTouchPoints(touches);
    }
}

void PluginRender::RegisterCallback(OH_NativeXComponent *nativeXComponent) {
    m_renderCallback.OnSurfaceCreated = OnSurfaceCreatedCB;
    m_renderCallback.OnSurfaceChanged = OnSurfaceChangedCB;
    m_renderCallback.OnSurfaceDestroyed = OnSurfaceDestroyedCB;
    m_renderCallback.DispatchTouchEvent = DispatchTouchEventCB;
    OH_NativeXComponent_RegisterCallback(nativeXComponent, &m_renderCallback);

    m_mouseCallback.DispatchMouseEvent = DispatchMouseEventCB;
    m_mouseCallback.DispatchHoverEvent = DispatchHoverEventCB;
    OH_NativeXComponent_RegisterMouseEventCallback(nativeXComponent, &m_mouseCallback);

    OH_NativeXComponent_RegisterFocusEventCallback(nativeXComponent, OnFocusEventCB);
    OH_NativeXComponent_RegisterKeyEventCallback(nativeXComponent, OnKeyEventCB);
    OH_NativeXComponent_RegisterBlurEventCallback(nativeXComponent, OnBlurEventCB);
}

void PluginRender::OnMouseEvent(OH_NativeXComponent *component, void *window) {
    DLOG(INFO) << "PluginRender OnMouseEvent";
    OH_NativeXComponent_MouseEvent mouseEvent;
    int32_t ret = OH_NativeXComponent_GetMouseEvent(component, window, &mouseEvent);
    if (ret == OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender",
                     "MouseEvent Info: x = %{public}f, y = %{public}f, action = %{public}d, button = %{public}d",
                     mouseEvent.x, mouseEvent.y, mouseEvent.action, mouseEvent.button);
    } else {
        DLOG(ERROR) << "PluginRender GetMouseEvent error";
    }
}

void PluginRender::OnHoverEvent(OH_NativeXComponent *component, bool isHover) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender", "OnHoverEvent isHover_ = %{public}d", isHover);
}

void PluginRender::OnFocusEvent(OH_NativeXComponent *component, void *window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender", "OnFocusEvent");
}

void PluginRender::OnBlurEvent(OH_NativeXComponent *component, void *window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender", "OnBlurEvent");
}

void PluginRender::OnKeyEvent(OH_NativeXComponent *component, void *window) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender", "OnKeyEvent");

    OH_NativeXComponent_KeyEvent *keyEvent = nullptr;
    if (OH_NativeXComponent_GetKeyEvent(component, &keyEvent) >= 0) {
        OH_NativeXComponent_KeyAction action;
        OH_NativeXComponent_GetKeyEventAction(keyEvent, &action);
        OH_NativeXComponent_KeyCode code;
        OH_NativeXComponent_GetKeyEventCode(keyEvent, &code);
        OH_NativeXComponent_EventSourceType sourceType;
        OH_NativeXComponent_GetKeyEventSourceType(keyEvent, &sourceType);
        int64_t deviceId;
        OH_NativeXComponent_GetKeyEventDeviceId(keyEvent, &deviceId);
        int64_t timeStamp;
        OH_NativeXComponent_GetKeyEventTimestamp(keyEvent, &timeStamp);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginRender",
                     "KeyEvent Info: action=%{public}d, code=%{public}d, sourceType=%{public}d, deviceId=%{public}ld, "
                     "timeStamp=%{public}ld",
                     action, code, sourceType, deviceId, timeStamp);
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginRender", "GetKeyEvent error");
    }
}

void PluginRender::Export(napi_env env, napi_value exports) {
    //     if ((env == nullptr) || (exports == nullptr)) {
    //         DLOG(ERROR) << "Export: env or exports is null";
    //         return;
    //     }
    //     napi_property_descriptor desc[] = {
    //         {"registerView", nullptr, RegisterView, nullptr, nullptr, nullptr, napi_default, nullptr},
    //
    //         {"init", nullptr, RNSkHarmonyVideo::NapiInit, nullptr, nullptr, nullptr, napi_default, nullptr},
    //         {"Play", nullptr, RNSkHarmonyVideo::NapiPlay, nullptr, nullptr, nullptr, napi_default, nullptr},
    //         {"Pause", nullptr, RNSkHarmonyVideo::NapiPause, nullptr, nullptr, nullptr, napi_default, nullptr},
    //         {"Release", nullptr, RNSkHarmonyVideo::NapiRelease, nullptr, nullptr, nullptr, napi_default, nullptr},
    //     };
    //     auto napiResult = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    //     if (napiResult != napi_ok) {
    //         DLOG(ERROR) << "Export: napi_define_properties failed";
    //     }
}

} // namespace RNSkia
