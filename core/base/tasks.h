#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace ballistic {

struct TaskSystem
{ 
    using Handle = std::shared_ptr<std::atomic<uint32_t>>;

    struct Task {
        std::function<void()> fn;
        Handle counter;
    };

    std::vector<std::thread> workers;
    std::deque<Task> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool running = false;

    void start(uint32_t worker);
    void stop();

    Handle dispatch(std::function<void()> fn);
    void parallel_for(uint32_t p_count, std::function<uint32_t(uint32_t)>& fn);
    void wait(const Handle& handle);

    bool _try_run_one();
    void _worker_loop();
};
    
}