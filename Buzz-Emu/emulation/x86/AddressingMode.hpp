#pragma once

enum class AddressingMode {
    REG,      // Register addressing
    DISP8,    // Displacement 8-bit
    DISP16,    // Displacement 16-bit
    DISP32,
    DISP64,
};