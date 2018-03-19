/**
 * Copyright (c) 2017-2018 Gauthier ARNOULD
 * This file is released under the zlib License (Zlib).
 * See file LICENSE or go to https://opensource.org/licenses/Zlib
 * for full license details.
 */

#include "ThreadPool.hpp"

namespace ee {

ThreadPool::ThreadPool(std::size_t nt) {
    spawn(nt);
}

ThreadPool::~ThreadPool() {
    join();
}

void ThreadPool::spawn(std::size_t nt) {
    for (size_t i = 0; i < nt; ++ i) {
        const std::size_t thread_id = workers_.size();

        workers_.emplace_back([this, thread_id] {
            while ( ! join_) {
                std::function<void(std::size_t)> f;

                {
                    std::unique_lock<std::mutex> lock(mutex_wakeup_);

                    wakeup_.wait(
                        lock,
                        [this] { return join_ || ! queue_.empty(); });

                    if (join_ && queue_.empty()) {
                        return;
                    }

                    ++ busy_;

                    f = std::move(queue_.front());
                    queue_.pop();
                }

                f(thread_id);

                {
                    std::unique_lock<std::mutex> lock(mutex_task_completed_);

                    -- busy_;

                    task_completed_.notify_all();
                }
            }
        });
    }
}

void ThreadPool::assist() {
    const std::size_t thread_id = workers_.size();

    while (true) {
        std::function<void(std::size_t)> f;

        {
            std::unique_lock<std::mutex> lock(mutex_wakeup_);

            if (queue_.empty()) {
                return;
            }

            ++busy_;

            f = std::move(queue_.front());
            queue_.pop();
        }

        f(thread_id);

        {
            std::unique_lock<std::mutex> lock(mutex_task_completed_);
            --busy_;
            task_completed_.notify_all();
        }
    }
}

void ThreadPool::wait_completion() {
    std::unique_lock<std::mutex> lock(mutex_task_completed_);
    task_completed_.wait(lock, [this] { return queue_.empty() && busy_ == 0; });
}

void ThreadPool::join() {
    join_ = true;

    wakeup_.notify_all();

    for (auto& worker : workers_) {
        worker.join();
    }

    workers_.clear();

    join_ = false;
}

std::size_t ThreadPool::worker_count() const {
    return workers_.size();
}

} // namespace ee