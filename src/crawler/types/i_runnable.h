#pragma once

namespace crawler {

class IRunnable {
protected:
    
    virtual void run() = 0;

    bool isRunning() const noexcept {
        return isRunning_;
    }

    virtual void stop() = 0;

    bool isRunning_{false};

};

}