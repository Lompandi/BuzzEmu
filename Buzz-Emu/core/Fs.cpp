
#define NOMINMAX

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <comutil.h>

#include "Fs.hpp"

#pragma comment(lib, "comsuppw.lib")

std::vector<u8> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        //throw std::runtime_error("Failed to read file: " + filename);
        return buffer; //TMP
    }

    return buffer;
}


std::vector<u8> read_file(const std::wstring& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + std::string(filename.begin(), filename.end()));
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<u8> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        //throw std::runtime_error("Failed to read file: " + std::string(filename.begin(), filename.end()));
        return buffer; //TMP
    }

    return buffer;
}

std::vector<u8> get_range(const std::vector<uint8_t>& vec, std::size_t offset, std::size_t length) {
    if (offset >= vec.size()) 
        throw std::out_of_range("Offset is out of range");

    length = std::min(length, vec.size() - offset);
    return std::vector<uint8_t>(vec.begin() + offset, vec.begin() + offset + length);
}

std::optional<u64> ReadU64FromVec(const std::vector<u8>& vec, size_t size, size_t offset) {
    u64 value = 0;
    if (offset >= vec.size())
        return std::nullopt;

    size_t num_bytes = std::min(size, std::min(static_cast<size_t>(8), vec.size() - offset));

    if (num_bytes == 0)
        return std::nullopt;
    std::memcpy(&value, vec.data() + offset, num_bytes);
    return value;
}

uint64_t ReadFromU64(uint64_t value, uint8_t size) {
    // Create a mask based on the size
    uint64_t mask = ((1ULL << size) - 1);

    // Apply mask to the value
    return value & mask;
}

std::string wstring_to_string(const std::wstring& wstr) {
    _bstr_t t = wstr.c_str();
    char* pchar = (char*)t;
    return pchar;
}