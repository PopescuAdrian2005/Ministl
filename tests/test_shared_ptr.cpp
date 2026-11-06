#include <cassert>
#include <iostream>
#include <utility>

#include <ministl/shared_ptr.h>

struct SharedTracked {
    static inline int alive = 0;

    int value;

    explicit SharedTracked(int initial_value)
        : value(initial_value)
    {
        ++alive;
    }

    ~SharedTracked()
    {
        --alive;
    }
};

struct SharedPair {
    int first;
    int second;
};

struct SharedBase {
    virtual ~SharedBase() = default;
    int base_value = 10;
};

struct SharedDerived : SharedBase {
    int derived_value = 20;
};

int main()
{
    {
        ministl::shared_ptr<int> pointer;
        assert(pointer.get() == nullptr);
        assert(pointer.use_count() == 0);
        assert(!pointer);

        ministl::shared_ptr<int> copied(pointer);
        assert(copied.get() == nullptr);
        assert(copied.use_count() == 0);
    }

    {
        ministl::shared_ptr<SharedTracked> pointer(
            new SharedTracked(10)
        );

        assert(SharedTracked::alive == 1);
        assert(pointer.use_count() == 1);
        assert(pointer->value == 10);
        assert((*pointer).value == 10);
    }
    assert(SharedTracked::alive == 0);

    {
        ministl::shared_ptr<SharedTracked> pointer(
            new SharedTracked(15)
        );
        SharedTracked* original_address = pointer.get();

        pointer = pointer;
        assert(pointer.get() == original_address);
        assert(pointer.use_count() == 1);

        pointer = std::move(pointer);
        assert(pointer.get() == original_address);
        assert(pointer.use_count() == 1);
    }
    assert(SharedTracked::alive == 0);

    {
        ministl::shared_ptr<SharedTracked> first(
            new SharedTracked(20)
        );

        {
            ministl::shared_ptr<SharedTracked> second(first);

            assert(first.get() == second.get());
            assert(first.use_count() == 2);
            assert(second.use_count() == 2);
            assert(SharedTracked::alive == 1);
        }

        assert(first.use_count() == 1);
        assert(SharedTracked::alive == 1);
    }
    assert(SharedTracked::alive == 0);

    {
        ministl::shared_ptr<SharedTracked> source(
            new SharedTracked(30)
        );
        SharedTracked* original_address = source.get();

        ministl::shared_ptr<SharedTracked> destination(
            std::move(source)
        );

        assert(!source);
        assert(source.use_count() == 0);
        assert(destination.get() == original_address);
        assert(destination.use_count() == 1);
    }
    assert(SharedTracked::alive == 0);

    {
        ministl::shared_ptr<SharedTracked> first(
            new SharedTracked(40)
        );
        ministl::shared_ptr<SharedTracked> second(
            new SharedTracked(50)
        );
        assert(SharedTracked::alive == 2);

        second = first;

        assert(SharedTracked::alive == 1);
        assert(first.get() == second.get());
        assert(first.use_count() == 2);
    }
    assert(SharedTracked::alive == 0);

    {
        ministl::shared_ptr<int> first(new int(1));
        ministl::shared_ptr<int> second(new int(2));

        first.swap(second);
        assert(*first == 2);
        assert(*second == 1);

        first.reset();
        assert(!first);
        assert(first.use_count() == 0);
    }

    {
        ministl::shared_ptr<SharedPair> owner(
            new SharedPair{10, 20}
        );
        ministl::shared_ptr<int> alias(owner, &owner->second);

        assert(owner.use_count() == 2);
        assert(alias.use_count() == 2);
        assert(*alias == 20);

        owner.reset();
        assert(alias.use_count() == 1);
        assert(*alias == 20);
    }

    {
        ministl::shared_ptr<SharedDerived> derived(
            new SharedDerived
        );
        ministl::shared_ptr<SharedBase> base(derived);

        assert(derived.use_count() == 2);
        assert(base.use_count() == 2);
        assert(base->base_value == 10);
    }

    {
        auto pointer = ministl::make_shared<SharedTracked>(70);

        assert(pointer);
        assert(pointer.use_count() == 1);
        assert(pointer->value == 70);
        assert(SharedTracked::alive == 1);
    }
    assert(SharedTracked::alive == 0);

    std::cout << "Toate testele de bază pentru ministl::shared_ptr au trecut\n";
}
