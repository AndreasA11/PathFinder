CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Iinclude

.PHONY: all build run test test_heap map clean

all: test_heap

test_heap: tests/test_min_heap.cpp src/heap_sort_node.cpp include/min_heap.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_min_heap tests/test_min_heap.cpp
	./build/test_min_heap

test: test_heap

build:
	mkdir -p build
	@if [ -f src/main.cpp ]; then \
		$(CXX) $(CXXFLAGS) -o build/project1 src/main.cpp; \
	else \
		echo "Note: src/main.cpp does not exist yet. Building test_min_heap instead."; \
		$(CXX) $(CXXFLAGS) -o build/test_min_heap tests/test_min_heap.cpp; \
	fi

run: build
	@if [ -f build/project1 ]; then \
		./build/project1; \
	else \
		./build/test_min_heap; \
	fi

map:
	@test -f maps/student_map.json || { echo "Create your own maps/student_map.json first; see maps/README.md" >&2; exit 1; }
	python3 tools/map_to_rosbridge.py maps/student_map.json

clean:
	rm -rf build
