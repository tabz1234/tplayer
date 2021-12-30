#include "draw_frame_to_terminal.hpp"

#include "terminal/Terminal.hpp"

void draw_frame_to_terminal(FFmpeg::Frame<FFmpeg::MediaType::video> frame,
                            const std::pair<int, int> position) noexcept {

    const auto& [sp_x, sp_y] = position;

    Terminal::clear();
    for (int y = 0; y < frame.ptr()->height; ++y) {
        Terminal::Cursor::move(sp_x, sp_y + y);

        for (int x = 0; x < frame.ptr()->width; ++x) {

            Terminal::set_fg_color(frame.ptr()->data[0][x + y * frame.ptr()->linesize[0]],
                                   frame.ptr()->data[0][x + y * frame.ptr()->linesize[0]],
                                   frame.ptr()->data[0][x + y * frame.ptr()->linesize[0]]);
            Terminal::write_char('#');
        }
    }
    Terminal::flush();
}
