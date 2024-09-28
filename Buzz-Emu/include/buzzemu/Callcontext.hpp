#pragma once

#include <type_traits>
#include <utility> // For std::forward
#include <variant>
#include <iostream>

template<typename Func>
struct call_context;

template<typename Ret, typename... Args>
struct call_context<Ret(Args...)> {
    using function_type = Ret(Args...);

    // Constructor takes a callable object
    call_context(function_type* func) : func_(func) {}

    // Call operator to invoke the stored callable with the provided arguments
    Ret operator()(Args... args) {
        return func_(std::forward<Args>(args)...);
    }

private:
    function_type* func_; // Pointer to the callable
};
