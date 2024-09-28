#pragma once

#include <variant>

#include "Immediate.hpp"
#include "AddressType.hpp"
#include "RegisterType.hpp"

using operand_type = std::variant<ImmValue, AddressValue, RegisterValue>;