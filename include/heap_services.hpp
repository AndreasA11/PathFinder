#ifndef HEAP_SERVICES_HPP
#define HEAP_SERVICES_HPP

#include <vector>
#include <string>
#include <cmath>
#include "min_heap.hpp"
#include "middleware/middleware.hpp"
#include "middleware/json.hpp"

namespace heap_services {

/*
1. Validate that the service arguments contain the required "values" array.
2. Extract numeric values from the array, validating that each value is finite.
3. Construct a MinHeap from the extracted numeric values using the heap primitive's O(n) constructor.
4. Export the resulting min-heap container as a vector of doubles.
5. Package the exported array into a service response object under the "heap" key.
6. Return a success pair {true, response}.
*/
inline std::pair<bool, middleware::JsonValue> handle_heapify(const middleware::JsonValue& args) {
    // Associated with Step 1: Validate argument presence
    std::vector<double> values;
    const middleware::JsonValue* arr_node = nullptr;

    if (args.is_object() && args.contains("values")) {
        arr_node = &args["values"];
    } else if (args.is_object() && args.contains("numbers")) {
        arr_node = &args["numbers"];
    } else if (args.is_array()) {
        arr_node = &args;
    } else {
        middleware::JsonValue err = middleware::JsonValue::make_object();
        err["error"] = "Invalid arguments: expected object with 'values' array";
        return {false, err};
    }

    if (!arr_node->is_array()) {
        middleware::JsonValue err = middleware::JsonValue::make_object();
        err["error"] = "'values' must be an array of numbers";
        return {false, err};
    }

    // Associated with Step 2: Extract and validate finite numeric values
    const auto& raw_arr = arr_node->as_array();
    values.reserve(raw_arr.size());
    for (const auto& item : raw_arr) {
        if (!item.is_number()) {
            middleware::JsonValue err = middleware::JsonValue::make_object();
            err["error"] = "All elements in 'values' must be finite numbers";
            return {false, err};
        }
        double num = item.as_number();
        if (!std::isfinite(num)) {
            middleware::JsonValue err = middleware::JsonValue::make_object();
            err["error"] = "Non-finite numbers (NaN/Infinity) are not supported";
            return {false, err};
        }
        values.push_back(num);
    }

    // Associated with Step 3: Construct min-heap using the reusable primitive's O(n) heapify
    MinHeap<double> h(values);

    // Associated with Step 4: Export resulting min-heap layout
    std::vector<double> heap_result = h.toVector();

    // Associated with Step 5: Package result into service response object
    middleware::JsonValue resp = middleware::JsonValue::make_object();
    middleware::JsonValue result_arr = middleware::JsonValue::make_array();
    for (double val : heap_result) {
        result_arr.push_back(val);
    }
    resp["heap"] = result_arr;

    // Associated with Step 6: Return success pair
    return {true, resp};
}

/*
1. Validate that the service arguments contain the required "numbers" array.
2. Extract numeric values from the array, validating that each value is finite.
3. Call the heap primitive's MinHeap::heapSort() method to produce an ascending sorted array.
4. Package the sorted values into a service response object under the "sorted" key.
5. Return a success pair {true, response}.
*/
inline std::pair<bool, middleware::JsonValue> handle_heap_sort(const middleware::JsonValue& args) {
    // Associated with Step 1: Validate argument presence
    std::vector<double> numbers;
    const middleware::JsonValue* arr_node = nullptr;

    if (args.is_object() && args.contains("numbers")) {
        arr_node = &args["numbers"];
    } else if (args.is_object() && args.contains("values")) {
        arr_node = &args["values"];
    } else if (args.is_array()) {
        arr_node = &args;
    } else {
        middleware::JsonValue err = middleware::JsonValue::make_object();
        err["error"] = "Invalid arguments: expected object with 'numbers' array";
        return {false, err};
    }

    if (!arr_node->is_array()) {
        middleware::JsonValue err = middleware::JsonValue::make_object();
        err["error"] = "'numbers' must be an array of numbers";
        return {false, err};
    }

    // Associated with Step 2: Extract and validate finite numeric values
    const auto& raw_arr = arr_node->as_array();
    numbers.reserve(raw_arr.size());
    for (const auto& item : raw_arr) {
        if (!item.is_number()) {
            middleware::JsonValue err = middleware::JsonValue::make_object();
            err["error"] = "All elements in 'numbers' must be finite numbers";
            return {false, err};
        }
        double num = item.as_number();
        if (!std::isfinite(num)) {
            middleware::JsonValue err = middleware::JsonValue::make_object();
            err["error"] = "Non-finite numbers (NaN/Infinity) are not supported";
            return {false, err};
        }
        numbers.push_back(num);
    }

    // Associated with Step 3: Call reusable heap primitive's heapSort() (exercises repeated extractMin())
    std::vector<double> sorted_result = MinHeap<double>::heapSort(numbers);

    // Associated with Step 4: Package sorted values into response object
    middleware::JsonValue resp = middleware::JsonValue::make_object();
    middleware::JsonValue result_arr = middleware::JsonValue::make_array();
    for (double val : sorted_result) {
        result_arr.push_back(val);
    }
    resp["sorted"] = result_arr;

    // Associated with Step 5: Return success pair
    return {true, resp};
}

/*
1. Register the "/heapify" service provider callback on the middleware.
2. Register the "/heap_sort" service provider callback on the middleware.
3. Return true if both services were registered successfully, false otherwise.
*/
inline bool register_heap_services(middleware::Middleware& mw) {
    // Associated with Step 1: Advertise /heapify service
    bool h_ok = mw.advertise_service("/heapify", handle_heapify);

    // Associated with Step 2: Advertise /heap_sort service
    bool s_ok = mw.advertise_service("/heap_sort", handle_heap_sort);

    // Associated with Step 3: Return success if both registered
    return h_ok && s_ok;
}

} // namespace heap_services

#endif // HEAP_SERVICES_HPP
