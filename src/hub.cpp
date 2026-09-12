#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <atomic>
#include "middleware/middleware.hpp"
#include "middleware/rosbridge_server.hpp"
// *CHANGES*: Included heap_services.hpp to register /heapify and /heap_sort as required by Heap Services Task.md.
#include "heap_services.hpp"
// *CHANGES*: Included reusable map producer to publish the Project 1 default /map payload.
#include "map_producer.hpp"
// *CHANGES*: Add the application-layer A* planner.
#include "path_planner.hpp"

// Global stop flag for signal handling
static std::atomic<bool> g_hub_shutdown{false};

/*
1. Set the global shutdown request flag to true upon receiving SIGINT or SIGTERM.
*/
void hub_signal_handler(int sig) {
    (void)sig;
    // Associated with Step 1: Set shutdown flag
    g_hub_shutdown.store(true);
}

/*
1. Register signal handlers for SIGINT and SIGTERM to enable graceful shutdown.
2. Parse command line arguments for port (default: 9095).
3. Instantiate the central Middleware hub instance.
4. Register the /heapify and /heap_sort services with the middleware.
5. Register the planner before startup map publication.
6. Instantiate RosbridgeServer bound to 127.0.0.1:port.
7. Start the RosbridgeServer on a background thread.
8. Publish the reusable default occupancy grid on /map.
9. Enter wait loop until shutdown is signaled.
10. Stop server and perform clean shutdown.
*/
int main(int argc, char* argv[]) {
    // Associated with Step 1: Register signal handlers
    std::signal(SIGINT, hub_signal_handler);
    std::signal(SIGTERM, hub_signal_handler);

    // Associated with Step 2: Parse port
    int port = 9095;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception&) {
            std::cerr << "Invalid port argument. Using default 9095." << std::endl;
        }
    }

    // Associated with Step 3: Create central middleware
    middleware::Middleware mw;

    // Associated with Step 4: Register /heapify and /heap_sort services
    // *CHANGES*: Registered Project 1 heap services on the middleware so external TCP clients can invoke them.
    heap_services::register_heap_services(mw);

    // Associated with Step 5: Register planner before map publication.
    // *CHANGES*: Keep /plan_path separate from generic middleware.
    path_planner::Planner planner(mw);

    // Associated with Step 6: Create RosbridgeServer
    middleware::RosbridgeServer server(mw, "127.0.0.1", port);

    // Associated with Step 7: Start server
    std::cout << "[Hub] Starting Project 1 Middleware Hub on 127.0.0.1:" << port << "..." << std::endl;
    if (!server.start()) {
        std::cerr << "[Hub] Error: Could not bind to 127.0.0.1:" << port << std::endl;
        return 1;
    }
    std::cout << "[Hub] Middleware Hub is running (services: /heapify, /heap_sort available)." << std::endl;

    // Associated with Step 8: Publish one valid, nonpersistent startup map.
    // *CHANGES*: Publish the default map through generic middleware so /map is available without a map server.
    try {
        map_producer::publish_map(mw, map_producer::make_default_map());
        std::cout << "[Hub] Published default occupancy grid on /map." << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "[Hub] Error: Default map is invalid: " << error.what() << std::endl;
        server.stop();
        return 1;
    }

    // Associated with Step 9: Wait for shutdown
    while (!g_hub_shutdown.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Associated with Step 10: Clean shutdown
    std::cout << "\n[Hub] Stopping Hub..." << std::endl;
    server.stop();
    std::cout << "[Hub] Hub stopped cleanly." << std::endl;
    return 0;
}
