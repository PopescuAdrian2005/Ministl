CXX ?= g++

CPPFLAGS := -Iinclude
CXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Wpedantic -Werror
DEPFLAGS := -MMD -MP

OUTPUT_DIR := build
TEST_SOURCES := $(wildcard tests/test_*.cpp)
TEST_NAMES := $(notdir $(basename $(TEST_SOURCES)))
TEST_BINARIES := $(addprefix $(OUTPUT_DIR)/,$(TEST_NAMES))
BENCHMARK_BINARY := $(OUTPUT_DIR)/benchmark_vector

.PHONY: all tests test sanitize benchmark bench-1m bench-10m bench-100m bench-1b bench-all clean

all: tests benchmark

tests: $(TEST_BINARIES)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/test_%: tests/test_%.cpp | $(OUTPUT_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) $(EXTRA_FLAGS) $< -o $@

test: tests
	@for executable in $(TEST_BINARIES); do \
		echo "Rulez $$(basename $$executable)"; \
		$$executable || exit 1; \
	done

sanitize:
	$(MAKE) clean
	$(MAKE) test EXTRA_FLAGS="-O0 -fsanitize=address,undefined -fno-omit-frame-pointer"

benchmark: $(BENCHMARK_BINARY)

$(BENCHMARK_BINARY): benchmarks/benchmark_vector.cpp | $(OUTPUT_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -O3 -DNDEBUG $< -o $@

bench-1m: benchmark
	$(BENCHMARK_BINARY) 1m

bench-10m: benchmark
	$(BENCHMARK_BINARY) 10m

bench-100m: benchmark
	$(BENCHMARK_BINARY) 100m

bench-1b: benchmark
	$(BENCHMARK_BINARY) 1b

bench-all: benchmark
	$(BENCHMARK_BINARY) 1m 10m 100m 1b

-include $(TEST_BINARIES:=.d) $(BENCHMARK_BINARY).d

clean:
	rm -rf $(OUTPUT_DIR)
