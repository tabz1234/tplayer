cmake_minimum_required(VERSION 3.20)


set(ProjectName tplayer)

project(${ProjectName} CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

add_library(project_warnings INTERFACE)

include(cmake/Cache.cmake)

include(cmake/Linker.cmake)
configure_linker(project_options)

include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

include(cmake/StaticAnalyzers.cmake)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
  target_precompile_headers(
    project_options
    INTERFACE
    <vector>
    <string>
    <iostream>
    <type_traits>
    <map>
    <list>
    <forward_list>
    <optional>
    <stdexcept>
    <memory>
    <utility>)
endif()

add_executable(${ProjectName}
	./src/main.cpp
	./src/Tplayer.cpp
	./src/draw_frame_to_terminal.cpp

	./src/terminal/Terminal.cpp

	./src/ffmpeg/InputHandle.cpp
	./src/ffmpeg/scale_video_buffer.cpp
	./src/ffmpeg/convert_audio_buffer.cpp
	./src/ffmpeg/shrink_audio_size_to_content.cpp

	./src/openal/Buffer.cpp
	./src/openal/StreamingSource.cpp
	./src/openal/Device.cpp

	)

add_subdirectory(./lib/tbb)
add_subdirectory(./lib/ffmpeg)
add_subdirectory(./lib/openal)

target_link_libraries(${ProjectName} -lpthread)
target_link_libraries(${ProjectName} project_options project_warnings)
target_link_libraries(${ProjectName} tbb ffmpeg openal)
