#include <charconv>
#include <string_view>

template <typename IntT>
std::string_view static integer_to_chars(const IntT int_val, char* buff, std::size_t buff_size)
{
    const auto [ptr, ec] = std::to_chars(buff, buff + buff_size, int_val);
    return {buff, ptr};
}
