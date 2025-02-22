cmake_minimum_required(VERSION 3.10)
project(audio_recorder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pkg-config
find_package(PkgConfig REQUIRED)

# Use pkg-config to find PortAudio and SndFile
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(SNDFILE REQUIRED sndfile)

# Find OpenGL
if (APPLE)
    find_package(OpenGL REQUIRED)
    link_directories(/System/Library/Frameworks)
    include_directories(/System/Library/Frameworks/OpenGL.framework/Headers)
    include_directories(/System/Library/Frameworks/GLUT.framework/Headers)
else()
    find_package(OpenGL REQUIRED)
endif()

# Download and include ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90
)
FetchContent_MakeAvailable(imgui)

# Download and include GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(audio_recorder 
    src/main.cpp
    ${IMGUI_SOURCES}
)

target_link_libraries(audio_recorder PRIVATE
    ${PORTAUDIO_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    glfw
    OpenGL::GL
)

# Include directories
target_include_directories(audio_recorder PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Add library search paths
target_link_directories(audio_recorder PRIVATE
    ${PORTAUDIO_LIBRARY_DIRS}
    ${SNDFILE_LIBRARY_DIRS}
)
