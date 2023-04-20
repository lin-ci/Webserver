#pragma once
#include"Common.h"
#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<functional>
#include<atomic>
#include<future>
class ThreadPool {
public:
    explicit ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    ~ThreadPool();
    template<typename F, typename... Args>
    auto add(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;
private:
    std::mutex queue_mutex_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::condition_variable cv_;
    std::atomic<bool>stop_{false};
};
template <class F, class... Args>
auto ThreadPool::add(F&& f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.push([task]() { (*task)(); });
    }
    cv_.notify_one();
    return res;
}