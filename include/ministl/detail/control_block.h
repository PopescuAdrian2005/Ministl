#pragma once

#include <cstddef>
#include <utility>
#include <atomic>

namespace ministl::detail
{
    struct control_block
    {
        using deleter_type = void (*)(void *) noexcept;
        deleter_type deleter;
        void *ptr = nullptr;
        std::atomic<std::size_t> strong_count = 0;
        std::atomic<std::size_t> weak_count = 0;
        ~control_block() = default;
        control_block(void *nptr, deleter_type function)
        {
            strong_count.fetch_add(1);
            weak_count.fetch_add(1);
            ptr = nptr;
            deleter = function;
        }
        void release_weak() noexcept
        {
            if (weak_count.fetch_sub(1) == 1)
            {
                delete this;
                return;
            }
        }
        void release_strong() noexcept
        {
            if (strong_count.fetch_sub(1) == 1)
            {
                deleter(ptr);
                ptr = nullptr;
                release_weak();
            }
        }
        void add_strong() noexcept
        {
            strong_count.fetch_add(1);
        }
        void add_weak() noexcept
        {
            weak_count.fetch_add(1);
        }
        bool try_add_strong() noexcept
        {
            std::size_t exp = strong_count.load();
            while (exp != 0)
            {
                if (strong_count.compare_exchange_weak(exp, exp + 1))
                {
                    return true;
                }
            }
            return false;
        }
        std::size_t get_strong() const noexcept { return strong_count.load(); };
    };
}
