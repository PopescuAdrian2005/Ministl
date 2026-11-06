# ministl

`ministl` is an educational, header-only C++20 library that reimplements
several familiar STL components. Its APIs follow the style of the standard
library and cover many common use cases for dynamic arrays, smart pointers,
and allocators.

The project is not intended to replace the entire standard library. Its goal
is to demonstrate how these components work internally, including manual
memory management, RAII, copy and move semantics, exception handling,
reference counting, and interchangeable allocators.

## Implemented components

| Component | Purpose |
|---|---|
| `ministl::vector<T, Allocator>` | Contiguous dynamic container with a configurable allocator |
| `ministl::unique_ptr<T>` | Exclusive ownership of an object |
| `ministl::shared_ptr<T>` | Shared ownership through atomic reference counting |
| `ministl::weak_ptr<T>` | Non-owning reference to an object managed by `shared_ptr` |
| `ministl::malloc_allocator<T>` | Allocation with `malloc` and deallocation with `free` |
| `ministl::mmap_allocator<T>` | Virtual memory allocation with `mmap`/`munmap` |
| `ministl::arena_allocator<T>` | Fast monotonic allocation from a shared arena |

### `vector`

`ministl::vector` stores its elements in contiguous memory and provides a
familiar interface modeled after `std::vector`. It implements:

- constructors for an empty vector, an element count, a repeated value, and an
  explicit allocator;
- copy and move construction, assignment, and comparison through
  `operator==`;
- element access through `operator[]`, `at`, `front`, `back`, and
  `data`;
- pointer-based iterators through `begin`, `end`, `cbegin`, and `cend`;
- container information through `size`, `capacity`, and `empty`;
- capacity management through `reserve`, `resize`, and `shrink_to_fit`;
- modifiers such as `assign`, `push_back`, `emplace_back`, `emplace`,
  `insert`, `erase`, `pop_back`, `clear`, and `swap`;
- support for non-trivial types, move operations, and custom allocators.

The container can therefore be used in many common scenarios where a
`std::vector` would normally be used, while keeping the implementation
compact enough to study.

### `unique_ptr`

`ministl::unique_ptr` implements exclusive ownership and automatically
destroys its object when the pointer leaves scope. It cannot be copied, but it
can be moved. Its interface includes `get`, `operator*`, `operator->`,
conversion to `bool`, `release`, `reset`, and `swap`.
`ministl::make_unique` constructs a managed object directly.

### `shared_ptr` and `weak_ptr`

`ministl::shared_ptr` allows multiple pointers to own the same object. The
last owner automatically destroys the object. The implementation supports:

- copy and move operations, including compatible conversions such as
  derived-to-base;
- an aliasing constructor;
- `get`, dereferencing, conversion to `bool`, `use_count`, `reset`, and
  `swap`;
- simplified construction through `ministl::make_shared`.

`ministl::weak_ptr` observes an object without extending its lifetime and
can be used to avoid ownership cycles. It provides `use_count`, `expired`,
`lock`, `reset`, and `swap`. `lock()` returns a valid `shared_ptr` only
while the object is still alive.

#### Atomic control block and thread safety

The shared control block uses `std::atomic<std::size_t>` for both strong and
weak reference counts. `weak_ptr::lock()` uses a compare-and-exchange loop,
preventing a thread from acquiring ownership after the last `shared_ptr` has
started destroying the object.

Distinct `shared_ptr` and `weak_ptr` instances that share the same control
block can therefore update their ownership counts safely from different
threads. As with standard smart pointers, concurrently modifying the same
pointer instance requires external synchronization. Access to the managed
object must also be synchronized separately.

### Allocators

All allocators expose the conventional `value_type`, `allocate`, and
`deallocate` interface. They can be used with both `ministl::vector` and
standard allocator-aware containers:

- `malloc_allocator` uses `malloc`/`free` and checks for allocation-size
  overflow;
- `mmap_allocator` uses `mmap`/`munmap` and targets POSIX systems;
- `arena_allocator` allocates sequentially from a preallocated buffer.
  Allocator copies share the arena through a `shared_ptr`, and its memory is
  released when the last owner is destroyed. Individual deallocations are
  intentionally no-ops.

## Usage example

```cpp
#include <ministl/ministl.h>

#include <iostream>

int main()
{
    ministl::vector<int> values;
    values.push_back(10);
    values.emplace_back(20);

    auto unique = ministl::make_unique<int>(30);

    auto shared = ministl::make_shared<int>(40);
    ministl::weak_ptr<int> weak(shared);

    if (auto locked = weak.lock())
    {
        std::cout << values.back() + *unique + *locked << '\n';
    }
}
```

Because the library is header-only, it does not require separate installation
or linking. Compile the program with the `include` directory on the include
path:

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude program.cpp -o program
./program
```

## Project structure

```text
include/ministl/            public headers
include/ministl/allocators/ allocator implementations
include/ministl/detail/     internal control block and arena
tests/                      component tests
benchmarks/                 vector and allocator benchmark
```

## Building and testing

Build all tests and the benchmark:

```bash
make
```

Run all tests:

```bash
make test
```

Run the tests with AddressSanitizer and UBSan:

```bash
make sanitize
```

## Vector benchmark

The benchmark compares `std::vector` and `ministl::vector` with four
allocators: `std::allocator`, `malloc_allocator`, `mmap_allocator`, and
`arena_allocator`.

```bash
make bench-1m
make bench-10m
make bench-100m
make bench-1b
```

Run all benchmark sizes with a single command:

```bash
make bench-all
```

Executables and compiler-generated dependency files are written to the
`build/` directory. This directory is excluded from Git through
`.gitignore` and can be removed with:

```bash
make clean
```

## Benchmark results

The results below are from a single `make bench-all` run performed on
November 5, 2026. Values represent the average time in nanoseconds per
operation; lower is better.

- CPU: Intel Core i7-13650HX (12 logical processors available)
- Compiler: g++ 11.4.0
- Compiler options: `-std=c++20 -O3 -DNDEBUG`

| Implementation | 1M operations | 10M operations | 100M operations | 1B operations |
|---|---:|---:|---:|---:|
| `std::vector` + `std::allocator` | 10.64 ns | 10.37 ns | 11.64 ns | 10.72 ns |
| `std::vector` + `malloc_allocator` | 10.59 ns | 10.18 ns | 11.84 ns | 10.70 ns |
| `std::vector` + `mmap_allocator` | 18.42 ns | 18.98 ns | 18.93 ns | 20.04 ns |
| `std::vector` + `arena_allocator` | 11.74 ns | 11.81 ns | 11.63 ns | 11.72 ns |
| `ministl::vector` + `std::allocator` | 10.49 ns | 10.79 ns | 10.70 ns | 10.58 ns |
| `ministl::vector` + `malloc_allocator` | 10.74 ns | 12.81 ns | 10.55 ns | 10.94 ns |
| `ministl::vector` + `mmap_allocator` | 18.76 ns | 18.25 ns | 18.33 ns | 18.64 ns |
| `ministl::vector` + `arena_allocator` | 11.64 ns | 12.32 ns | 11.94 ns | 11.58 ns |

Timings depend on the CPU, compiler, operating system, and current system
load, so results obtained on another machine may differ.
