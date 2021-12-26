#include <stdexcept>

#include "Solaris.hpp"
#include "terminal/TerminalEmulator.hpp"

int
main(const int argc, const char** const argv)
{
    try {

        Solaris application(argc, argv);

        application.run();

        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        Solaris::print_msg_header();

        Terminal::out<Terminal::RGB{ 255, 0, 0 }>(" _FATAL_ERROR_");
        Terminal::out(" : ", e.what());

        return EXIT_FAILURE;
    }
}
