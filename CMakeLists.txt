# basic configuration :
set(Label Solaris)
set(Standard 20)
set(Link_libs ffmpeg openal tbb -lpthread)

set(DEFAULT_ADDITIONAL_DEBUG_OPTIONS -O0 -Wall -Wextra -Weffc++ -pedantic )

set(DEFAULT_ADDITIONAL_RELEASE_OPTIONS )

# cmake code :
cmake_minimum_required(VERSION 3.21)

project(${Label})

add_subdirectory(lib/openal)
add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/tbb)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD ${Standard})
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB FFMPEG_SRC "src/ffmpeg/*.c*")
file(GLOB OPENAL_SRC "src/openal/*.c*")
file(GLOB TERMINAL_SRC "src/terminal/*.c*")
file(GLOB BASE_SRC "src/*.c*")

add_executable(${PROJECT_NAME} ${FFMPEG_SRC} ${TERMINAL_SRC} ${OPENAL_SRC} ${BASE_SRC})

if(CMAKE_BUILD_TYPE STREQUAL "debug")
enable_testing()
  target_compile_options(${PROJECT_NAME}
                         PUBLIC ${DEFAULT_ADDITIONAL_DEBUG_OPTIONS})
else()
  target_compile_options(${PROJECT_NAME}
                         PUBLIC ${DEFAULT_ADDITIONAL_RELEASE_OPTIONS})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${Link_libs})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src /usr/include)

