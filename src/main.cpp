#include <stdexcept>

#include "Solaris.hpp"
#include "terminal/TerminalEmulator.hpp"

auto
main(const int argc, const char** const argv) -> int
{
    try {

        Solaris application(argc, argv);

        application.run();

        return EXIT_SUCCESS;

    } catch (const std::exception& e) {

        Terminal::set_fg_color({ 255, 0, 0 });

        printf("\nFATAL ERROR : %s\n", e.what());

        Terminal::reset_attributes();

        Terminal::flush();

        return EXIT_FAILURE;
    }
}
