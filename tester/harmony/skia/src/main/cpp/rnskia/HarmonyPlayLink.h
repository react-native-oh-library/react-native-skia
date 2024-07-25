#ifndef HARMONY_PLAY_LINK_H
#define HARMONY_PLAY_LINK_H

#include <chrono>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>

namespace RNSkia {
class PlayLink {
public:
    PlayLink(std::function<void(double)> CallBack, double interval_ms = 16.667);
    ~PlayLink() {}

    void startDrawLoop();
    void stopDrawLoop();

private:
    std::function<void(double)> CallBack;
    std::chrono::steady_clock::duration interval;
    std::unique_ptr<std::thread> thread;
    std::atomic<bool> running = {false};

    void postFrameLoop();
};

} // namespace RNSkia
#endif // HARMONY_DISPLAY_LINK_H