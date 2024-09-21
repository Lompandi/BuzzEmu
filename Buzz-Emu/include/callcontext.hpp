#pragma once

#include <type_traits>
#include <utility> // For std::forward
#include <variant>
#include <iostream>

namespace bzmu::detail {
	template<typename Func, typename... Args>
	constexpr bool is_return_void() {
		using ReturnType = std::invoke_result_t<Func, Args...>;
		return std::is_void_v<ReturnType>;
	}
}