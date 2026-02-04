#pragma once

#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <type_traits>

namespace amemory::meta
{
    class thread_pool
    {
    public:
        explicit thread_pool(std::size_t thread_count = std::thread::hardware_concurrency())
            : stop_(false)
        {
            if (thread_count == 0)
                thread_count = 1;

            workers_.reserve(thread_count);
            for (std::size_t i = 0; i < thread_count; ++i)
            {
                workers_.emplace_back([this] {
                    worker_loop();
                });
            }
        }

        thread_pool(const thread_pool&) = delete;
        thread_pool& operator=(const thread_pool&) = delete;

        ~thread_pool()
        {
            shutdown();
        }

        template <class F, class... Args>
        auto submit(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>>
        {
            using result_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<result_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<result_type> res = task->get_future();

            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (stop_)
                    throw std::runtime_error("thread_pool is stopped");

                tasks_.emplace([task]() {
                    (*task)();
                });
            }

            cv_.notify_one();
            return res;
        }

        template <class F>
        void submit_detached(F&& f)
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (stop_)
                    return;

                tasks_.emplace(std::forward<F>(f));
            }
            cv_.notify_one();
        }

        void shutdown()
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (stop_)
                    return;
                stop_ = true;
            }

            cv_.notify_all();
            for (auto& t : workers_)
            {
                if (t.joinable())
                    t.join();
            }
        }

    private:
        void worker_loop()
        {
            while (true)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                    });

                    if (stop_ && tasks_.empty())
                        return;

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                task();
            }
        }

    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;

        std::mutex mutex_;
        std::condition_variable cv_;
        bool stop_;
    };
}