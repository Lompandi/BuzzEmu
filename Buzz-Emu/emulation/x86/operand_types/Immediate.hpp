#pragma once

#include <variant>

#include "../../../core/Memtypes.hpp"

struct ImmValue {
public:
    // Using std::variant to hold various integral types
    using ValueType = std::variant<s8, u8, s16, u16, s32, u32, s64, u64>;

    ImmValue(ValueType value) noexcept : value_(value) {}

    [[nodiscard]] ValueType get() const {
        return value_;
    }

    void set(ValueType value) {
        value_ = value;
    }

    ImmValue& operator=(const ImmValue& other) {
        if (this != &other) {
            value_ = other.value_;
        }
        return *this;
    }

private:
    ValueType value_;
};