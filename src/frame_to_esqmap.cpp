#include "frame_to_esqmap.hpp"

#include "terminal/Terminal.hpp"

#include <array>
#include <concepts>
#include <vector>

template <std::integral IntT, std::integral auto buff_size_>
std::string_view static integer_to_chars(const IntT int_val, std::array<char, buff_size_>& buff_view)
{

    const auto [ptr, ec] = std::to_chars(buff_view.data(), buff_view.data() + buff_view.size(), int_val);

    return {buff_view.data(), ptr};
}
std::string frame_to_esqmap(const AVFrame* const frame)
{

    std::string esqmap;
    esqmap.reserve(static_cast<long>(frame->linesize[0]) * (frame->height + 1));

    esqmap += "\033[0;0H";

    constexpr auto color_buff_size = 4;
    std::array<char, color_buff_size> color_buff;

    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0, itr = 0; itr < frame->width; ++itr, x = x + 4) {

            const auto a = frame->data[0][x + y * frame->linesize[0]];
            const auto b = frame->data[0][x + y * frame->linesize[0] + 1];
            const auto c = frame->data[0][x + y * frame->linesize[0] + 2];

            esqmap += "\033[48;2;";
            esqmap += integer_to_chars<uint8_t, color_buff_size>(a, color_buff);
            esqmap += ';';
            esqmap += integer_to_chars<uint8_t, color_buff_size>(b, color_buff);
            esqmap += ';';
            esqmap += integer_to_chars<uint8_t, color_buff_size>(c, color_buff);
            esqmap += 'm';

            esqmap += ' ';
        }
        esqmap += "\r\n";
    }
    esqmap.pop_back(); // remove last newline
    esqmap.pop_back();

    return esqmap;
}
