#pragma once

#include <cstddef>
#include <utility>

namespace ministl
{
    template <class T>
    struct unique_ptr
    {

    private:
        T *data_ = nullptr;

    public:
        unique_ptr() noexcept = default;
        unique_ptr(unique_ptr &&oth) noexcept
        {
            data_ = oth.release();
        }
        unique_ptr(const unique_ptr &oth) = delete;
        explicit unique_ptr(T *pointer) noexcept
        {
            data_ = pointer;
        }
        ~unique_ptr() noexcept
        {
            delete data_;
        }
        unique_ptr &operator=(unique_ptr &&oth) noexcept
        {
            if (this != &oth)
            {
                reset(oth.release());
            }
            return *this;
        }
        unique_ptr &operator=(const unique_ptr &oth) = delete;
        T &operator*() const noexcept
        {
            return *(data_);
        }
        T *operator->() const noexcept { return data_; }
        T *get() const noexcept { return data_; }
        T *release() noexcept
        {
            T *cop;
            cop = data_;
            data_ = nullptr;
            return cop;
        }
        explicit operator bool() const noexcept
        {
            return (data_ != nullptr);
        }
        void reset(T *newpointer = nullptr) noexcept
        {
            if (newpointer == data_)
            {
                return;
            }
            delete data_;
            data_ = newpointer;
        }
        void swap(unique_ptr &oth) noexcept
        {
            std::swap(data_, oth.data_);
        }
    };
    template <class T, class... Args>
    unique_ptr<T> make_unique(Args &&...args)
    {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
