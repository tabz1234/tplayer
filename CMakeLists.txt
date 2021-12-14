# basic configuration :
set(Label Solaris)
set(Standard 20)
set(Link_libs ffmpeg openal -lpthread)

set(DEFAULT_ADDITIONAL_DEBUG_OPTIONS -O0 -Wall -Wextra -Weffc++ -pedantic )

set(DEFAULT_ADDITIONAL_RELEASE_OPTIONS )

# cmake code :
cmake_minimum_required(VERSION 3.21)

project(${Label})

add_subdirectory(lib/openal)
add_subdirectory(lib/ffmpeg)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD ${Standard})
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(sources
	./src/main.cpp

	./src/ffmpeg/FFmpegVideoLoader.cpp
	./src/ffmpeg/FFmpegAudioFormatConverter.cpp

	./src/openal/OpenAlDevice.cpp
	./src/openal/OpenAlStreamingSource.cpp
	./src/openal/OpenAlBuffer.cpp

	./src/terminal/TerminalEmulator.cpp
	
)
add_executable(${PROJECT_NAME} ${sources})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
enable_testing()
  target_compile_options(${PROJECT_NAME}
                         PUBLIC ${DEFAULT_ADDITIONAL_DEBUG_OPTIONS})
else()
  target_compile_options(${PROJECT_NAME}
                         PUBLIC ${DEFAULT_ADDITIONAL_RELEASE_OPTIONS})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${Link_libs})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src /usr/include)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
	COMMAND cp ${CMAKE_BINARY_DIR}/${PROJECT_NAME} ${CMAKE_SOURCE_DIR})
endif()



