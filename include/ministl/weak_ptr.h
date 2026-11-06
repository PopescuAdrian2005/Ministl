#pragma once
#include "detail/control_block.h"
#include "shared_ptr.h"
#include <cstddef>
#include <utility>
#include <concepts>

namespace ministl{

template <class T>
struct weak_ptr
{
    private:
    T *data_ = nullptr;
    detail::control_block *cblockptr = nullptr;
    public:
    weak_ptr() noexcept = default;
    template <class U>
    requires std::convertible_to<U*,T*>
    weak_ptr(const shared_ptr<U> &oth) noexcept
    {
        data_ = oth.data_;
        cblockptr = oth.cblockptr;
        if (cblockptr != nullptr)
        {
            cblockptr->add_weak();
        }
    }
    template <class U>
    weak_ptr(T *nptr, const shared_ptr<U> &oth) noexcept
    {
        data_ = nptr;
        cblockptr = oth.cblockptr;
        if (cblockptr != nullptr)
        {
            cblockptr->add_weak();
        }
    }
    weak_ptr(const weak_ptr<T> &oth) noexcept
    {
        data_ = oth.data_;
        cblockptr = oth.cblockptr;
        if (cblockptr != nullptr)
        {
            cblockptr->add_weak();
        }
    }
    weak_ptr(weak_ptr<T> &&oth) noexcept
    {
        data_ = std::exchange(oth.data_, nullptr);
        cblockptr = std::exchange(oth.cblockptr, nullptr);
    }
    ~weak_ptr()
    {
        release_weak();
    }
    void release_weak() noexcept
    {
        if (cblockptr != nullptr)
        {
            cblockptr->release_weak();
        }
        cblockptr = nullptr;
        data_ = nullptr;
    }
    weak_ptr &operator=(const shared_ptr<T> &oth) noexcept
    {
        if (cblockptr != nullptr)
        {
            cblockptr->release_weak();
        }
        data_ = oth.data_;
        cblockptr = oth.cblockptr;
        if (cblockptr != nullptr)
        {
            cblockptr->add_weak();
        }
        return *this;
    }
    weak_ptr &operator=(const weak_ptr<T> &oth) noexcept
    {
        if (this == &oth)
        {
            return *this;
        }
        if (cblockptr != nullptr)
        {
            cblockptr->release_weak();
        }
        data_ = oth.data_;
        cblockptr = oth.cblockptr;
        if (cblockptr != nullptr)
        {
            cblockptr->add_weak();
        }
        return *this;
    }
    weak_ptr &operator=(weak_ptr<T> &&oth) noexcept
    {
        if (this == &oth)
        {
            return *this;
        }
        if (cblockptr != nullptr)
        {
            cblockptr->release_weak();
        }
        data_ = std::exchange(oth.data_, nullptr);
        cblockptr = std::exchange(oth.cblockptr, nullptr);
        return *this;
    }
    void reset() noexcept
    {
        release_weak();
    }
    void swap(weak_ptr &oth) noexcept
    {
        std::swap(data_, oth.data_);
        std::swap(cblockptr, oth.cblockptr);
    }
    std::size_t use_count() const noexcept
    {
        if (cblockptr == nullptr)
        {
            return 0;
        }
        return cblockptr->get_strong();
    }
    bool expired() const noexcept
    {
        return use_count() == 0;
    }
    shared_ptr<T> lock() const noexcept
    {
        if (cblockptr == nullptr)
        {
            return {};
        }
        if (cblockptr->try_add_strong() == false)
        {
            return {};
        }
        return shared_ptr<T>(data_, cblockptr);
    }
};
}
