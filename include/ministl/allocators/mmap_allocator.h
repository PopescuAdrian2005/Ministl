
#pragma once
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <sys/mman.h>

namespace ministl
{

    template <class T>
    struct mmap_allocator
    {
        using value_type = T;
        mmap_allocator() noexcept = default;
        template <class U>
        mmap_allocator(const mmap_allocator<U> &) noexcept {}
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
            void *ptr = mmap(nullptr, size * sizeof(T), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr == MAP_FAILED)
            {
                throw std::bad_alloc{};
            }
            return static_cast<T *>(ptr);
        }
        void deallocate(T *ptr, std::size_t size) noexcept
        {
            if (ptr == nullptr || size == 0)
            {
                return;
            }
            munmap(ptr, size * sizeof(T));
        }
    };

    template <class T, class U>
    bool operator==(const mmap_allocator<T> &, const mmap_allocator<U> &) noexcept
    {
        return true;
    }

}