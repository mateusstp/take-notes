cmake_minimum_required(VERSION 3.10)
project(audio_recorder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pkg-config
find_package(PkgConfig REQUIRED)

# Use pkg-config to find PortAudio and SndFile
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(SNDFILE REQUIRED sndfile)

add_executable(audio_recorder src/main.cpp)

target_link_libraries(audio_recorder PRIVATE
    ${PORTAUDIO_LIBRARIES}
    ${SNDFILE_LIBRARIES}
)

# Include directories
target_include_directories(audio_recorder PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS})

# Add library search paths
target_link_directories(audio_recorder PRIVATE
    ${PORTAUDIO_LIBRARY_DIRS}
    ${SNDFILE_LIBRARY_DIRS})
