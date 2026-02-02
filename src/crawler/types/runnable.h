#pragma once

#include <atomic>
#include <thread>

namespace crawler {

namespace types {

class Runnable {
public:

    void run();

    virtual void runImpl() = 0;

    bool isRunning() const noexcept {
        return isRunning_.load();
    }

    void stop() noexcept;
    
    ~Runnable() = default;

private:
    std::thread eventLoopThread_;
    std::atomic<bool> isRunning_{ false };
};

}

}