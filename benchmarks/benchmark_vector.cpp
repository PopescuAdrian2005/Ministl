#include <ministl/ministl.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
    const std::uint64_t seed = 0x9E3779B97F4A7C15ULL;
    const std::uint64_t operations_per_round = 10'000;
    const std::size_t maximum_elements = 65'536;
    const std::size_t arena_bytes = 32 * 1024 * 1024;

    const unsigned probability_scale = 10'000;
    const unsigned read_limit = 4'000;
    const unsigned write_limit = 6'500;
    const unsigned push_back_limit = 8'500;
    const unsigned pop_back_limit = 9'500;
    const unsigned insert_limit = 9'750;
    const unsigned erase_limit = 9'990;
    const unsigned copy_limit = 9'999;

    class random_generator
    {
    private:
        std::uint64_t state_;

    public:
        explicit random_generator(std::uint64_t initial_state) noexcept
            : state_(initial_state)
        {
        }

        std::uint64_t next() noexcept
        {
            std::uint64_t value = state_;
            value ^= value >> 12;
            value ^= value << 25;
            value ^= value >> 27;
            state_ = value;
            return value * 0x2545F4914F6CDD1DULL;
        }
    };

    struct benchmark_result
    {
        std::string name;
        double seconds = 0.0;
        std::uint64_t checksum = 0;
    };

    std::uint64_t parse_operation_count(std::string_view text)
    {
        if (text.empty())
        {
            throw std::invalid_argument("Numarul de operatii este gol");
        }

        std::uint64_t multiplier = 1;
        const char suffix = static_cast<char>(
            std::tolower(static_cast<unsigned char>(text.back())));

        if (suffix == 'k' || suffix == 'm' || suffix == 'b')
        {
            if (suffix == 'k')
            {
                multiplier = 1'000;
            }
            else if (suffix == 'm')
            {
                multiplier = 1'000'000;
            }
            else
            {
                multiplier = 1'000'000'000;
            }
            text.remove_suffix(1);
        }

        if (text.empty())
        {
            throw std::invalid_argument("Lipseste numarul din fata sufixului");
        }

        std::uint64_t value = 0;
        for (const char character : text)
        {
            if (character < '0' || character > '9')
            {
                throw std::invalid_argument("Numar de operatii invalid");
            }

            const auto digit = static_cast<std::uint64_t>(character - '0');
            if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10)
            {
                throw std::overflow_error("Numarul de operatii este prea mare");
            }
            value = value * 10 + digit;
        }

        if (value > std::numeric_limits<std::uint64_t>::max() / multiplier)
        {
            throw std::overflow_error("Numarul de operatii este prea mare");
        }
        return value * multiplier;
    }

    template <template <class, class> class Vector,
              class Allocator,
              class AllocatorFactory>
    benchmark_result run_benchmark(
        std::string name,
        std::uint64_t operation_count,
        AllocatorFactory create_allocator)
    {
        random_generator random(seed);
        std::uint64_t checksum = 0;
        std::uint64_t remaining = operation_count;

        const auto start = std::chrono::steady_clock::now();

        while (remaining != 0)
        {
            const std::uint64_t round_operations =
                std::min(remaining, operations_per_round);
            Allocator allocator = create_allocator();
            Vector<int, Allocator> values(allocator);

            for (std::uint64_t operation = 0;
                 operation < round_operations;
                 ++operation)
            {
                const std::uint64_t random_value = random.next();
                const unsigned choice = static_cast<unsigned>(
                    random_value % probability_scale);

                if (values.empty())
                {
                    values.push_back(static_cast<int>(random_value));
                    continue;
                }

                if (choice < read_limit)
                {
                    const std::size_t index = static_cast<std::size_t>(
                        random.next() % values.size());
                    checksum += static_cast<std::uint32_t>(values[index]);
                }
                else if (choice < write_limit)
                {
                    const std::size_t index = static_cast<std::size_t>(
                        random.next() % values.size());
                    values[index] ^= static_cast<int>(random_value);
                }
                else if (choice < push_back_limit)
                {
                    if (values.size() < maximum_elements)
                    {
                        values.push_back(static_cast<int>(random_value));
                    }
                    else
                    {
                        values.pop_back();
                    }
                }
                else if (choice < pop_back_limit)
                {
                    values.pop_back();
                }
                else if (choice < insert_limit)
                {
                    const std::size_t index = static_cast<std::size_t>(
                        random.next() % values.size());
                    values.insert(
                        values.begin() + index,
                        static_cast<int>(random_value));
                }
                else if (choice < erase_limit)
                {
                    const std::size_t index = static_cast<std::size_t>(
                        random.next() % values.size());
                    values.erase(values.begin() + index);
                }
                else if (choice < copy_limit)
                {
                    Vector tmp(values);
                    values = std::move(tmp);
                }
                else
                {
                    values.shrink_to_fit();
                }
            }

            checksum ^= static_cast<std::uint64_t>(values.size());
            if (!values.empty())
            {
                checksum ^= static_cast<std::uint32_t>(values.back());
            }
            remaining -= round_operations;
        }

        const auto stop = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(stop - start).count();
        return {std::move(name), seconds, checksum};
    }

    template <class Allocator>
    auto stateless_allocator_factory()
    {
        return []
        {
            return Allocator{};
        };
    }

    std::vector<benchmark_result> run_all(std::uint64_t operation_count)
    {
        std::vector<benchmark_result> results;
        results.reserve(8);

        results.push_back(run_benchmark<std::vector, std::allocator<int>>(
            "std::vector + std::allocator",
            operation_count,
            stateless_allocator_factory<std::allocator<int>>()));
        results.push_back(run_benchmark<std::vector, ministl::malloc_allocator<int>>(
            "std::vector + malloc_allocator",
            operation_count,
            stateless_allocator_factory<ministl::malloc_allocator<int>>()));
        results.push_back(run_benchmark<std::vector, ministl::mmap_allocator<int>>(
            "std::vector + mmap_allocator",
            operation_count,
            stateless_allocator_factory<ministl::mmap_allocator<int>>()));
        results.push_back(run_benchmark<std::vector, ministl::arena_allocator<int>>(
            "std::vector + arena_allocator",
            operation_count,
            []
            {
                return ministl::arena_allocator<int>(arena_bytes);
            }));

        results.push_back(run_benchmark<ministl::vector, std::allocator<int>>(
            "ministl::vector + std::allocator",
            operation_count,
            stateless_allocator_factory<std::allocator<int>>()));
        results.push_back(run_benchmark<ministl::vector, ministl::malloc_allocator<int>>(
            "ministl::vector + malloc_allocator",
            operation_count,
            stateless_allocator_factory<ministl::malloc_allocator<int>>()));
        results.push_back(run_benchmark<ministl::vector, ministl::mmap_allocator<int>>(
            "ministl::vector + mmap_allocator",
            operation_count,
            stateless_allocator_factory<ministl::mmap_allocator<int>>()));
        results.push_back(run_benchmark<ministl::vector, ministl::arena_allocator<int>>(
            "ministl::vector + arena_allocator",
            operation_count,
            []
            {
                return ministl::arena_allocator<int>(arena_bytes);
            }));

        return results;
    }

    void print_results(
        std::uint64_t operation_count,
        const std::vector<benchmark_result> &results)
    {
        std::cout << "\nOperatii: " << operation_count << "\n\n";
        std::cout << std::left << std::setw(43) << "Varianta"
                  << std::right << std::setw(14) << "Secunde"
                  << std::setw(16) << "ns/operatie\n";
        std::cout << std::string(95, '-') << '\n';

        const std::uint64_t expected_checksum = results.front().checksum;
        for (const auto &result : results)
        {
            if (result.checksum != expected_checksum)
            {
                throw std::runtime_error(
                    "Rezultate diferite intre implementarile de vector");
            }

            const double nanoseconds_per_operation =
                result.seconds * 1'000'000'000.0 /
                static_cast<double>(operation_count);

            std::cout << std::left << std::setw(43) << result.name
                      << std::right << std::fixed << std::setprecision(6)
                      << std::setw(14) << result.seconds
                      << std::setprecision(2)
                      << std::setw(16) << nanoseconds_per_operation << '\n';
        }
    }
}

int main(int argc, char **argv)
{
    try
    {
        if (argc == 1)
        {
            const std::uint64_t operations = 1'000'000;
            print_results(operations, run_all(operations));
            return 0;
        }

        for (int argument = 1; argument < argc; ++argument)
        {
            const std::uint64_t operations = parse_operation_count(argv[argument]);
            if (operations == 0)
            {
                throw std::invalid_argument(
                    "Numarul de operatii trebuie sa fie pozitiv");
            }
            print_results(operations, run_all(operations));
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Eroare: " << error.what() << '\n';
        return 1;
    }
}
