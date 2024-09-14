
#include "HarmonyPlayLink.h"
#include "RNSkPlatformContext.h"
#include <mutex>
#include <glog/logging.h>

namespace RNSkia {
// 假设屏幕刷新率为16.667ms
PlayLink::PlayLink(std::function<void(double)> CallBack, double interval_ms)
    : CallBack(CallBack), interval(std::chrono::milliseconds(static_cast<int>(std::round(interval_ms)))) {}

void PlayLink::startDrawLoop() {
    if (!running) {
        running = true;
        thread = std::make_unique<std::thread>(&PlayLink::postFrameLoop, this);
    }
}

void PlayLink::stopDrawLoop() {
    if (running) {
        running = false;
        if (thread->joinable()) {
            thread->join();
        }
        thread.reset(); // 线程销毁OK
    }
}
// 在后台线程上创建循环
void PlayLink::postFrameLoop() {
    auto lastTime = std::chrono::steady_clock::now();
    auto nextTime = lastTime + interval;
    while (running) {
        std::this_thread::sleep_until(nextTime);

        auto currentTime = std::chrono::steady_clock::now();
        // 延迟时间
        double deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();
        lastTime = currentTime; // 更新 lastTime 为当前时间

        CallBack(deltaTime);
        // 重新计算下一次运行的时间点
        nextTime = lastTime + interval;
        
        while (!taskQueue.empty()) {
            DLOG(INFO) << "postFrameLoop run task running:" << running;
            auto task = std::move(taskQueue.front());
            taskQueue.pop();
            task(); // 执行任务
        }
    }
}

void PlayLink::runOnDrawThread(std::function<void()> task) {
    DLOG(INFO) << "runOnDrawThread running:" << running;
    if (!running) {
        startDrawLoop();
    }
    taskQueue.push(std::move(task));
}


} // namespace RNSkia