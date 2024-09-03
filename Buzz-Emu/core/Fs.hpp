#pragma once

#include <string>
#include <vector>
#include <type_traits>
#include <optional>

#include "../core/Memtypes.hpp"

std::vector<u8> read_file(const std::string& filename);
std::vector<uint8_t> get_range(const std::vector<uint8_t>& vec, std::size_t offset, std::size_t length);

template<typename T>
T ReadFromVec(const std::vector<u8>& vec, size_t offset) {
    T result;
    std::memcpy(&result, vec.data() + offset, sizeof(T));
    return result;
}

template <typename T>
std::vector<uint8_t> ToByteVector(T value) {
    static_assert(std::is_integral_v<T>, "T must be an integral type");

    std::vector<uint8_t> bytes(sizeof(T));

    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = static_cast<uint8_t>(value >> (i * 8));
    }

    return bytes;
}

std::optional<u64> ReadU64FromVec(const std::vector<uint8_t>& vec, size_t size, size_t offset);

template <typename T>
std::optional<T> ReadFromVec(const std::vector<uint8_t>& vec, size_t size, size_t offset) {
    static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable");

    if (offset >= vec.size() || size < sizeof(T)) 
        return std::nullopt;

    size_t bytes_to_copy = std::min(size, vec.size() - offset);
    if (bytes_to_copy < sizeof(T)) 
        return std::nullopt;

    T value;
    std::memcpy(&value, vec.data() + offset, sizeof(T));

    return value;
}