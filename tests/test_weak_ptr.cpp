#include <cassert>
#include <iostream>
#include <utility>

#include <ministl/weak_ptr.h>

struct WeakTracked {
    static inline int alive = 0;

    int value;

    explicit WeakTracked(int initial_value)
        : value(initial_value)
    {
        ++alive;
    }

    ~WeakTracked()
    {
        --alive;
    }
};

struct WeakPair {
    int first;
    int second;
};

int main()
{
    {
        ministl::weak_ptr<int> weak;

        assert(weak.use_count() == 0);
        assert(weak.expired());

        auto locked = weak.lock();
        assert(!locked);
        assert(locked.use_count() == 0);
    }

    {
        auto owner = ministl::make_shared<WeakTracked>(10);
        ministl::weak_ptr<WeakTracked> weak(owner);

        assert(owner.use_count() == 1);
        assert(weak.use_count() == 1);
        assert(!weak.expired());

        {
            auto locked = weak.lock();
            assert(locked);
            assert(locked.get() == owner.get());
            assert(locked->value == 10);
            assert(owner.use_count() == 2);
        }

        assert(owner.use_count() == 1);
    }
    assert(WeakTracked::alive == 0);

    {
        ministl::weak_ptr<WeakTracked> weak;

        {
            auto owner = ministl::make_shared<WeakTracked>(20);
            weak = owner;

            assert(WeakTracked::alive == 1);
            assert(!weak.expired());
        }

        assert(WeakTracked::alive == 0);
        assert(weak.expired());
        assert(weak.use_count() == 0);
        assert(!weak.lock());

        weak.reset();
        assert(weak.expired());
    }

    {
        auto owner = ministl::make_shared<int>(30);
        ministl::weak_ptr<int> first(owner);
        ministl::weak_ptr<int> copied(first);
        ministl::weak_ptr<int> moved(std::move(copied));

        assert(!first.expired());
        assert(!moved.expired());
        assert(copied.expired());
        assert(*moved.lock() == 30);

        ministl::weak_ptr<int> assigned;
        assigned = first;
        assert(*assigned.lock() == 30);

        ministl::weak_ptr<int> move_assigned;
        move_assigned = std::move(assigned);
        assert(assigned.expired());
        assert(*move_assigned.lock() == 30);
    }

    {
        auto owner = ministl::make_shared<WeakPair>(WeakPair{1, 2});
        ministl::shared_ptr<int> alias(owner, &owner->second);
        ministl::weak_ptr<int> weak(alias);

        owner.reset();
        alias.reset();

        assert(weak.expired());
        assert(!weak.lock());
    }

    std::cout << "Toate testele pentru ministl::weak_ptr au trecut\n";
}
