#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <ministl/vector.h>
#include <ministl/unique_ptr.h>

template struct ministl::vector<int>;

struct Tracked {
    static inline int alive = 0;
    static inline int copies = 0;
    static inline int moves = 0;
    static inline int destructions = 0;

    int value = 0;

    explicit Tracked(int value)
        : value(value)
    {
        ++alive;
    }

    Tracked(const Tracked& other)
        : value(other.value)
    {
        ++alive;
        ++copies;
    }

    Tracked(Tracked&& other) noexcept
        : value(std::exchange(other.value, -1))
    {
        ++alive;
        ++moves;
    }

    ~Tracked() {
        --alive;
        ++destructions;
    }
};

int main() {
    {
        ministl::vector<int> values;

        assert(values.size() == 0);
        assert(values.capacity() == 0);
        assert(values.empty());
        assert(values.data() == nullptr);
        assert(values.begin() == values.end());

        values.reserve(4);

        assert(values.size() == 0);
        assert(values.capacity() >= 4);
        assert(values.data() != nullptr);
        assert(values.begin() == values.end());

        values.push_back(10);
        values.push_back(20);
        values.push_back(30);

        assert(values.size() == 3);
        assert(!values.empty());

        assert(values[0] == 10);
        assert(values[1] == 20);
        assert(values[2] == 30);

        values[1] = 25;
        assert(values[1] == 25);

        assert(values.data() == values.begin());
        assert(values.end() - values.begin() == 3);

        const ministl::vector<int>& const_values = values;

        assert(const_values.size() == 3);
        assert(const_values.capacity() >= 3);
        assert(!const_values.empty());
        assert(const_values.data()[0] == 10);
        assert(const_values.begin()[1] == 25);
        assert(const_values.end() - const_values.begin() == 3);
        assert(const_values[2] == 30);

        values.pop_back();

        assert(values.size() == 2);
        assert(values[0] == 10);
        assert(values[1] == 25);

        ministl::vector<int> copied(values);

        assert(copied.size() == values.size());
        assert(copied[0] == 10);
        assert(copied[1] == 25);

        copied[0] = 100;

        assert(copied[0] == 100);
        assert(values[0] == 10);

        ministl::vector<int> assigned;
        assigned.push_back(999);
        assigned = values;

        assert(assigned.size() == values.size());
        assert(assigned[0] == 10);
        assert(assigned[1] == 25);

        assigned = assigned;

        assert(assigned.size() == 2);
        assert(assigned[0] == 10);
        assert(assigned[1] == 25);

        const std::size_t old_capacity = values.capacity();

        values.clear();

        assert(values.empty());
        assert(values.size() == 0);
        assert(values.capacity() == old_capacity);
        assert(values.begin() == values.end());
    }

    assert(Tracked::alive == 0);

    {
        ministl::vector<Tracked> objects;

        Tracked first(10);
        Tracked second(20);

        objects.push_back(first);
        objects.push_back(second);

        assert(objects.size() == 2);
        assert(objects[0].value == 10);
        assert(objects[1].value == 20);

        objects.reserve(100);

        assert(objects.size() == 2);
        assert(objects[0].value == 10);
        assert(objects[1].value == 20);

        objects.pop_back();

        assert(objects.size() == 1);
        assert(objects[0].value == 10);

        objects.clear();
        assert(objects.empty());

        assert(Tracked::alive == 2);
    }

    assert(Tracked::alive == 0);

    ministl::vector<int> source;
    source.push_back(10);
    source.push_back(20);

    int* original_address = source.data();

    ministl::vector<int> moved(std::move(source));

    assert(source.empty());
    assert(source.data() == nullptr);

    assert(moved.size() == 2);
    assert(moved.data() == original_address);
    assert(moved[0] == 10);
    assert(moved[1] == 20);

    source.push_back(30);

    assert(source.size() == 1);
    assert(source[0] == 30);

    ministl::vector<int> destination;
    destination.push_back(999);

    int* moved_address = moved.data();

    destination = std::move(moved);

    assert(moved.empty());
    assert(moved.data() == nullptr);

    assert(destination.size() == 2);
    assert(destination.data() == moved_address);
    assert(destination[0] == 10);
    assert(destination[1] == 20);

    moved.push_back(40);

    assert(moved.size() == 1);
    assert(moved[0] == 40);

    destination = std::move(destination);

    assert(destination.size() == 2);
    assert(destination[0] == 10);
    assert(destination[1] == 20);

    {
        ministl::vector<int> values(3);

        assert(values.size() == 3);
        assert(values.capacity() == 3);
        assert(values[0] == 0);
        assert(values[1] == 0);
        assert(values[2] == 0);
    }

    {
        ministl::vector<std::string> words(3, std::string("test"));

        assert(words.size() == 3);
        assert(words[0] == "test");
        assert(words[1] == "test");
        assert(words[2] == "test");
    }

    {
        ministl::vector<int> values(3, 7);
        const ministl::vector<int>& const_values = values;

        assert(values.front() == 7);
        assert(values.back() == 7);
        assert(const_values.at(1) == 7);
        assert(const_values.cend() - const_values.cbegin() == 3);

        bool exception_caught = false;
        try {
            (void)values.at(3);
        }
        catch (const std::out_of_range&) {
            exception_caught = true;
        }
        assert(exception_caught);
    }

    {
        ministl::vector<int> values;
        values.push_back(10);
        values.resize(4);

        assert(values.size() == 4);
        assert(values[0] == 10);
        assert(values[1] == 0);
        assert(values[2] == 0);
        assert(values[3] == 0);

        values.resize(7, 25);
        assert(values.size() == 7);
        assert(values[4] == 25);
        assert(values[5] == 25);
        assert(values[6] == 25);

        values.resize(2);
        assert(values.size() == 2);
        assert(values[0] == 10);
        assert(values[1] == 0);
    }

    {
        ministl::vector<std::string> words;
        std::string& first = words.emplace_back(3, 'a');
        assert(first == "aaa");

        words.push_back("ccc");
        std::string* inserted = words.insert(1, std::string("bbb"));
        assert(inserted == words.begin() + 1);
        assert(words.size() == 3);
        assert(words[0] == "aaa");
        assert(words[1] == "bbb");
        assert(words[2] == "ccc");

        words.emplace(0, 2, 'z');
        assert(words.size() == 4);
        assert(words[0] == "zz");
        assert(words[1] == "aaa");
        assert(words[2] == "bbb");
        assert(words[3] == "ccc");
    }

    {
        ministl::vector<int> values;
        for (int value = 1; value <= 6; ++value) {
            values.push_back(value);
        }

        int* next = values.erase(values.begin() + 1);
        assert(next == values.begin() + 1);
        assert(*next == 3);

        next = values.erase(values.begin() + 1, values.begin() + 3);
        assert(next == values.begin() + 1);
        assert(*next == 5);
        assert(values.size() == 3);
        assert(values[0] == 1);
        assert(values[1] == 5);
        assert(values[2] == 6);

        next = values.erase(values.begin() + 1, values.begin() + 1);
        assert(next == values.begin() + 1);
        assert(values.size() == 3);
    }

    {
        ministl::vector<int> values;
        values.reserve(20);
        values.push_back(4);
        values.push_back(8);
        values.push_back(12);
        values.shrink_to_fit();

        assert(values.size() == 3);
        assert(values.capacity() == 3);
        assert(values[0] == 4);
        assert(values[1] == 8);
        assert(values[2] == 12);
    }

    {
        ministl::vector<std::unique_ptr<int>> pointers;
        pointers.push_back(std::make_unique<int>(11));
        pointers.emplace_back(new int(22));

        assert(pointers.size() == 2);
        assert(*pointers[0] == 11);
        assert(*pointers[1] == 22);
    }

    std::cout << "Toate testele au trecut\n";
}
