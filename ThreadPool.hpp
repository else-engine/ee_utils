/**
 * Copyright (c) 2017-2018 Gauthier ARNOULD
 * This file is released under the zlib License (Zlib).
 * See file LICENSE or go to https://opensource.org/licenses/Zlib
 * for full license details.
 */

#pragma once

#include <cstddef>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <future>
#include <atomic>

namespace ee {

class ThreadPool {
    public:
        ThreadPool(size_t = 0);
        ~ThreadPool();

        void spawn(size_t);

        template <class Fn, class... Args>
        auto arun(Fn&& fn, Args&&... args) -> std::future<decltype(fn(std::size_t{}, args...))>;

        template <class Fn, class... Args>
        auto run(Fn&& fn, Args&&... args) -> decltype(fn(std::size_t{}, args...));

        void assist();
        void wait_completion();
        void join();

        std::size_t worker_count() const;

    private:
        std::atomic<bool> join_{false};
        std::atomic<unsigned int> busy_{0};

        std::vector<std::thread> workers_;
        std::queue<std::function<void(std::size_t)>> queue_;
        std::condition_variable wakeup_;
        std::condition_variable task_completed_;
        std::mutex mutex_wakeup_;
        std::mutex mutex_task_completed_;
};

template <class Fn, class... Args>
auto ThreadPool::arun(Fn&& fn, Args&&... args) -> std::future<decltype(fn(std::size_t{}, args...))> {
    using return_type = typename std::result_of<Fn(std::size_t, Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type(std::size_t)>>(
        std::bind(std::forward<Fn>(fn),
                  std::placeholders::_1,
                  std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(mutex_wakeup_);

        if ( ! join_) {
            queue_.emplace([task](std::size_t thread_id){ (*task)(thread_id); });
            wakeup_.notify_one();
        }
    }

    return res;
}

template <class Fn, class... Args>
auto ThreadPool::run(Fn&& fn, Args&&... args) -> decltype(fn(std::size_t{}, args...)) {
    auto res = arun(std::forward<Fn>(fn), std::forward<Args>(args)...);

    return res.get();
}

template <typename Fn>
void split_for(ThreadPool& pool,
               std::size_t count, std::size_t stride, std::size_t split, std::size_t at_least,
               Fn&& fn) {
    std::size_t batch = std::max((count / split) + ((count % split) ? 1 : 0), at_least);

    for (std::size_t i{0}; i < count; i += batch) {
        std::size_t sub_count = stride * std::min(i + batch, count);

        pool.arun([&fn, i, sub_count, stride](std::size_t thread_id) {
            for (std::size_t j = i * stride; j < sub_count; j += stride) {
                fn(thread_id, j);
            }
        });
    }

    pool.assist();
    pool.wait_completion();
}

} // namespace ee