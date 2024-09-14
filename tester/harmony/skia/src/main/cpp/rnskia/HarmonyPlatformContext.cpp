#include "HarmonyPlatformContext.h"
#include "RemoteCommunicationKit/rcp.h"
#include <cstdlib>
#include <rawfile/raw_file_manager.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include "RemoteCommunicationKit/rcp.h"
#include <iostream>
#include <regex>
#include "include/core/SkTypeface.h"
#include "RNSkPlatformContext.h"
#include "native_buffer/native_buffer.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/ports/SkFontMgr_android.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "plugin_manager.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "SkiaManager.h"
#include "skia_ohos/SkFontMgr_ohos.h"
#include "RNOH/RNInstance.h"
#include "RNSkHarmonyVideo.h"

namespace RNSkia {

thread_local SkiaOpenGLContext ThreadContextHolder::ThreadSkiaOpenGLContext;


HarmonyPlatformContext::HarmonyPlatformContext(jsi::Runtime *runtime, std::shared_ptr<react::CallInvoker> callInvoker,
                                               float pixelDensity)
    : RNSkPlatformContext(runtime, callInvoker, pixelDensity), drawLoopActive(false),
      playLink(std::make_unique<PlayLink>([this](double deltaTime) {
          notifyDrawLoop(false);
      })) {
    mainThread = std::thread(&HarmonyPlatformContext::runTaskOnMainThread, this);
}

HarmonyPlatformContext::~HarmonyPlatformContext() { SetStopRunOnMainThread(); }

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
    case NATIVEBUFFER_PIXEL_FMT_RGBA_1010102:
        format = GrBackendFormats::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_EXTERNAL);
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

std::shared_ptr<RNSkVideo> HarmonyPlatformContext::createVideo(const std::string &url) 
{
    return std::make_shared<RNSkHarmonyVideo>(url, this, nativeResourceManager); 
}

std::string getScheme(const std::string &uri) { // 解析uri
    std::regex uriRegex(R"(^([a-zA-Z][a-zA-Z0-9+.-]*):)");
    std::smatch match;
    if (std::regex_search(uri, match, uriRegex)) {
        return match[1].str();
    }
    return "";
}

// 异步执行流操作
void HarmonyPlatformContext::performStreamOperation(const std::string &sourceUri,
                                                    const std::function<void(std::unique_ptr<SkStreamAsset>)> &op) {
    auto loader = [=]() {
        std::vector<uint8_t> buffer;
        try {
            std::string scheme = getScheme(sourceUri);
            if (scheme == "file") {
                buffer = ReadFileData(sourceUri);
            } else if (scheme == "asset") {
                buffer = ReadAssetsData(sourceUri);
            } else if (scheme == "http" || "https") {
                buffer = PerformHTTPRequest(sourceUri);
            } else {
                DLOG(ERROR) << "performStreamOperation The URL is invalid (scheme is 'file || http || https')";
            }

            if (!buffer.empty()) {
                size_t length = buffer.size();
                uint8_t *uint8Ptr = buffer.data();
                // 将 uint8_t* 转换为 char*
                char *charPtr = reinterpret_cast<char *>(uint8Ptr);
                charPtr[length] = '\0';

                // 使用SkData::MakeFromCopy创建SkData
                sk_sp<SkData> skData = SkData::MakeWithCopy(charPtr, length);
                auto skStream = SkMemoryStream::Make(skData);

                op(std::move(skStream));
            } else {
                DLOG(ERROR) << "performStreamOperation buffer is empty";
            }
        } catch (const std::exception &e) {
            DLOG(ERROR) << "performStreamOperation exception";
        }
    };
    std::thread(loader).detach();
}

void HarmonyPlatformContext::raiseError(const std::exception &err) {
    std::cerr << "Error: " << err.what() << std::endl;
    std::terminate();
}

sk_sp<SkSurface> HarmonyPlatformContext::makeOffscreenSurface(int width, int height) {
    DLOG(INFO) << "makeOffscreenSurface START";
    // 关联Skia和OpenGL，
    if (!SkiaOpenGLHelper::createSkiaDirectContextIfNecessary(&ThreadContextHolder::ThreadSkiaOpenGLContext)) {
        DLOG(ERROR) << "Could not create Skia Surface from native window / surface."
                   << "Failed creating Skia Direct Context\n";
        return nullptr;
    }

    auto colorType = kN32_SkColorType; //
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry); // kUnknown_SkPixelGeometry
    if (!SkiaOpenGLHelper::makeCurrent(&ThreadContextHolder::ThreadSkiaOpenGLContext,
                                       ThreadContextHolder::ThreadSkiaOpenGLContext.gl1x1Surface)) {
        DLOG(ERROR) << "Could not create EGL Surface from native window / surface. Could "
                      "not set new surface as current surface.\n";
        return nullptr;
    }

    // 创建纹理
    auto texture = ThreadContextHolder::ThreadSkiaOpenGLContext.directContext->createBackendTexture(
        width, height, colorType, skgpu::Mipmapped::kNo, GrRenderable::kYes);

    if (!texture.isValid()) {
        DLOG(ERROR) << "couldn't create offscreen texture:" << width << height;
    }

    struct ReleaseContext {
        SkiaOpenGLContext *context;
        GrBackendTexture texture;
    };

    auto releaseCtx = new ReleaseContext({&ThreadContextHolder::ThreadSkiaOpenGLContext, texture});

    DLOG(INFO) << "makeOffscreenSurface END";
    // GrBackendTexture->SkSurface
    return SkSurfaces::WrapBackendTexture(
        ThreadContextHolder::ThreadSkiaOpenGLContext.directContext.get(), texture, kTopLeft_GrSurfaceOrigin, 0,
        colorType, nullptr, &props,
        [](void *addr) {
            auto releaseCtx = reinterpret_cast<ReleaseContext *>(addr);

            releaseCtx->context->directContext->deleteBackendTexture(releaseCtx->texture);
        
            DLOG(INFO) << "makeOffscreenSurface RELEASE";
        },
        releaseCtx);
}

sk_sp<SkFontMgr> HarmonyPlatformContext::createFontMgr() {
    return SkFontMgr_New_OHOS();
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
    return skImage;
    return nullptr;*/
    DLOG(ERROR) << "takeScreenshotFromViewTag not implement tag: " << tag;
    return nullptr;
}

// 读取文件数据
std::vector<uint8_t> HarmonyPlatformContext::ReadFileData(const std::string &sourceUri) {
    std::vector<uint8_t> buffer;

    if (access(sourceUri.c_str(), F_OK) != 0) {
        DLOG(ERROR) << "File does not exist: " << sourceUri << std::endl;
        return buffer;
    }
    std::ifstream file(sourceUri, std::ios::binary | std::ios::ate);
    if (!file) {
        DLOG(ERROR) << "Failed to open file: " << sourceUri << std::endl;
        return buffer;
    }
    std::streamsize size = file.tellg();
    buffer.resize(size);
    file.seekg(0, std::ios::beg); // 回到文件头
    // 读取文件内容到buffer中
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        DLOG(ERROR) << "Failed to read file: " << sourceUri << std::endl;
        buffer.clear();
    }
    return buffer;
}

std::vector<uint8_t> HarmonyPlatformContext::ReadAssetsData(const std::string &sourceUri) {
    std::string assetsFilePath = sourceUri;
    std::string Prefixes = "asset://";
    if (sourceUri.find(Prefixes) == 0) {
        assetsFilePath = DEFAULT_ASSETS_DEST + sourceUri.substr(Prefixes.length());
    }
    if (nativeResourceManager == nullptr) {
        DLOG(ERROR) << "ReadAssetsData env error ;";
    }
    RawFile *_file = OH_ResourceManager_OpenRawFile(nativeResourceManager, assetsFilePath.c_str());
    size_t length = OH_ResourceManager_GetRawFileSize(_file);
    std::unique_ptr<char[]> mediaData = std::make_unique<char[]>(length);
    int rawFileOffset = OH_ResourceManager_ReadRawFile(_file, mediaData.get(), length);
    DLOG(INFO) << "ReadAssetsData assetsFilePath=" << assetsFilePath + " rawFileOffset=" << rawFileOffset
               << " length=" << length;
    std::vector<uint8_t> vec(length);
    std::memcpy(vec.data(), mediaData.get(), length);

    OH_ResourceManager_CloseRawFile(_file);

    return vec;
}

std::vector<uint8_t> HarmonyPlatformContext::PerformHTTPRequest(const std::string &sourceUri) {
    std::string Prefixes = "http://localhost";
    if (sourceUri.find(Prefixes) == 0) {
        std::size_t questionMarkPos = sourceUri.find('?');
        std::string pathWithParams =
            (questionMarkPos != std::string::npos) ? sourceUri.substr(0, questionMarkPos) : sourceUri;
        std::size_t thirdSlashPos =
            pathWithParams.find('/', pathWithParams.find('/', pathWithParams.find('/') + 1) + 1);
        if (thirdSlashPos != std::string::npos) {
            return ReadAssetsData(pathWithParams.substr(thirdSlashPos + 1));
        }
    }
    std::vector<uint8_t> buffer;
    reinterpret_cast<char *>(buffer.data());
    Rcp_Response *response;
    const char *kHttpServerAddress = sourceUri.c_str();
    Rcp_Request *request = HMS_Rcp_CreateRequest(kHttpServerAddress);
    request->method = RCP_METHOD_GET;

    uint32_t errCode = 0;
    // 创建session
    Rcp_Session *session = HMS_Rcp_CreateSession(NULL, &errCode);
    // 配置请求回调
    Rcp_Response *ctx = nullptr;
    // Rcp_ResponseCallbackObject responseCallback = {ResponseCallback, request};
    //  发起请求
    response = HMS_Rcp_FetchSync(session, request, &errCode);

    if (response != nullptr) {
        buffer.resize(response->body.length);
        const char *data = response->body.buffer;
        std::copy(data, data + response->body.length, buffer.begin());
    }
    DLOG(INFO) << "PerformHTTPRequest HMS_Rcp_FetchSync errCode:" << errCode;

    // 在退出前取消可能还在执行的requests
    errCode = HMS_Rcp_CancelSession(session);

    // 清理request
    HMS_Rcp_DestroyRequest(request);
    // 关闭session
    errCode = HMS_Rcp_CloseSession(&session);

    return buffer;
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

void HarmonyPlatformContext::setNativeResourceManager(const NativeResourceManager *nativeResMgr) {
    this->nativeResourceManager = nativeResMgr;
}

void HarmonyPlatformContext::runOnDrawThread(std::function<void()> task){
    playLink->runOnDrawThread(task);
}

} // namespace RNSkia