cmake_minimum_required(VERSION 3.21)
project(tbb)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TBB REQUIRED IMPORTED_TARGET tbb)

add_library(tbb INTERFACE IMPORTED GLOBAL)

target_link_libraries(tbb INTERFACE
    PkgConfig::TBB
)
