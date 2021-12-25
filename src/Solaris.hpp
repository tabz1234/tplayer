#pragma once

#include <filesystem>

struct Solaris final
{

    Solaris(const int argc, const char** const argv);

    void run();

    ~Solaris();

  public:
    std::filesystem::path filepath_;
    bool loop_ = false;
};
