#pragma once

#undef max

#include <span>
#include <string>
#include <vector>
#include <type_traits>
#include <optional>
#include <limits>

#include "../core/Memtypes.hpp"

#include <iostream>

std::vector<u8> read_file(const std::string& filename);
std::vector<u8> read_file(const std::wstring& filename);
std::vector<uint8_t> get_range(const std::vector<uint8_t>& vec, std::size_t offset, std::size_t length);

template<typename T>
T read_from_vec(const std::vector<u8>& vec, size_t offset) {
    static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable");
    return *reinterpret_cast<const T*>(vec.data() + offset);
}

template <typename FuncType, typename... ExtraArgs>
auto call_function(FuncType func, u64 value1, u64 value2, ExtraArgs... extraArgs)
-> decltype(func(value1, value2, extraArgs...)) {
    return func(value1, value2, extraArgs...);
}

template <typename FuncType, typename... ExtraArgs>
auto call_function(FuncType func, u64 value1, ExtraArgs... extraArgs)
-> decltype(func(value1, extraArgs...)) {
    return func(value1, extraArgs...);
}

template <typename T>
std::vector<uint8_t> to_byte_vec(T value) {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    std::vector<uint8_t> bytes(sizeof(T));

    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = static_cast<uint8_t>(value >> (i * 8));
    }

    return bytes;
}

std::optional<u64> ReadU64FromVec(const std::vector<uint8_t>& vec, size_t size, size_t offset);

template <typename T>
std::optional<T> read_disp_from_inst(const std::vector<uint8_t>& vec, size_t size_bit, size_t offset) {
    size_t size = size_bit >> 3; // Convert bits to bytes

    std::cout << "read_disp_from_inst(): size " << size << "\n";

    if (offset + size > vec.size() || size > sizeof(T)) {
        return std::nullopt;
    }

    T data = 0;

    if (size == 1) {
        data = static_cast<T>(*reinterpret_cast<const uint8_t*>(vec.data() + offset));
    }
    else if (size == 2) {
        data = static_cast<T>(*reinterpret_cast<const uint16_t*>(vec.data() + offset));
    }
    else if (size == 4) {
        data = static_cast<T>(*reinterpret_cast<const uint32_t*>(vec.data() + offset));
    }
    else {
        return std::nullopt;
    }

    std::cout << "\ndisplacement: 0x" << std::hex << data << "\n";

    return data;
}
template <typename T>
T CastFromVec(const std::vector<uint8_t>& vec, size_t offset) {
    return *reinterpret_cast<const T*>(vec.data() + offset);
}

template <typename Tret, typename Tx>
std::optional<Tret> wrap_around(std::optional<Tx> val) {
    return val.has_value() ? std::optional(reinterpret_cast<Tret>(val.value())) : std::nullopt;
}

template <typename T> requires std::is_integral_v<T>
std::optional<T> checked_add(const T x1, const T x2) {
    if (((x1 / 2) + (x2 / 2)) > (std::numeric_limits<T>::max() / 2)) //will overflowed
        return std::nullopt;
    return x1 + x2;
}

uint64_t ReadFromU64(uint64_t value, uint8_t size);

std::string wstring_to_string(const std::wstring& wstr);