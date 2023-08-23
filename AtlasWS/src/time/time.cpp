#include "./time.hpp"

namespace atlas {

    uint64_t time() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        return milliseconds.count();
    }

}