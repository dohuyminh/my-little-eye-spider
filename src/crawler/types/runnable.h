#pragma once

#include <atomic>
#include <thread>
#include <mutex>

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
    std::mutex runMutex_;
    bool isRunning_{ false };

};

}

}