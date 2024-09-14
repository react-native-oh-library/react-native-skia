/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hilog/log.h>
#include <glog/logging.h>
#include "common.h"
#include "plugin_manager.h"

#include <js_native_api.h>
#include <js_native_api_types.h>
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <napi/native_api.h>

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    DLOG(INFO) << "Init begins";
    if ((env == nullptr) || (exports == nullptr)) {
        DLOG(INFO) << "env or exports is null";
        return nullptr;
    }
    napi_property_descriptor desc[] = {
        {"registerView", nullptr, RNSkia::PluginRender::RegisterView, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setModeAndDebug", nullptr, RNSkia::PluginRender::SetModeAndDebug, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onSurfaceSizeChanged", nullptr, RNSkia::PluginRender::SurfaceSizeChanged, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    auto napiResult = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    if (napiResult != napi_ok) {
        DLOG(ERROR) << "Export: napi_define_properties failed";
    }
    RNSkia::PluginManager::GetInstance()->Export(env, exports);
    
    return exports;
}

EXTERN_C_END
static napi_module rn_skiaModule = {.nm_version = 1,
                                         .nm_flags = 0,
                                         .nm_filename = nullptr,
                                         // 入口函数
                                         .nm_register_func = Init,
                                         // 模块名称
                                         .nm_modname = "rnoh_skia",
                                         .nm_priv = ((void *)0),
                                         .reserved = {0}};

// 使用NAPI接口napi_module_register()传入模块描述信息进行模块注册
extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&rn_skiaModule); }