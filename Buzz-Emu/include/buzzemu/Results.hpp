#pragma once

#include <variant>

namespace bzmu {
	template <class E>
	class result_error {
		template<typename T, typename R>
		friend class result;
	public:
		constexpr explicit result_error(const E& t) : error(t) {}
		constexpr explicit result_error(E&& t) : error(std::move(t)) {}

		constexpr result_error(const result_error<E>&) = default;
		constexpr result_error(result_error<E>&&) noexcept = default;
	private:
		E error;
	};

	template <class T, class E>
	class result {
	public:
		constexpr result(const T& t) : f_result(std::in_place_index<0>, t) {}
		constexpr result(T&& t) : f_result(std::in_place_index<0>, std::move(t)) {}

		constexpr result(const result_error<E>& e) : f_result(std::in_place_index<1>, e.error) {}
		constexpr result(result_error<E>&& e) : f_result(std::in_place_index<1>, std::move(e.error)) {}

		constexpr result(const result<T, E>&) = default;
		constexpr result(result<T, E>&&) noexcept = default;

		[[nodiscard]] constexpr bool has_value() const {
			return f_result.index() == 0;
		}

		constexpr const T& value() noexcept {
			return std::get<0>(f_result);
		}

		constexpr const T& value() const noexcept {
			return std::get<0>(f_result);
		}

		constexpr E& error() noexcept {
			return std::get<1>(f_result);
		}

		constexpr const E& error() const noexcept {
			return std::get<1>(f_result);
		}
	private:
		std::variant<T, E> f_result;
	};
}