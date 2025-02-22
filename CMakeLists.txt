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

if(APPLE)
    add_executable(audio_recorder MACOSX_BUNDLE
        src/main.cpp
        ${IMGUI_SOURCES}
    )
    set_target_properties(audio_recorder PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.audiorecorder.app"
        MACOSX_BUNDLE_BUNDLE_NAME "Audio Recorder"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
else()
    add_executable(audio_recorder 
        src/main.cpp
        ${IMGUI_SOURCES}
    )
endif()

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

# Installation configuration
if(APPLE)
    set(INSTALL_BIN_DIR "/Applications" CACHE PATH "Installation directory for executables")
elseif(UNIX)
    set(INSTALL_BIN_DIR "bin" CACHE PATH "Installation directory for executables")
    set(INSTALL_LIB_DIR "lib" CACHE PATH "Installation directory for libraries")
    set(INSTALL_DATA_DIR "share/${PROJECT_NAME}" CACHE PATH "Installation directory for data files")
else()
    set(INSTALL_BIN_DIR "." CACHE PATH "Installation directory for executables")
    set(INSTALL_LIB_DIR "." CACHE PATH "Installation directory for libraries")
    set(INSTALL_DATA_DIR "." CACHE PATH "Installation directory for data files")
endif()

# Install targets
install(TARGETS audio_recorder
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    BUNDLE DESTINATION ${INSTALL_BIN_DIR}
)

# CPack configuration
set(CPACK_PACKAGE_NAME "AudioRecorder")
set(CPACK_PACKAGE_VENDOR "Your Company")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio Recording Application")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# Platform specific packaging
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AudioRecorder")
    set(CPACK_DMG_FORMAT "UDZO")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "Audio Recorder")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "portaudio19-dev, libsndfile1-dev")
endif()

include(CPack)
