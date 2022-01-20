#pragma once

template <typename T>
class ptr_view final {

    T* const raw_ptr_;

  public:
    constexpr ptr_view(T* raw_ptr) noexcept : raw_ptr_{raw_ptr} {
    }

    constexpr const T* get() const noexcept {
        return raw_ptr_;
    }
    constexpr T* get() noexcept {
        return raw_ptr_;
    }

    constexpr ptr_view(const ptr_view& lval) noexcept : raw_ptr_{lval.raw_ptr_} {
    }
    constexpr ptr_view& operator=(const ptr_view& lval) noexcept {
        raw_ptr_ = lval.raw_ptr_;
    }

    constexpr T operator*() const noexcept {
        return *raw_ptr_;
    }
    constexpr T* operator->() const noexcept {
        return raw_ptr_;
    }
};
