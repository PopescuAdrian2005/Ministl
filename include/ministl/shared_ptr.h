#pragma once
#include "detail/control_block.h"
#include <cstddef>
#include <utility>
#include <concepts>

namespace ministl
{
    template <class T>
    struct shared_ptr
    {
    private:
        T *data_ = nullptr;
        detail::control_block *cblockptr = nullptr;
        shared_ptr(T *ptr, detail::control_block *ncblockptr) noexcept
        {
            data_ = ptr;
            cblockptr = ncblockptr;
        }
        template <class>
        friend struct shared_ptr;

        template <class>
        friend struct weak_ptr;

    public:
        shared_ptr() noexcept = default;
        template <class U>
            requires std::convertible_to<U *, T *>
        explicit shared_ptr(U *nptr)
        {
            data_ = static_cast<T *>(nptr);
            try
            {
                cblockptr = new detail::control_block(nptr, [](void *ptr) noexcept
                                                 { delete static_cast<U *>(ptr); });
            }
            catch (...)
            {
                delete nptr;
                data_ = nullptr;
                throw;
            }
        }
        template <class U>
        shared_ptr(const shared_ptr<U> &oth, T *alias_pointer) noexcept
        {
            data_ = alias_pointer;
            cblockptr = oth.cblockptr;
            if (cblockptr != nullptr)
            {
                cblockptr->add_strong();
            }
        }

        template <class U>
            requires std::convertible_to<U *, T *>
        shared_ptr(const shared_ptr<U> &oth) noexcept
        {
            data_ = oth.data_;
            cblockptr = oth.cblockptr;
            if (cblockptr != nullptr)
            {
                cblockptr->add_strong();
            }
        }
        template <class U>
            requires std::convertible_to<U *, T *>
        shared_ptr(shared_ptr<U> &&oth) noexcept
        {
            data_ = std::exchange(oth.data_, nullptr);
            cblockptr = std::exchange(oth.cblockptr, nullptr);
        }
        shared_ptr(const shared_ptr &oth) noexcept : data_(oth.data_), cblockptr(oth.cblockptr)
        {
            if (cblockptr != nullptr)
                cblockptr->add_strong();
        }
        shared_ptr(shared_ptr &&oth) noexcept
        {
            data_ = std::exchange(oth.data_, nullptr);
            cblockptr = std::exchange(oth.cblockptr, nullptr);
        }
        ~shared_ptr()
        {
            release_ownership();
        }
        void release_ownership() noexcept
        {
            detail::control_block *old_control = cblockptr;

            data_ = nullptr;
            cblockptr = nullptr;
            if (old_control != nullptr)
            {
                old_control->release_strong();
            }
        }
        T *get() const noexcept { return data_; };
        T &operator*() const noexcept { return *data_; };
        T *operator->() const noexcept { return data_; };
        explicit operator bool() const noexcept { return data_ != nullptr; };
        shared_ptr &operator=(const shared_ptr &oth) noexcept
        {
            if (&oth == this)
            {
                return *this;
            }
            release_ownership();
            data_ = oth.data_;
            cblockptr = oth.cblockptr;
            if (cblockptr != nullptr)
            {
                cblockptr->add_strong();
            }
            return *this;
        }
        shared_ptr &operator=(shared_ptr &&oth) noexcept
        {
            if (&oth == this)
            {
                return *this;
            }
            release_ownership();
            data_ = std::exchange(oth.data_, nullptr);
            cblockptr = std::exchange(oth.cblockptr, nullptr);
            return *this;
        }

        void reset() noexcept
        {
            release_ownership();
        }
        template <class U>
        void reset(U *ptr)
        {
            shared_ptr temp(ptr);
            swap(temp);
        }
        void swap(shared_ptr &oth) noexcept
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
    };

    template <class T, class... Args>
    shared_ptr<T> make_shared(Args &&...args)
    {
        return shared_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
