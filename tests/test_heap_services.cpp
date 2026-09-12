#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "heap_services.hpp"
#include "middleware/middleware.hpp"
#include "middleware/rosbridge_server.hpp"

// Helper: check binary min-heap invariant
bool is_valid_min_heap(const std::vector<double>& v) {
    size_t n = v.size();
    for (size_t i = 0; i < n; ++i) {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        if (left < n && v[left] < v[i]) return false;
        if (right < n && v[right] < v[i]) return false;
    }
    return true;
}

// Helper: check multiset equality
bool multiset_equal(std::vector<double> a, std::vector<double> b) {
    if (a.size() != b.size()) return false;
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    return a == b;
}

// Helper: connect TCP client
int connect_tcp(const std::string& host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

// Helper: send line
bool send_tcp_line(int sock, const std::string& msg) {
    std::string payload = msg + "\n";
    size_t total = payload.size();
    size_t sent = 0;
    const char* buf = payload.data();
    while (sent < total) {
        ssize_t n = send(sock, buf + sent, total - sent, 0);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

// Helper: read line
std::string read_tcp_line(int sock, int timeout_ms = 2000) {
    std::string result;
    auto start = std::chrono::steady_clock::now();
    struct timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char c = 0;
    while (true) {
        ssize_t n = recv(sock, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c != '\r') result += c;
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_ms) {
            break;
        }
    }
    return result;
}

/*
1. Test in-process /heapify service invocation.
2. Verify /heapify preserves multiset and satisfies binary min-heap invariant.
3. Verify empty input, singletons, duplicates, negative, and fractional numbers.
*/
void test_in_process_heapify() {
    std::cout << "[TEST] In-process /heapify service..." << std::endl;
    middleware::Middleware mw;
    heap_services::register_heap_services(mw);

    // Standard input [3.0, 1.0, 2.0]
    middleware::JsonValue req = middleware::JsonValue::make_object();
    middleware::JsonValue values = middleware::JsonValue::make_array();
    values.push_back(3.0);
    values.push_back(1.0);
    values.push_back(2.0);
    req["values"] = values;

    // Associated with Step 1 & 2: Call /heapify service
    auto res = mw.call_service("/heapify", req);
    assert(res.first == true && "Call to /heapify must succeed");
    assert(res.second.contains("heap"));

    const auto& heap_arr = res.second["heap"].as_array();
    std::vector<double> heap_vec;
    for (const auto& item : heap_arr) {
        heap_vec.push_back(item.as_number());
    }

    assert(is_valid_min_heap(heap_vec) && "Result of /heapify must satisfy min-heap invariant");
    assert(multiset_equal(heap_vec, {3.0, 1.0, 2.0}) && "Multiset must be preserved");

    // Associated with Step 3: Empty input
    req["values"] = middleware::JsonValue::make_array();
    auto empty_res = mw.call_service("/heapify", req);
    assert(empty_res.first == true);
    assert(empty_res.second["heap"].as_array().empty());

    // Mixed values: negative, fractional, duplicates
    std::vector<double> mixed = {-5.5, 10.2, 0.0, -5.5, 3.14, -100.0, 0.0};
    middleware::JsonValue mixed_arr = middleware::JsonValue::make_array();
    for (double v : mixed) mixed_arr.push_back(v);
    req["values"] = mixed_arr;

    auto mixed_res = mw.call_service("/heapify", req);
    assert(mixed_res.first == true);
    std::vector<double> mixed_out;
    for (const auto& item : mixed_res.second["heap"].as_array()) {
        mixed_out.push_back(item.as_number());
    }
    assert(is_valid_min_heap(mixed_out));
    assert(multiset_equal(mixed_out, mixed));
    std::cout << "  -> PASSED" << std::endl;
}

/*
1. Test in-process /heap_sort service invocation.
2. Verify /heap_sort returns ascending sorted array and preserves multiset.
3. Verify empty input, singletons, duplicates, negative, and fractional numbers.
*/
void test_in_process_heap_sort() {
    std::cout << "[TEST] In-process /heap_sort service..." << std::endl;
    middleware::Middleware mw;
    heap_services::register_heap_services(mw);

    // Standard input [3.0, 1.0, 2.0]
    middleware::JsonValue req = middleware::JsonValue::make_object();
    middleware::JsonValue numbers = middleware::JsonValue::make_array();
    numbers.push_back(3.0);
    numbers.push_back(1.0);
    numbers.push_back(2.0);
    req["numbers"] = numbers;

    // Associated with Step 1 & 2: Call /heap_sort service
    auto res = mw.call_service("/heap_sort", req);
    assert(res.first == true && "Call to /heap_sort must succeed");
    assert(res.second.contains("sorted"));

    std::vector<double> sorted_vec;
    for (const auto& item : res.second["sorted"].as_array()) {
        sorted_vec.push_back(item.as_number());
    }

    std::vector<double> expected = {1.0, 2.0, 3.0};
    assert(sorted_vec == expected);

    // Associated with Step 3: Empty input
    req["numbers"] = middleware::JsonValue::make_array();
    auto empty_res = mw.call_service("/heap_sort", req);
    assert(empty_res.first == true);
    assert(empty_res.second["sorted"].as_array().empty());

    // Mixed values: negative, fractional, duplicates
    std::vector<double> mixed = {4.2, -1.1, 0.0, -1.1, 100.5, -99.9};
    middleware::JsonValue mixed_arr = middleware::JsonValue::make_array();
    for (double v : mixed) mixed_arr.push_back(v);
    req["numbers"] = mixed_arr;

    auto mixed_res = mw.call_service("/heap_sort", req);
    assert(mixed_res.first == true);
    std::vector<double> mixed_out;
    for (const auto& item : mixed_res.second["sorted"].as_array()) {
        mixed_out.push_back(item.as_number());
    }

    assert(multiset_equal(mixed_out, mixed));
    for (size_t i = 1; i < mixed_out.size(); ++i) {
        assert(mixed_out[i - 1] <= mixed_out[i]);
    }
    std::cout << "  -> PASSED" << std::endl;
}

/*
1. Test external TCP/JSON invocation of /heapify and /heap_sort on 127.0.0.1:9095.
2. Verify request/response ID correlation across independent external clients.
3. Verify repeated calls.
*/
void test_tcp_gateway_heap_services() {
    std::cout << "[TEST] TCP Gateway /heapify and /heap_sort (127.0.0.1:9095)..." << std::endl;
    middleware::Middleware mw;
    heap_services::register_heap_services(mw);

    int test_port = 9095;
    middleware::RosbridgeServer server(mw, "127.0.0.1", test_port);
    assert(server.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Client 1 calls /heapify
    int c1 = connect_tcp("127.0.0.1", test_port);
    assert(c1 >= 0);
    assert(send_tcp_line(c1, "{\"op\":\"call_service\",\"service\":\"/heapify\",\"args\":{\"values\":[5.0,2.0,8.0,1.0]},\"id\":\"req_heapify_1\"}"));

    std::string line1 = read_tcp_line(c1);
    assert(!line1.empty());
    middleware::JsonValue resp1 = middleware::JsonValue::parse(line1);
    assert(resp1["op"].as_string() == "service_response");
    assert(resp1["service"].as_string() == "/heapify");
    assert(resp1["id"].as_string() == "req_heapify_1");
    assert(resp1["result"].as_bool() == true);

    std::vector<double> heap1;
    for (const auto& v : resp1["values"]["heap"].as_array()) {
        heap1.push_back(v.as_number());
    }
    assert(is_valid_min_heap(heap1));
    assert(multiset_equal(heap1, {5.0, 2.0, 8.0, 1.0}));

    // Client 2 calls /heap_sort concurrently
    int c2 = connect_tcp("127.0.0.1", test_port);
    assert(c2 >= 0);
    assert(send_tcp_line(c2, "{\"op\":\"call_service\",\"service\":\"/heap_sort\",\"args\":{\"numbers\":[5.0,2.0,8.0,1.0]},\"id\":\"req_heapsort_1\"}"));

    std::string line2 = read_tcp_line(c2);
    assert(!line2.empty());
    middleware::JsonValue resp2 = middleware::JsonValue::parse(line2);
    assert(resp2["op"].as_string() == "service_response");
    assert(resp2["service"].as_string() == "/heap_sort");
    assert(resp2["id"].as_string() == "req_heapsort_1");
    assert(resp2["result"].as_bool() == true);

    std::vector<double> sorted2;
    for (const auto& v : resp2["values"]["sorted"].as_array()) {
        sorted2.push_back(v.as_number());
    }
    std::vector<double> expected2 = {1.0, 2.0, 5.0, 8.0};
    assert(sorted2 == expected2);

    // Repeated call on existing connection
    assert(send_tcp_line(c1, "{\"op\":\"call_service\",\"service\":\"/heap_sort\",\"args\":{\"numbers\":[10.0,-5.0,0.0]},\"id\":\"req_repeat_c1\"}"));
    std::string line3 = read_tcp_line(c1);
    assert(!line3.empty());
    middleware::JsonValue resp3 = middleware::JsonValue::parse(line3);
    assert(resp3["id"].as_string() == "req_repeat_c1");
    assert(resp3["result"].as_bool() == true);
    std::vector<double> sorted3;
    for (const auto& v : resp3["values"]["sorted"].as_array()) {
        sorted3.push_back(v.as_number());
    }
    std::vector<double> expected3 = {-5.0, 0.0, 10.0};
    assert(sorted3 == expected3);

    close(c1);
    close(c2);
    server.stop();
    std::cout << "  -> PASSED" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Heap Services Acceptance Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_in_process_heapify();
    test_in_process_heap_sort();
    test_tcp_gateway_heap_services();

    std::cout << "========================================" << std::endl;
    std::cout << "All Heap Services tests PASSED successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
