#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <atomic>
#include "middleware/middleware.hpp"
#include "middleware/rosbridge_server.hpp"

// Global stop flag for signal handling
static std::atomic<bool> g_shutdown_requested{false};

/*
1. Set the global shutdown request flag to true upon receiving SIGINT or SIGTERM.
*/
void signal_handler(int sig) {
    (void)sig;
    // Associated with Step 1: Set shutdown flag
    g_shutdown_requested.store(true);
}

/*
1. Register signal handlers for SIGINT and SIGTERM to enable graceful shutdown.
2. Instantiate the central Middleware instance.
3. Instantiate RosbridgeServer bound to 127.0.0.1:9095.
4. Start the RosbridgeServer listening for incoming TCP clients.
5. Enter loop waiting for shutdown request signal.
6. Stop RosbridgeServer cleanly and exit.
*/
int main(int argc, char* argv[]) {
    // Associated with Step 1: Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int port = 9095;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    // Associated with Step 2: Create central middleware instance
    middleware::Middleware mw;

    // Associated with Step 3: Create RosbridgeServer gateway
    middleware::RosbridgeServer server(mw, "127.0.0.1", port);

    // Associated with Step 4: Start TCP gateway server
    std::cout << "[RosbridgeServer] Starting on 127.0.0.1:" << port << "..." << std::endl;
    if (!server.start()) {
        std::cerr << "[RosbridgeServer] Failed to bind/listen on 127.0.0.1:" << port << std::endl;
        return 1;
    }
    std::cout << "[RosbridgeServer] Ready and listening for connections." << std::endl;

    // Associated with Step 5: Wait for shutdown signal
    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Associated with Step 6: Graceful shutdown
    std::cout << "\n[RosbridgeServer] Shutting down..." << std::endl;
    server.stop();
    std::cout << "[RosbridgeServer] Stopped cleanly." << std::endl;
    return 0;
}
