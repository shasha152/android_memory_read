#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace amemory::meta
{
    class buffer_pool
    {
    public:
        using buffer_type = std::unique_ptr<std::uint8_t[]>;

        explicit buffer_pool(std::size_t buffer_size, std::size_t buffer_count)
            : buffer_size_(buffer_size), buffer_count_(buffer_count)
        {
            buffers.reserve(buffer_count);

            for (std::size_t i = 0; i < buffer_count; ++i)
            {
                buffers.emplace_back(
                    std::make_unique<std::uint8_t[]>(buffer_size_)
                );
            }
        }

        std::size_t size() const
        {
            return buffer_size_;
        }

        std::size_t buffer_count() const noexcept
        {
            return buffer_count_;
        }

        std::size_t buffer_size() const noexcept
        {
            return buffer_size_;
        }

        buffer_type acquire()
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [&] { return !buffers.empty(); });

            buffer_type buf = std::move(buffers.back());
            buffers.pop_back();
            return std::move(buf);
        }

        template <typename WaitNotice>
        buffer_type acquire(WaitNotice&& notice)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (buffers.empty())
            {
                lock.unlock();
                std::forward<WaitNotice>(notice)();
                lock.lock();
            }
            condition.wait(lock, [&] { return !buffers.empty(); });

            buffer_type buf = std::move(buffers.back());
            buffers.pop_back();
            return std::move(buf);
        }

        std::optional<buffer_type> try_acquire() noexcept
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (buffers.empty())
                return std::nullopt;
            buffer_type buf = std::move(buffers.back());
            buffers.pop_back();
            return std::move(buf);
        }

        void release(buffer_type buffer)
        {
            std::lock_guard<std::mutex> lock(mutex);
            buffers.emplace_back(std::move(buffer));
            condition.notify_one();
        }

    private:
        std::vector<buffer_type> buffers;
        mutable std::mutex mutex;
        std::condition_variable condition;

        std::size_t buffer_size_;
        std::size_t buffer_count_;
    };
}