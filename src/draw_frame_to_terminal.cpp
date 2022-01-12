#include "draw_frame_to_terminal.hpp"

#include "terminal/Terminal.hpp"

#include <vector>

template <std::integral IntT, std::integral auto buff_size_>
std::string_view static integer_to_chars(const IntT int_val,
                                         std::array<char, buff_size_>& buff_view) {

    const auto [ptr, ec] =
        std::to_chars(buff_view.data(), buff_view.data() + buff_view.size(), int_val);

    return {buff_view.data(), ptr};
}
void draw_frame_to_terminal(FFmpeg::Frame<FFmpeg::MediaType::video> frame,
                            const std::pair<int, int> position) noexcept {

    const auto& [sp_x, sp_y] = position;

    static std::vector<char> buff;
    buff.reserve(frame.ptr()->linesize[0] * (frame.ptr()->height + 1));

    Terminal::Cursor::move(1, 1);

    static std::array<char, 20> tbf;
    for (int y = 0; y < frame.ptr()->height; ++y) {
        for (int x = 0, itr = 0; itr < frame.ptr()->width; itr++, x = x + 4) {
#if 0 
            printf("%d|%d\n", frame.ptr()->width, frame.ptr()->height);
#endif
            const int a = frame.ptr()->data[0][x + y * frame.ptr()->linesize[0]];
            const int b = frame.ptr()->data[0][x + y * frame.ptr()->linesize[0] + 1];
            const int c = frame.ptr()->data[0][x + y * frame.ptr()->linesize[0] + 2];

            for (const auto ch : std::string_view{"\033[38;2;"}) {
                buff.push_back(ch);
            }
            for (const auto ch : integer_to_chars<uint8_t, 20>(a, tbf)) {
                buff.push_back(ch);
            }
            buff.push_back(';');

            for (const auto ch : integer_to_chars<uint8_t, 20>(b, tbf)) {
                buff.push_back(ch);
            }
            buff.push_back(';');

            for (const auto ch : integer_to_chars<uint8_t, 20>(c, tbf)) {
                buff.push_back(ch);
            }
            buff.push_back('m');

            buff.push_back('#');
        }
        buff.push_back('\r');
        buff.push_back('\n');
    }
    write(STDOUT_FILENO, buff.data(), buff.size());

    buff.clear();
}
