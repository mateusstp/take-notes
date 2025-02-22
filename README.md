# Cross-Platform Audio Recorder

A modern C++ application with a graphical interface for recording audio from your system's input devices. Works on Windows, macOS, and Linux.

## Features

- Graphical user interface using Dear ImGui
- Records audio from any available input device
- Mono recording with 44.1kHz sampling rate
- Custom filename support with automatic .wav extension
- Real-time recording duration display
- Cross-platform compatibility
- Modern and intuitive interface

## Installation

### macOS
1. Download the latest `AudioRecorder-x.x.x-Darwin.dmg`
2. Mount the DMG file
3. Drag 'Audio Recorder.app' to your Applications folder
4. When running for the first time, right-click the app and select 'Open' to bypass Gatekeeper

### Linux (Debian/Ubuntu)
1. Download the latest `AudioRecorder-x.x.x-Linux.deb`
2. Install using:
```bash
sudo dpkg -i AudioRecorder-x.x.x-Linux.deb
sudo apt-get install -f  # Install dependencies if needed
```

### Windows
1. Download the latest `AudioRecorder-x.x.x-Win64.exe`
2. Run the installer
3. Follow the installation wizard

## Building from Source

### Dependencies

- PortAudio (for audio capture)
- libsndfile (for WAV file handling)
- CMake 3.10 or higher
- C++17 compatible compiler
- OpenGL

### Installing Dependencies

#### macOS
```bash
brew install portaudio libsndfile cmake
```

#### Ubuntu/Debian Linux
```bash
sudo apt-get install libportaudio2 libportaudiocpp0 libsndfile1 libportaudio-dev libsndfile1-dev cmake build-essential libgl1-mesa-dev
```

#### Windows
1. Install Visual Studio with C++ support
2. Install CMake
3. Use vcpkg to install dependencies:
```bash
vcpkg install portaudio:x64-windows
vcpkg install libsndfile:x64-windows
```

### Building

1. Clone this repository:
```bash
git clone <repository-url>
cd <repository-directory>
```

2. Create and navigate to build directory:
```bash
mkdir build
cd build
```

3. Generate build files and build:
```bash
cmake ..
cmake --build .
```

4. Create installer (optional):
- macOS: `cpack -G "DragNDrop"`
- Linux: `cpack -G "DEB"`
- Windows: `cpack -G "NSIS"`

## Usage

1. Launch the application
2. Select your input device from the dropdown menu
3. Enter a filename for your recording (optional, defaults to 'recording.wav')
4. Click 'Record' to start recording
5. Click 'Stop' to end recording and save the file

## Troubleshooting

### Permission Issues

#### macOS
- Grant microphone permissions when prompted
- If no prompt appears, check System Preferences → Security & Privacy → Microphone

#### Linux
- Ensure your user is in the 'audio' group:
```bash
sudo usermod -a -G audio $USER
Newgrp audio  # Apply changes without logging out
```

#### Windows
- Check microphone permissions in Windows Settings → Privacy → Microphone
- Run the application as administrator if needed

### No Audio Devices Found
- Ensure your microphone is properly connected
- Check if the device is recognized by your system
- Try unplugging and reconnecting the device

## License

This project is licensed under the MIT License - see the LICENSE file for details.