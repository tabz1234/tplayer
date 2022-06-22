cmake_minimum_required(VERSION 3.18)


set(ProjectName tplayer)

project(${ProjectName} LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)

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

option(ENABLE_PCH "Enable Precompiled Headers" ON)
if(ENABLE_PCH)
  target_precompile_headers(
    project_options
    INTERFACE
    <vector>
    <string>
    <chrono>
    <queue>
    <iostream>
    <cstdlib>
    <thread>
    <type_traits>
    <optional>
    <memory>
    <utility>)
endif()

add_executable(${ProjectName}
	./src/tplayer/main.cpp
	./src/tplayer/Settings.cpp
	./src/tplayer/parse_cmd_arguments.cpp
	./src/tplayer/init_cache_space.cpp
	./src/tplayer/compute_scaling_factor.cpp
	./src/tplayer/end_write_frame.cpp
	./src/tplayer/prepare_to_write_video_frame.cpp
	./src/tplayer/write_video_frame.cpp
	./src/tplayer/write_audio_frame.cpp
	./src/tplayer/prepare_to_write_audio_frame.cpp

	./src/ffmpeg/Format.cpp
	./src/ffmpeg/FormatHandle.cpp
	./src/ffmpeg/Packet.cpp
	./src/ffmpeg/read_packet.cpp
	./src/ffmpeg/Codec.cpp
	./src/ffmpeg/Frame.cpp
	./src/ffmpeg/decode_packet.cpp
	./src/ffmpeg/HWAccelCodec.cpp
	./src/ffmpeg/decode_multi_packet.cpp
	./src/ffmpeg/shrink_audio_size_to_content.cpp
	./src/ffmpeg/scale_video_frame.cpp
	./src/ffmpeg/SWScaler.cpp
	./src/ffmpeg/resample_audio_frame.cpp
	./src/ffmpeg/SWResampler.cpp

	./src/terminal/get_window_size.cpp

	)

add_subdirectory(./lib/ffmpeg)
add_subdirectory(./lib/openal)

target_link_libraries(${ProjectName} -lpthread)
target_link_libraries(${ProjectName} project_options project_warnings)
target_link_libraries(${ProjectName} ffmpeg openal)
