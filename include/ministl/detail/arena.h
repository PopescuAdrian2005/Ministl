#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <new>

namespace ministl::detail
{
    class arena
    {
    private:
        std::byte *memory_ = nullptr;
        std::size_t remaining_ = 0;
        std::size_t offset_ = 0;

    public:
        arena() = default;

        explicit arena(std::size_t size)
        {
            if (size == 0)
            {
                return;
            }

            memory_ = static_cast<std::byte *>(std::malloc(size));
            if (memory_ == nullptr)
            {
                throw std::bad_alloc{};
            }
            remaining_ = size;
        }

        arena(const arena &) = delete;
        arena &operator=(const arena &) = delete;

        ~arena()
        {
            std::free(memory_);
        }

        [[nodiscard]] void *allocate(
            std::size_t size,
            std::size_t alignment)
        {
            if (size == 0)
            {
                return nullptr;
            }
            if (size > remaining_)
            {
                throw std::bad_alloc{};
            }

            void *current = memory_ + offset_;
            void *aligned = std::align(alignment, size, current, remaining_);
            if (aligned == nullptr)
            {
                throw std::bad_alloc{};
            }

            const auto aligned_offset = static_cast<std::size_t>(
                static_cast<std::byte *>(aligned) - memory_);
            remaining_ -= size;
            offset_ = aligned_offset + size;
            return aligned;
        }
    };
}
