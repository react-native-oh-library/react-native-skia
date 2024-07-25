#include "HarmonyPlatformContext.h"
// #include "HarmonyBufferUtils.h"


#include <multimedia/image_framework/image_pixel_map_mdk.h>
#include <cstring>
#include <fstream>
#include <iostream>
// #include <curl/curl.h>
#include "include/core/SkTypeface.h"
#include "RNSkPlatformContext.h"
#include "native_buffer/native_buffer.h" // releaseNativeBuffer/makeImageFromNativeBuffer

// #include "include/core/SkData.h"
// #include "include/core/SkImageInfo.h"       // SkImageInfo::Make
// #include "include/core/SkColorSpace.h"      // SkColorSpace::MakeSRGB());
// #include "include/core/SkRefCnt.h"          // createFontMgr
// #include "include/core/SkAlphaType.h"       // kPremul_SkAlphaType
// #include "include/core/SkBitmap.h"
// #include "include/core/SkCanvas.h"
// #include "include/core/SkSurface.h"

#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/ports/SkFontMgr_android.h"
#include "include/gpu/ganesh/SkImageGanesh.h" // BorrowTextureFrom
// #include "include/gpu/ganesh/gl/GrGLBackendSurface.h"    // MakeGL
// #include "include/gpu/GrTypes.h"            // kTopLeft_GrSurfaceOrigin
#include "src/gpu/ganesh/gl/GrGLDefines.h"


namespace RNSkia {

thread_local SkiaOpenGLContext ThreadContextHolder::ThreadSkiaOpenGLContext;


HarmonyPlatformContext::HarmonyPlatformContext(jsi::Runtime *runtime, std::shared_ptr<react::CallInvoker> callInvoker,
                                               float pixelDensity)
    : RNSkPlatformContext(runtime, callInvoker, pixelDensity), drawLoopActive(false),
      playLink(std::make_unique<PlayLink>([](double deltaTime) {})) {
    mainThread = std::thread(&HarmonyPlatformContext::runTaskOnMainThread, this);
}

HarmonyPlatformContext::~HarmonyPlatformContext() { SetStopRunOnMainThread(); }

// void HarmonyPlatformContext::InitializeGLTextureHelper(GrGLuint texID, EGLImageKHR image, EGLDisplay display,
//                                                        GrGLuint texTarget) {
//     glTextureHelper = std::make_unique<GLTextureHelper>(texID, image, display, texTarget);
// }

// 启动绘图循环
void HarmonyPlatformContext::startDrawLoop() {
    if (drawLoopActive) {
        return;
    }
    // 确保不会重复启动绘图循环
    drawLoopActive = true;

    if (playLink) {
        playLink->startDrawLoop();
    }
}

void HarmonyPlatformContext::stopDrawLoop() {
    if (drawLoopActive) {
        drawLoopActive = false;
    }

    if (playLink) {
        playLink->stopDrawLoop();
    }
}

// 添加任务队列，通知主线程
void HarmonyPlatformContext::runTaskOnMainThread() {
    while (!stopMainLoop) {
        std::unique_lock<std::mutex> lock(taskMutex);
        // taskCond 线程间的同步和通知
        taskCond.wait(lock, [this] { return !taskQueue.empty() || stopMainLoop; });

        while (!taskQueue.empty()) {
            auto task = std::move(taskQueue.front());
            taskQueue.pop();
            lock.unlock();
            task(); // 执行任务
            lock.lock();
        }
    }
}

void HarmonyPlatformContext::SetStopRunOnMainThread() {
    std::lock_guard<std::mutex> lock(taskMutex);
    stopMainLoop = true;
    taskCond.notify_all(); // 通知主线程停止
    if (mainThread.joinable()) {
        mainThread.join(); // 等待主线程结束
    }
}

void HarmonyPlatformContext::runOnMainThread(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(taskMutex);
    taskQueue.push(std::move(task));
    taskCond.notify_one(); // 通知主线程有新任务
}

// 从本地缓冲区（native buffer）转为Skia的SkImage对象
sk_sp<SkImage> HarmonyPlatformContext::makeImageFromNativeBuffer(void *buffer) {
    OH_NativeBuffer *nativeBuffer = static_cast<OH_NativeBuffer *>(buffer);

    DeleteImageProc deleteImageProc = nullptr;
    UpdateImageProc updateImageProc = nullptr;
    TexImageCtx deleteImageCtx = nullptr;

    OH_NativeBuffer_Config config;
    OH_NativeBuffer_GetConfig(nativeBuffer, &config);

    GrBackendFormat format;
    switch (config.format) {
    case NATIVEBUFFER_PIXEL_FMT_RGBA_8888:
        format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
    case NATIVEBUFFER_PIXEL_FMT_RGB_565:
        format = GrBackendFormats::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_EXTERNAL);
    case NATIVEBUFFER_PIXEL_FMT_RGB_888:
        format = GrBackendFormats::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_EXTERNAL);
    default:
        format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
    }

    auto backendTex = MakeGLBackendTexture(ThreadContextHolder::ThreadSkiaOpenGLContext.directContext.get(),
                                           const_cast<OH_NativeBuffer *>(nativeBuffer), config.width, config.height,
                                           &deleteImageProc, &updateImageProc, &deleteImageCtx, false, format, false);
    if (!backendTex.isValid()) {
        DLOG(INFO) << "couldn't create offscreen texture:"
                      "Failed to convert HardwareBuffer to OpenGL Texture!";
        return nullptr;
    }

    sk_sp<SkImage> image = SkImages::BorrowTextureFrom(ThreadContextHolder::ThreadSkiaOpenGLContext.directContext.get(),
                                                       backendTex, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                                       kOpaque_SkAlphaType, nullptr, deleteImageProc, deleteImageCtx);
    return image;
}

/*
将 Skia 图像 (SkImage) 转为本地硬件缓冲区 (OH_NativeBuffer)
*/
uint64_t HarmonyPlatformContext::makeNativeBuffer(sk_sp<SkImage> image) {
    auto bytesPerPixel = image->imageInfo().bytesPerPixel();
    int bytesPerRow = image->width() * bytesPerPixel;
    auto buf = SkData::MakeUninitialized(image->width() * image->height() * bytesPerPixel);
    SkImageInfo info = SkImageInfo::Make(image->width(), image->height(), image->colorType(), image->alphaType());
    image->readPixels(nullptr, info, const_cast<void *>(buf->data()), bytesPerRow, 0, 0);
    const void *pixelData = buf->data();
    // 配置
    OH_NativeBuffer_Config config = {};
    config.width = image->width();
    config.height = image->height();
    config.format = GetBufferFormatFromSkColorType(image->colorType());
    // 本地缓冲区的使用情况，CPU 读取缓冲区，CPU 写入内存，直接内存访问（DMA）缓冲器
    config.usage = NATIVEBUFFER_USAGE_CPU_READ | NATIVEBUFFER_USAGE_CPU_WRITE | NATIVEBUFFER_USAGE_MEM_DMA;
    config.stride = bytesPerRow;

    // 分配与传递的 BufferRequestConfig 匹配的 OH_NativeBuffer 文件
    // 每次调用此函数时，都会创建一个新的 OH_NativeBuffer 实例
    OH_NativeBuffer *buffer = OH_NativeBuffer_Alloc(&config);
    if (buffer == nullptr) {
        return 0;
    }

    void *mappedBuffer = nullptr;
    // 提供对进程地址空间中 OH_NativeBuffer 的直接 CPU 访问
    if (OH_NativeBuffer_Map(buffer, &mappedBuffer) != 0) {
        // 减少 OH_NativeBuffer 的引用计数，当引用计数为 0 时，销毁该 OH_NativeBuffer
        OH_NativeBuffer_Unreference(buffer);
        return 0;
    }
    memcpy(mappedBuffer, pixelData, config.height * bytesPerRow);
    // 删除进程地址空间中 OH_NativeBuffer 的直接 CPU 访问权限
    OH_NativeBuffer_Unmap(buffer);
    return reinterpret_cast<uint64_t>(buffer);
}

void HarmonyPlatformContext::releaseNativeBuffer(uint64_t pointer) {
    OH_NativeBuffer *buffer = reinterpret_cast<OH_NativeBuffer *>(pointer);
    if (buffer) {
        //  销毁该 OH_NativeBuffer
        OH_NativeBuffer_Unreference(buffer);
    }
}

std::shared_ptr<RNSkVideo> HarmonyPlatformContext::createVideo(const std::string &url) { return nullptr; }

// 异步执行流操作
void HarmonyPlatformContext::performStreamOperation(const std::string &sourceUri,
                                                    const std::function<void(std::unique_ptr<SkStreamAsset>)> &op) {
    std::thread([=]() {
        std::vector<uint8_t> data;
        try {
            // 引入URI解析库
            if (sourceUri.find(':') == std::string::npos && sourceUri.find('.') != std::string::npos) {
                data = ReadFileData(sourceUri); // 读取文件数据
            } else {
                data = PerformHTTPRequest(sourceUri); // 执行HTTP请求
            }

            if (!data.empty()) {
                // 使用SkData::MakeFromCopy创建SkData
                sk_sp<SkData> skData = SkData::MakeWithCopy(data.data(), data.size());
                auto skStream = SkMemoryStream::Make(skData);

                //                 sk_sp<SkMemoryStream> stream(SkMemoryStream::Make(skData).release());
                //                 // 释放sk_sp
                //                 std::unique_ptr<SkStreamAsset> streamUniquePtr(stream.release());
                //                 // 将unique_ptr传递给回调函数
                //                 op(std::move(streamUniquePtr));

                op(std::move(skStream));
            }
        } catch (const std::exception &e) {
            std::cerr << "出现异常: " << e.what() << std::endl;
            op(nullptr);
        }
    }).detach();
}

void HarmonyPlatformContext::raiseError(const std::exception &err) {
    std::cerr << "Error: " << err.what() << std::endl;
    std::terminate();
}

sk_sp<SkSurface> HarmonyPlatformContext::makeOffscreenSurface(int width, int height) {
    return SkiaOpenGLSurfaceFactory::makeOffscreenSurface(width, height);
}

sk_sp<SkFontMgr> HarmonyPlatformContext::createFontMgr() {
    //     return SkFontMgr_New_Android(nullptr);
    return nullptr;
}

// 从具有给定标签（tag）的视图（View）中捕获屏幕截图，并将其转换为 SkImage 对象
sk_sp<SkImage> HarmonyPlatformContext::takeScreenshotFromViewTag(size_t tag) {
    // 截图
    /*
    auto env = jni::Environment::current();
    static auto method = javaPart_->getClass()->getMethod<jobject(int)>(
      "takeScreenshotFromViewTag");

    typedef struct OH_Drawing_Bitmap OH_Drawing_Bitmap;
    OH_Drawing_Bitmap *bitmap = method(javaPart_.get(), tag).release();

  // Let's convert to a native bitmap and get some info about the bitmap

    auto *bmi=OH_Drawing_BitmapGetPixels(bitmap);
  // Convert bitmap info to a Skia bitmap info
    auto colorType = color_type(bmi.format);
    auto alphaType = alpha_type(bmi.flags);

    auto skInfo = SkImageInfo::Make(SkISize::Make(bmi->width, bmi.height),
                                  colorType, alphaType);

  // Lock pixels
    NativePixelMap *pixels;
    OH_PixelMap_AccessPixels(&pixels, &bitmap);

  // Create pixmap from pixels and make a copy of it so that
  // the SkImage owns its own pixels
    SkPixmap pm(skInfo, pixels, 4*OH_Drawing_BitmapGetWidth(bitmap));
    auto skImage = SkImages::RasterFromPixmapCopy(pm);

  // Unlock pixels
    OH_PixelMap_UnAccessPixels(bitmap);

  // Return our newly created SkImage!
    return skImage;*/
    return nullptr;
}

// 读取文件数据
std::vector<uint8_t> HarmonyPlatformContext::ReadFileData(const std::string &sourceUri) {
    std::vector<uint8_t> buffer;
    // 检查文件是否存在
    // std::filesystem::path sourcePath(sourceUri.c_str());
    // if (!std::filesystem::exists(sourcePath)) {
    //    std::cerr << "File does not exist: " << sourceUri << std::endl;
    //    return buffer;
    //}
    if (access(sourceUri.c_str(), F_OK) != 0) {
        std::cerr << "File does not exist: " << sourceUri << std::endl;
        return buffer;
    }
    std::ifstream file(sourceUri, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << sourceUri << std::endl;
        return buffer;
    }
    std::streamsize size = file.tellg();
    buffer.resize(size);
    file.seekg(0, std::ios::beg); // 回到文件头
    // 读取文件内容到buffer中
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        std::cerr << "Failed to read file: " << sourceUri << std::endl;
        buffer.clear();
    }
    return buffer;
}

// 使用libcurl库的写入函数
size_t HarmonyPlatformContext::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    WriteData *writeData = static_cast<WriteData *>(userp);
    size_t totalBytes = size * nmemb;
    //     writeData->responseData->insert(writeData->responseData->end(), static_cast<char *>(contents),
    //                                     static_cast<char *>(contents) + totalBytes);
    return totalBytes;
}

// 执行HTTP请求并返回数据
std::vector<uint8_t> HarmonyPlatformContext::PerformHTTPRequest(const std::string &sourceUri) {
    std::vector<uint8_t> responseData;
    //     CURL *curl;
    //     CURLcode res;
    //
    //     curl = curl_easy_init();
    //     if (curl) {
    //         WriteData writeData = {&responseData}; // 创建并初始化WriteData结构体
    //         curl_easy_setopt(curl, CURLOPT_URL, sourceUri.c_str());
    //         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // 使用静态成员函数或全局函数
    //         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);        // 传递WriteData结构体的地址
    //
    //         // 执行请求
    //         res = curl_easy_perform(curl);
    //         // 检查并处理错误
    //         if (res != CURLE_OK) {
    //             std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    //             responseData.clear();
    //         }
    //
    //         curl_easy_cleanup(curl);
    //     } else {
    //         std::cerr << "curl_easy_init() failed" << std::endl;
    //     }

    return responseData;
}

uint32_t HarmonyPlatformContext::GetBufferFormatFromSkColorType(SkColorType bufferFormat) {
    switch (bufferFormat) {
    case kRGBA_8888_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_RGBA_8888; // RGBA8888 格式
    case kRGB_888x_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_RGBX_8888; // RGBX8888 格式
    case kRGBA_F16_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_BUTT; // 鸿蒙NativeBuffer库中不支持该格式返回无效格式
    case kRGB_565_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_RGB_565; // RGB565 格式
    case kRGBA_1010102_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_BUTT; // 鸿蒙NativeBuffer库中不支持该格式返回无效格式
    case kAlpha_8_SkColorType:
        return NATIVEBUFFER_PIXEL_FMT_BUTT; // 鸿蒙NativeBuffer库中不支持该格式返回无效格式
    default:
        return NATIVEBUFFER_PIXEL_FMT_RGBA_8888;
    }
}

} // namespace RNSkia