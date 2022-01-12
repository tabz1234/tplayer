#pragma once

#include <filesystem>

struct Tplayer final {

    Tplayer(const int argc, const char** const argv);

    void run();
    static void print_msg_prefix();

    ~Tplayer();

  public:
    std::filesystem::path filepath_;
    bool loop_ = false;
};
