#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string_view>

auto inline check(const bool expr, std::string_view msg)
{
    if (!expr) [[unlikely]] {
        throw std::runtime_error(msg.data());
    }
}
