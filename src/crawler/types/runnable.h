#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>

namespace crawler {

namespace types {

class Runnable {
public:

    void run();

    virtual void runImpl() = 0;

    bool isRunning() const noexcept {
        return isRunning_;
    }

    void stop() noexcept;
    
    ~Runnable() = default;

private:
    std::thread eventLoopThread_;
    bool isRunning_{ false };
    std::mutex runMutex_;
    std::condition_variable runCv_;
};

}

}