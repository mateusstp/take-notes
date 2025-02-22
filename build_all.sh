#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print with color
print_status() {
    echo -e "${GREEN}==>${NC} $1"
}

print_error() {
    echo -e "${RED}Error:${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}Warning:${NC} $1"
}

# Check if Docker is installed
check_docker() {
    if ! command -v docker &> /dev/null; then
        print_error "Docker is required for cross-compilation but it's not installed."
        exit 1
    fi
}

# Create build directory if it doesn't exist
create_build_dir() {
    local platform=$1
    local build_dir="build_${platform}"
    mkdir -p "$build_dir"
    echo "$build_dir"
}

# Build for macOS (native)
build_macos() {
    print_status "Building for macOS..."
    local build_dir=$(create_build_dir "macos")
    
    # Build using native tools
    (cd "$build_dir" && \
        cmake .. \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 && \
        cmake --build . && \
        cpack -G "DragNDrop") || {
            print_error "macOS build failed"
            return 1
        }
    
    print_status "macOS build completed successfully"
    return 0
}

# Build for Linux using Docker
build_linux() {
    print_status "Building for Linux..."
    local build_dir=$(create_build_dir "linux")
    
    # Use multiarch image for x86_64
    print_status "Pulling Ubuntu multiarch image..."
    docker pull --platform linux/amd64 ubuntu:20.04 || {
        print_error "Failed to pull Ubuntu image"
        return 1
    }
    
    # Create Linux build container
    print_status "Starting Linux build..."
    docker run --platform linux/amd64 --rm -v "$(pwd):/src" \
        ubuntu:20.04 /bin/bash -c "
            set -e
            echo 'Updating package lists...'
            apt-get update && \
            echo 'Installing dependencies...'
            DEBIAN_FRONTEND=noninteractive apt-get install -y \
                build-essential \
                cmake \
                portaudio19-dev \
                libsndfile1-dev \
                libgl1-mesa-dev \
                pkg-config && \
            echo 'Starting CMake build...'
            cd /src/$build_dir && \
            cmake .. \
                -DCMAKE_BUILD_TYPE=Release && \
            cmake --build . && \
            cpack -G DEB" || {
                print_error "Linux build failed"
                return 1
            }
    
    print_status "Linux build completed successfully"
    return 0
}

# Build for Windows using Docker
build_windows() {
    print_status "Building for Windows..."
    local build_dir=$(create_build_dir "windows")
    
    # Pull Windows cross-compilation image first
    print_status "Pulling Windows cross-compilation image..."
    docker pull --platform linux/amd64 dockcross/windows-static-x64 || {
        print_error "Failed to pull Windows cross-compilation image"
        return 1
    }
    
    # Create Windows build container
    print_status "Starting Windows build..."
    docker run --platform linux/amd64 --rm -v "$(pwd):/src" \
        dockcross/windows-static-x64 /bin/bash -c "
            set -e
            echo 'Starting CMake build...'
            cd /src/$build_dir && \
            cmake .. \
                -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_TOOLCHAIN_FILE=/usr/src/mxe/usr/x86_64-w64-mingw32.static/share/cmake/mxe-conf.cmake && \
            cmake --build . && \
            cpack -G NSIS" || {
                print_error "Windows build failed"
                return 1
            }
    
    print_status "Windows build completed successfully"
    return 0
}

# Main build process
main() {
    print_status "Starting multi-platform build process..."
    
    # Check requirements
    check_docker
    
    # Build for each platform
    build_macos || print_warning "macOS build failed"
    build_linux || print_warning "Linux build failed"
    build_windows || print_warning "Windows build failed"
    
    # Show results
    echo -e "\nBuild Results:"
    echo "=============="
    
    # Check for generated packages
    for platform in "macos" "linux" "windows"; do
        echo -e "\n${GREEN}${platform}:${NC}"
        find "build_${platform}" -name "*.dmg" -o -name "*.deb" -o -name "*.exe" 2>/dev/null | \
            sed 's/^/  /' || echo "  No packages found"
    done
}

# Run the build process
main
