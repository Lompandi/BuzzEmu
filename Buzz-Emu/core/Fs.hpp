#pragma once

#include <span>
#include <string>
#include <vector>
#include <type_traits>
#include <optional>

#include "../core/Memtypes.hpp"

std::vector<u8> read_file(const std::string& filename);
std::vector<uint8_t> get_range(const std::vector<uint8_t>& vec, std::size_t offset, std::size_t length);

template<typename T>
T ReadFromVec(const std::vector<u8>& vec, size_t offset) {
    static_assert(std::is_trivially_copyable<T>::value, "Type T must be trivially copyable");
    return *reinterpret_cast<const T*>(vec.data() + offset);
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
std::optional<T> ReadFromVec(const std::vector<uint8_t>& vec, size_t size_bit, size_t offset) {
    size_t size = size_bit >> 3; // Convert bits to bytes

    std::cout << "ReadFromVec(): size " << size << "\n";
    if (offset + size > vec.size() || size > sizeof(T)) 
        return std::nullopt; 
    return (T)(*(vec.data() + offset));
}

template <typename T>
T CastFromVec(const std::vector<uint8_t>& vec, size_t offset) {
    return *reinterpret_cast<const T*>(vec.data() + offset);
}

uint64_t ReadFromU64(uint64_t value, uint8_t size);