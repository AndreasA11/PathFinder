CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Iinclude

.PHONY: build run map clean test test_heap test_middleware test_heap_services test_map test_path_planner test_challenge hub rosbridge_server gui

build: src/main.cpp include/map_producer.hpp include/path_planner.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/project1 src/main.cpp

run: build
	./build/project1

test_heap: tests/test_min_heap.cpp src/heap_sort_node.cpp include/min_heap.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_min_heap tests/test_min_heap.cpp
	./build/test_min_heap

test_middleware: tests/test_middleware.cpp include/middleware/middleware.hpp include/middleware/rosbridge_server.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_middleware tests/test_middleware.cpp
	./build/test_middleware

test_heap_services: tests/test_heap_services.cpp include/heap_services.hpp include/min_heap.hpp include/middleware/middleware.hpp include/middleware/rosbridge_server.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_heap_services tests/test_heap_services.cpp
	./build/test_heap_services

test_map: tests/test_map_producer.cpp include/map_producer.hpp include/middleware/middleware.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_map tests/test_map_producer.cpp
	./build/test_map

test_path_planner: tests/test_path_planner.cpp include/path_planner.hpp include/middleware/middleware.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/test_path_planner tests/test_path_planner.cpp
	./build/test_path_planner

test: test_heap test_middleware test_heap_services test_map test_path_planner

test_challenge:
	@python3 tests/run_tests.py $(TESTS)

hub: src/hub.cpp include/heap_services.hpp include/map_producer.hpp include/path_planner.hpp include/middleware/middleware.hpp include/middleware/rosbridge_server.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/hub src/hub.cpp

rosbridge_server: src/rosbridge_server.cpp include/middleware/middleware.hpp include/middleware/rosbridge_server.hpp include/middleware/json.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o build/rosbridge_server src/rosbridge_server.cpp

map:
	@test -f maps/student_map.json || { echo "Create your own maps/student_map.json first; see maps/README.md" >&2; exit 1; }
	python3 tools/map_to_rosbridge.py maps/student_map.json

gui:
	python3 tools/web_gui.py

clean:
	rm -rf build
