#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <atomic>
#include "middleware/middleware.hpp"
#include "middleware/rosbridge_server.hpp"
#include "heap_services.hpp"
#include "map_producer.hpp"
// *CHANGES*: Add application-layer planner registration for /plan_path.
#include "path_planner.hpp"

// Global flag to request clean shutdown
static std::atomic<bool> g_shutdown_requested{false};

/*
1. Set the global shutdown flag to true upon receiving a termination signal (SIGINT or SIGTERM).
*/
void signal_handler(int sig) {
    (void)sig;
    // Associated with Step 1: Set shutdown request flag
    g_shutdown_requested.store(true);
}

/*
1. Register signal handlers for SIGINT and SIGTERM to enable graceful process shutdown.
2. Instantiate the central Project 1 Middleware instance.
3. Register the Project 1 heap services (/heapify and /heap_sort) on the middleware.
4. Register the planner before startup map publication.
5. Instantiate and start the RosbridgeServer listening on 127.0.0.1:9095 for external TCP/JSON clients.
6. Publish the reusable default occupancy grid on /map.
7. Enter execution loop waiting for termination signal.
8. Stop RosbridgeServer cleanly and exit with code 0.
*/
int main(int argc, char* argv[]) {
    // Associated with Step 1: Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int port = 9095;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception&) {
            std::cerr << "Invalid port. Defaulting to 9095." << std::endl;
        }
    }

    // Associated with Step 2: Create central middleware
    middleware::Middleware mw;

    // Associated with Step 3: Register /heapify and /heap_sort services
    heap_services::register_heap_services(mw);

    // Associated with Step 4: Register planner before map publication.
    // *CHANGES*: The planner must receive the initial and replacement /map messages.
    path_planner::Planner planner(mw);

    // Associated with Step 5: Create and start rosbridge server
    middleware::RosbridgeServer server(mw, "127.0.0.1", port);
    if (!server.start()) {
        std::cerr << "[project1] Error: Failed to start rosbridge server on 127.0.0.1:" << port << std::endl;
        return 1;
    }
    std::cout << "[project1] Rosbridge gateway is ready and listening on 127.0.0.1:" << port << std::endl;

    // Associated with Step 6: Publish one valid startup map without introducing map-server state.
    // *CHANGES*: Expose the reusable default occupancy grid on /map in the normal runtime.
    try {
        map_producer::publish_map(mw, map_producer::make_default_map());
        std::cout << "[project1] Published default occupancy grid on /map." << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "[project1] Error: Default map is invalid: " << error.what() << std::endl;
        server.stop();
        return 1;
    }

    // Associated with Step 7: Wait for shutdown signal
    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Associated with Step 8: Graceful shutdown
    std::cout << "[project1] Stopping rosbridge gateway..." << std::endl;
    server.stop();
    std::cout << "[project1] Stopped cleanly." << std::endl;
    return 0;
}
