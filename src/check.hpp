#pragma once

#include <concepts>
#include <cstdlib>
#include <stdexcept>
#include <string_view>

template <typename ExceptionT = std::runtime_error>
requires std::derived_from<ExceptionT, std::exception>
constexpr auto check(const bool expr, std::string_view msg) {
    if (!expr) [[unlikely]] {
        throw ExceptionT{msg.data()};
    }
}
