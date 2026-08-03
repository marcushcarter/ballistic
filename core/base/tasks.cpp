#include <core/base/tasks.h>

namespace ballistic {

void TaskSystem::start(uint32_t p_worker_count)
{
    if (running) return;
    running = true;
    workers.reserve(p_worker_count);
    for (uint32_t i = 0; i < p_worker_count; i++)
        workers.emplace_back([this]{ _worker_loop(); });
}

void TaskSystem::stop()
{
    { std::lock_guard lock(mutex); running = false; }
    cv.notify_all();
    for (auto& t : workers) if (t.joinable()) t.join();
    workers.clear();
    queue.clear();
}

TaskSystem::Handle TaskSystem::dispatch(std::function<void()> fn)
{
    Handle counter = std::make_shared<std::atomic<uint32_t>>(1);
    { std::lock_guard lock(mutex); queue.push_back(Task{ std::move(fn), counter }); }
    cv.notify_one();
    return counter;
}

void TaskSystem::parallel_for(uint32_t p_count, std::function<uint32_t(uint32_t)>& fn)
{
    if (p_count == 0) return;
    uint32_t chunks = workers.empty() ? 1u : (uint32_t)workers.size();
    if (chunks > p_count) chunks = p_count;
    uint32_t per = (p_count + chunks - 1) / chunks;

    Handle counter = std::make_shared<std::atomic<uint32_t>>(chunks);
    {
        std::lock_guard lock(mutex);
        for (uint32_t c = 0; c < chunks; c++) {
            uint32_t begin = c * per;
            uint32_t end = begin + per > p_count ? p_count : begin + per;
            queue.push_back(Task{ [&fn, begin, end]{ for (uint32_t i = begin; i < end; i++) fn(i); }, counter });
        }
    }
    cv.notify_all();
    wait(counter);
}

void TaskSystem::wait(const Handle& handle)
{
    while (handle->load(std::memory_order_acquire) != 0)
        if (!_try_run_one()) std::this_thread::yield();
}

bool TaskSystem::_try_run_one()
{
    Task task;
    {
        std::lock_guard lock(mutex);
        if (queue.empty()) return false;
        task = std::move(queue.front());
        queue.pop_front();
    }
    task.fn();
    task.counter->fetch_sub(1, std::memory_order_release);
    return true;
}

void TaskSystem::_worker_loop()
{
    for (;;) {
        Task task;
        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [this]{ return !queue.empty() || !running; });
            if (queue.empty()) { if (!running) return; continue; }
            task = std::move(queue.front());
            queue.pop_front();
        }
        task.fn();
        task.counter->fetch_sub(1, std::memory_order_release);
    }
}

}