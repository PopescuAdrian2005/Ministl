
#pragma once
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>

namespace ministl
{
    template <class T>
    struct malloc_allocator
    {
        using value_type = T;
        malloc_allocator() noexcept = default;
        template <class U>
        malloc_allocator(const malloc_allocator<U> &) noexcept {}
        [[nodiscard]]
        T *allocate(std::size_t size)
        {
            if (size == 0)
            {
                return nullptr;
            }
            if (size > std::numeric_limits<std::size_t>::max() / sizeof(T))
            {
                throw std::bad_array_new_length{};
            }
            T *ptr = static_cast<T *>(std::malloc(size * sizeof(T)));
            if (ptr == nullptr)
            {
                throw std::bad_alloc{};
            }
            return ptr;
        }
        void deallocate(T *ptr, std::size_t) noexcept
        {
            std::free(ptr);
        }
    };

    template <class T, class U>
    bool operator==(const malloc_allocator<T> &, const malloc_allocator<U> &) noexcept
    {
        return true;
    }
}