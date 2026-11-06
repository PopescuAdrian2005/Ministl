#include <ministl/ministl.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

struct alignas(64) aligned_value
{
    int value = 0;
};

template <class Allocator>
void test_with_std_vector(const Allocator &allocator)
{
    std::vector<int, Allocator> values(allocator);
    for (int i = 0; i < 1000; ++i)
    {
        values.push_back(i);
    }
    for (int i = 0; i < 1000; ++i)
    {
        assert(values[static_cast<std::size_t>(i)] == i);
    }
}

int main()
{
    test_with_std_vector(ministl::malloc_allocator<int>{});
    test_with_std_vector(ministl::mmap_allocator<int>{});

    ministl::arena_allocator<int> arena(1U << 20);
    test_with_std_vector(arena);

    int *first = arena.allocate(1);
    int *second = arena.allocate(1);
    assert(first != second);

    ministl::arena_allocator<aligned_value> aligned_allocator(arena);
    aligned_value *aligned = aligned_allocator.allocate(1);
    assert(reinterpret_cast<std::uintptr_t>(aligned) % alignof(aligned_value) == 0);

    ministl::vector<std::string, ministl::malloc_allocator<std::string>> strings;
    strings.emplace_back("abc");
    assert(strings.front() == "abc");
}
