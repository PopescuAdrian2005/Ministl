#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

#include <ministl/unique_ptr.h>

struct TrackedObject {
    static inline int alive = 0;
    static inline int destroyed = 0;

    int value;

    explicit TrackedObject(int initial_value)
        : value(initial_value)
    {
        ++alive;
    }

    ~TrackedObject()
    {
        --alive;
        ++destroyed;
    }

    int doubled() const
    {
        return value * 2;
    }
};

static_assert(!std::is_copy_constructible_v<ministl::unique_ptr<int>>);
static_assert(!std::is_copy_assignable_v<ministl::unique_ptr<int>>);
static_assert(std::is_move_constructible_v<ministl::unique_ptr<int>>);
static_assert(std::is_move_assignable_v<ministl::unique_ptr<int>>);

int main()
{
    {
        ministl::unique_ptr<int> pointer;

        assert(pointer.get() == nullptr);
        assert(!pointer);
    }

    {
        ministl::unique_ptr<int> pointer(new int(42));

        assert(pointer);
        assert(pointer.get() != nullptr);
        assert(*pointer == 42);

        *pointer = 50;
        assert(*pointer == 50);
    }

    {
        ministl::unique_ptr<TrackedObject> pointer(new TrackedObject(12));

        assert(pointer->value == 12);
        assert(pointer->doubled() == 24);
    }
    assert(TrackedObject::alive == 0);

    {
        ministl::unique_ptr<int> source(new int(17));
        int* original_address = source.get();

        ministl::unique_ptr<int> destination(std::move(source));

        assert(!source);
        assert(source.get() == nullptr);
        assert(destination.get() == original_address);
        assert(*destination == 17);
    }

    {
        const int destroyed_before = TrackedObject::destroyed;

        ministl::unique_ptr<TrackedObject> destination(new TrackedObject(10));
        ministl::unique_ptr<TrackedObject> source(new TrackedObject(20));
        assert(TrackedObject::alive == 2);

        destination = std::move(source);

        assert(!source);
        assert(destination->value == 20);
        assert(TrackedObject::alive == 1);
        assert(TrackedObject::destroyed == destroyed_before + 1);

        TrackedObject* old_address = destination.get();
        destination = std::move(destination);
        assert(destination.get() == old_address);
        assert(destination->value == 20);
    }
    assert(TrackedObject::alive == 0);

    {
        ministl::unique_ptr<TrackedObject> pointer(new TrackedObject(30));

        TrackedObject* raw_pointer = pointer.release();

        assert(!pointer);
        assert(raw_pointer != nullptr);
        assert(raw_pointer->value == 30);
        assert(TrackedObject::alive == 1);

        delete raw_pointer;
        assert(TrackedObject::alive == 0);
    }

    {
        ministl::unique_ptr<TrackedObject> pointer(new TrackedObject(40));
        assert(TrackedObject::alive == 1);

        pointer.reset(new TrackedObject(50));
        assert(TrackedObject::alive == 1);
        assert(pointer->value == 50);

        pointer.reset();
        assert(!pointer);
        assert(TrackedObject::alive == 0);

        pointer.reset();
        assert(!pointer);
    }

    {
        ministl::unique_ptr<int> first(new int(1));
        ministl::unique_ptr<int> second(new int(2));

        int* first_address = first.get();
        int* second_address = second.get();

        first.swap(second);

        assert(first.get() == second_address);
        assert(second.get() == first_address);
        assert(*first == 2);
        assert(*second == 1);
    }

    {
        auto number = ministl::make_unique<int>(42);
        assert(number);
        assert(*number == 42);

        auto object = ministl::make_unique<TrackedObject>(60);
        assert(object);
        assert(object->value == 60);
        assert(TrackedObject::alive == 1);
    }

    assert(TrackedObject::alive == 0);
    std::cout << "Toate testele pentru ministl::unique_ptr au trecut\n";
}
