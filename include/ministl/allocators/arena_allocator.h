#pragma once

#include "../detail/arena.h"
#include "../shared_ptr.h"

#include <cstddef>
#include <limits>
#include <new>

namespace ministl
{
    template <class T>
    class arena_allocator
    {
    public:
        using value_type = T;

    private:
        shared_ptr<detail::arena> arena_;

        template <class>
        friend class arena_allocator;

    public:
        arena_allocator() noexcept = default;

        explicit arena_allocator(std::size_t size)
            : arena_(make_shared<detail::arena>(size))
        {
        }

        arena_allocator(const arena_allocator &) noexcept = default;
        arena_allocator &operator=(const arena_allocator &) noexcept = default;

        template <class U>
        arena_allocator(const arena_allocator<U> &other) noexcept
            : arena_(other.arena_)
        {
        }

        [[nodiscard]] T *allocate(std::size_t size)
        {
            if (!arena_)
            {
                throw std::bad_alloc{};
            }
            if (size > std::numeric_limits<std::size_t>::max() / sizeof(T))
            {
                throw std::bad_array_new_length{};
            }

            return static_cast<T *>(
                arena_->allocate(size * sizeof(T), alignof(T)));
        }

        void deallocate(T *, std::size_t) noexcept
        {
        }

        void swap(arena_allocator &other) noexcept
        {
            arena_.swap(other.arena_);
        }

        template <class U>
        bool operator==(const arena_allocator<U> &other) const noexcept
        {
            return arena_.get() == other.arena_.get();
        }
    };
}
