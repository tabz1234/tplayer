#include "load_file_str.hpp"

#include <cstdio>

namespace fs {
    long long load_file_str(const std::string_view path, std::size_t read_limit, char* out) noexcept
    {
        std::size_t ret;

        FILE* f = fopen(path.data(), "rb");
        if (!f) [[unlikely]] {
            return -1;
        }

        ret = fread(out, 1, read_limit, f);

        fclose(f);

        return ret;
    }
} // namespace fs
