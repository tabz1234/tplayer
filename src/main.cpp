#include <stdexcept>

#include "Solaris.hpp"
#include "terminal/TerminalEmulator.hpp"

int main(const int argc, const char** const argv) {

    try {

        Solaris application(argc, argv);

        application.run();

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {

        Solaris::print_msg_prefix();

        Terminal::out<Terminal::RGB{255, 0, 0}>(" FATAL ERROR ");
        Terminal::out(" : ", e.what());

        return EXIT_FAILURE;
    }
}
