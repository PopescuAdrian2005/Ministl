#pragma once
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <stdexcept>

namespace ministl
{
    template <class T, class Allocator = std::allocator<T>>
    struct vector
    {
        std::size_t size_ = 0, capacity_ = 0;
        T *data_ = nullptr;
        Allocator allocator_;
        vector() = default;
        vector(const vector &oth) : allocator_(oth.allocator_)
        {
            if (oth.empty())
            {
                return;
            }
            size_t constructed = 0;
            try
            {
                data_ = (T *)allocator_.allocate(oth.size());
                for (; constructed < oth.size(); constructed++)
                {
                    std::construct_at(data_ + constructed, oth[constructed]);
                }
            }
            catch (...)
            {
                for (; constructed > 0; constructed--)
                {
                    std::destroy_at(data_ + constructed - 1);
                }
                if (data_ != nullptr)
                {
                    allocator_.deallocate(data_, oth.size());
                }
                throw;
            }
            size_ = oth.size();
            capacity_ = oth.size();
        }
        vector(vector &&oth) : allocator_(oth.allocator_)
        {
            data_ = std::exchange(oth.data_, nullptr);
            size_ = std::exchange(oth.size_, 0);
            capacity_ = std::exchange(oth.capacity_, 0);
        }
        bool operator==(const vector &oth) const
        {
            if (oth.size_ != size_)
            {
                return false;
            }
            for (size_t it = 0; it < size_; it++)
            {
                if (data_[it] != oth[it])
                {
                    return false;
                }
            }
            return true;
        }
        template <class F>
        void construct_count(std::size_t count, F function)
        {
            if (count == 0)
            {
                return;
            }
            T *ndata_ = (T *)allocator_.allocate(count);
            std::size_t constructed = 0;
            try
            {
                for (; constructed < count; constructed++)
                {
                    function(ndata_ + constructed);
                }
            }
            catch (...)
            {
                for (; constructed > 0; constructed--)
                {
                    std::destroy_at(ndata_ + constructed - 1);
                }
                if (ndata_ != nullptr)
                {
                    allocator_.deallocate(ndata_, count);
                }
                throw;
            }
            data_ = ndata_;
            size_ = count;
            capacity_ = count;
        }
        explicit vector(std::size_t count)
        {
            construct_count(count, [](T *addrs)
                            { std::construct_at(addrs); });
        }
        vector(std::size_t count, const T &obj)
        {
            construct_count(count, [&obj](T *addrs)
                            { std::construct_at(addrs, obj); });
        }
        explicit vector(const Allocator &allocator) : allocator_(allocator) {}
        ~vector()
        {
            clear();
            if (data_ != nullptr)
                allocator_.deallocate(data_, capacity_);
        }
        void assign(std::size_t count, const T &value)
        {
            vector temporary(allocator_);
            temporary.resize(count, value);
            swap(temporary);
        }
        std::size_t size() const noexcept { return size_; }
        std::size_t capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return (size_ == 0); }
        T *data() noexcept { return data_; }
        const T *data() const noexcept { return data_; }
        T *begin() noexcept { return data_; }
        const T *begin() const noexcept { return data_; }
        T &operator[](size_t i) noexcept { return data_[i]; }
        const T &operator[](size_t i) const noexcept { return data_[i]; }
        Allocator get_allocator() const { return allocator_; }
        const T *cbegin() const noexcept { return data_; }
        const T *cend() const noexcept { return data_ + size_; }
        T &at(std::size_t i)
        {
            if (i >= size_)
            {
                throw std::out_of_range("index out of range");
            }
            return data_[i];
        }
        const T &at(size_t i) const
        {
            if (i >= size_)
            {
                throw std::out_of_range("index out of range");
            }
            return data_[i];
        }
        T &front()
        {
            return *begin();
        }
        const T &front() const
        {
            return *begin();
        }
        T &back()
        {
            return *(begin() + size_ - 1);
        }
        const T &back() const
        {
            return *(begin() + size_ - 1);
        }
        T *end() noexcept
        {
            if (data_ == nullptr)
                return data_;
            return data_ + size_;
        }
        const T *end() const noexcept
        {
            if (data_ == nullptr)
                return data_;
            return data_ + size_;
        }
        vector &operator=(vector &&oth)
        {
            if (this == &oth)
            {
                return *this;
            }
            clear();
            if (data_ != nullptr)
                allocator_.deallocate(data_, capacity_);
            std::swap(allocator_, oth.allocator_);
            data_ = std::exchange(oth.data_, nullptr);
            size_ = std::exchange(oth.size_, 0);
            capacity_ = std::exchange(oth.capacity_, 0);
            return *this;
        }
        vector &operator=(const vector &oth)
        {
            if (&oth == this)
            {
                return *this;
            }
            std::size_t constructed = 0;
            T *ndata_ = nullptr;
            try
            {
                ndata_ = (T *)allocator_.allocate(oth.size());
                for (constructed = 0; constructed < oth.size(); constructed++)
                {
                    std::construct_at(ndata_ + constructed, oth[constructed]);
                }
            }
            catch (...)
            {
                for (; constructed > 0; constructed--)
                {
                    std::destroy_at(ndata_ + constructed - 1);
                }
                if (ndata_ != nullptr)
                    allocator_.deallocate(ndata_, oth.size());
                throw;
            }
            clear();
            if (data_ != nullptr)
                allocator_.deallocate(data_, capacity_);
            data_ = ndata_;
            size_ = oth.size();
            capacity_ = oth.size();
            return *this;
        }
        void reserve(std::size_t newcap)
        {
            if (newcap <= capacity_)
                return;
            std::size_t constructed = 0;
            T *ndata_ = nullptr;
            try
            {
                ndata_ = (T *)allocator_.allocate(newcap);
                for (constructed = 0; constructed < size_; constructed++)
                {
                    std::construct_at(ndata_ + constructed, std::move_if_noexcept(data_[constructed]));
                }
            }
            catch (...)
            {
                for (; constructed > 0; constructed--)
                {
                    std::destroy_at(ndata_ + constructed - 1);
                }
                if (ndata_ != nullptr)
                {
                    allocator_.deallocate(ndata_, newcap);
                }
                throw;
            }
            for (std::size_t i = size_; i > 0; i--)
            {
                std::destroy_at(data_ + i - 1);
            }
            if (data_ != nullptr)
                allocator_.deallocate(data_, capacity_);
            data_ = ndata_;
            capacity_ = newcap;
        }
        template <class... Args>
        void emplace_push(std::size_t newcap, Args &&...args)
        {
            if (newcap <= capacity_)
                return;
            std::size_t constructed = 0;
            T *ndata_ = nullptr;
            try
            {
                ndata_ = (T *)allocator_.allocate(newcap);
                std::construct_at(ndata_ + size_, std::forward<Args>(args)...);
                constructed++;
                for (; constructed <= size_; constructed++)
                {
                    std::construct_at(ndata_ + constructed - 1, std::move_if_noexcept(data_[constructed - 1]));
                }
            }
            catch (...)
            {
                for (; constructed > 1; constructed--)
                {
                    std::destroy_at(ndata_ + constructed - 2);
                }
                if (constructed > 0)
                {
                    std::destroy_at(ndata_ + size_);
                    constructed--;
                }
                if (ndata_ != nullptr)
                {
                    allocator_.deallocate(ndata_, newcap);
                }
                throw;
            }
            for (std::size_t i = size_; i > 0; i--)
            {
                std::destroy_at(data_ + i - 1);
            }
            if (data_ != nullptr)
                allocator_.deallocate(data_, capacity_);
            data_ = ndata_;
            capacity_ = newcap;
        }
        void push_back(T &&obj)
        {
            emplace_back(std::move(obj));
        }
        void push_back(const T &obj)
        {
            emplace_back(obj);
        }
        template <class... Args>
        T &emplace_back(Args &&...args)
        {
            if (capacity_ > size_)
            {
                std::construct_at(data_ + size_, std::forward<Args>(args)...);
            }
            else
            {
                emplace_push(capacity_ * 2 + 1, std::forward<Args>(args)...);
            }
            size_++;
            return back();
        }
        template <class... Args>
        T *emplace(std::size_t pos, Args &&...args)
        {
            if (pos > size_)
            {
                throw std::out_of_range("emplace position out of range");
            }
            emplace_back(std::forward<Args>(args)...);
            T tmp(std::move(data_[size_ - 1]));
            for (std::size_t right = size_ - 1; right > pos; right--)
            {
                data_[right] = std::move(data_[right - 1]);
            }
            data_[pos] = std::move(tmp);
            return data_ + pos;
        }
        T *insert(std::size_t pos, const T &obj)
        {
            return emplace(pos, obj);
        }
        T *insert(std::size_t pos, T &&obj)
        {
            return emplace(pos, std::move(obj));
        }
        T *insert(T *ptr, const T &obj)
        {
            return emplace(static_cast<std::size_t>(ptr - data_), obj);
        }
        T *insert(T *ptr, T &&obj)
        {
            return emplace(static_cast<std::size_t>(ptr - data_), std::move(obj));
        }
        void pop_back()
        {
            assert(size_ >= 1);
            size_--;
            std::destroy_at(data_ + size_);
        }
        void clear()
        {
            for (std::size_t i = size_; i > 0; i--)
            {
                std::destroy_at(data_ + i - 1);
            }
            size_ = 0;
        }
        template <class F>
        void resize_impl(std::size_t newsize, F function)
        {
            if (newsize < size_)
            {
                for (; size_ > newsize; size_--)
                {
                    std::destroy_at(data_ + size_ - 1);
                }
            }
            else if (newsize <= capacity_)
            {
                std::size_t constructed = 0;
                try
                {
                    for (; constructed + size_ < newsize; constructed++)
                    {
                        function(data_ + size_ + constructed);
                    }
                }
                catch (...)
                {
                    for (; constructed > 0; constructed--)
                    {
                        std::destroy_at(data_ + size_ + constructed - 1);
                    }
                    throw;
                }
                size_ = newsize;
            }
            else
            {
                std::size_t newcons = 0, oldcons = 0;
                T *ndata_ = (T *)allocator_.allocate(2 * newsize + 1);
                try
                {
                    for (; newcons < newsize - size_; newcons++)
                    {
                        function(ndata_ + size_ + newcons);
                    }
                    for (; oldcons < size_; oldcons++)
                    {
                        std::construct_at(ndata_ + oldcons, std::move_if_noexcept(data_[oldcons]));
                    }
                }
                catch (...)
                {
                    for (; oldcons > 0; oldcons--)
                    {
                        std::destroy_at(ndata_ + oldcons - 1);
                    }
                    for (; newcons > 0; newcons--)
                    {
                        std::destroy_at(ndata_ + size_ + newcons - 1);
                    }
                    allocator_.deallocate(ndata_, 2 * newsize + 1);
                    throw;
                }
                for (; size_ > 0; size_--)
                {
                    std::destroy_at(data_ + size_ - 1);
                }
                if (data_ != nullptr)
                {
                    allocator_.deallocate(data_, capacity_);
                }
                capacity_ = 2 * newsize + 1;
                size_ = newsize;
                data_ = ndata_;
            }
        }
        void resize(std::size_t newsize)
        {
            resize_impl(newsize, [](T *adrs)
                        { std::construct_at(adrs); });
        }
        void resize(std::size_t newsize, const T &obj)
        {
            resize_impl(newsize, [&obj](T *adrs)
                        { std::construct_at(adrs, obj); });
        }
        void swap(vector &oth)
        {
            std::swap(size_, oth.size_);
            std::swap(capacity_, oth.capacity_);
            std::swap(allocator_, oth.allocator_);
            std::swap(data_, oth.data_);
        }
        void shrink_to_fit()
        {
            if (capacity_ == size_)
            {
                return;
            }
            size_t newsize = size_;
            std::size_t constructed = 0;
            T *ndata_ = (T *)allocator_.allocate(size_);
            ;
            try
            {
                for (; constructed < size_; constructed++)
                {
                    std::construct_at(ndata_ + constructed, std::move_if_noexcept(data_[constructed]));
                }
            }
            catch (...)
            {
                for (; constructed > 0; constructed--)
                {
                    std::destroy_at(ndata_ + constructed - 1);
                }
                if (ndata_ != nullptr)
                {
                    allocator_.deallocate(ndata_, size_);
                }
                throw;
            }
            for (; size_ > 0; size_--)
            {
                std::destroy_at(data_ + size_ - 1);
            }
            if (data_ != nullptr)
            {
                allocator_.deallocate(data_, capacity_);
            }
            data_ = ndata_;
            size_ = newsize;
            capacity_ = newsize;
        }
        T *erase(const T *first, const T *last)
        {
            std::size_t first_index = (std::size_t)(first - data_);
            std::size_t last_index = (std::size_t)(last - data_);
            if (first == last)
            {
                return data_ + first_index;
            }
            for (std::size_t i = last_index; i < size_; i++)
            {
                data_[i - last_index + first_index] = std::move(data_[i]);
            }
            std::size_t newsize = size_ - (last_index - first_index);
            for (; size_ > newsize; size_--)
            {
                std::destroy_at(data_ + size_ - 1);
            }
            size_ = newsize;
            return data_ + first_index;
        }
        T *erase(const T *pos)
        {
            return erase(pos, pos + 1);
        }
    };
}
