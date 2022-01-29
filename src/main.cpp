#include <stdexcept>

#include "Tplayer.hpp"
#include "terminal/Terminal.hpp"

int main(const int argc, const char** const argv)
{

    try {
        Tplayer application(argc, argv);
        application.run();
    }
    catch (const std::exception& e) {

        Tplayer::print_msg_prefix();
        Terminal::out(RGB_t{255, 0, 0}, "FATAL ERROR", Terminal::DefaultAttr, " : ", e.what());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
