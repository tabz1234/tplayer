cmake_minimum_required(VERSION 3.21)
project(openal)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENAL REQUIRED IMPORTED_TARGET openal)

add_library(openal INTERFACE IMPORTED GLOBAL)

target_link_libraries(openal INTERFACE
    PkgConfig::OPENAL
)
