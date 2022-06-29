#pragma once

#include <string_view>

namespace fs {
    long long load_file_str(const std::string_view path, std::size_t read_limit, char* out) noexcept;
} // namespace fs
