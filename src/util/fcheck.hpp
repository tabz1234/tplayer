#pragma once

template <typename LambdaT>
constexpr auto fcheck(const bool expr, LambdaT&& f) noexcept
{
    if (!expr) [[unlikely]] {
        f();
    }
}
