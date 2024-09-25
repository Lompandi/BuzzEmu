
#include <windows.h>

#include "Strings.hpp"

namespace bzmu {
    std::wstring bytes_to_wstring(const char* bytes) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, bytes, -1, nullptr, 0);
        std::wstring wstr(size_needed - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, bytes, -1, wstr.data(), size_needed - 1);
        return wstr;
    }
}