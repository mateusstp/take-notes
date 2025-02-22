# Cross-Platform Audio Recorder

A simple C++ application that records audio from your system's default input device and saves it as a WAV file. Works on Windows, macOS, and Linux.

## Dependencies

This application requires:
- PortAudio (for audio capture)
- libsndfile (for WAV file handling)
- CMake (for building)
- A C++17 compatible compiler

### Installing Dependencies

#### macOS
```bash
brew install portaudio libsndfile
```

#### Ubuntu/Debian Linux
```bash
sudo apt-get install libportaudio2 libportaudiocpp0 libsndfile1 libportaudio-dev libsndfile1-dev
```

#### Windows
Either:
- Use vcpkg to install PortAudio and libsndfile
- Download pre-built binaries from their respective websites
- Build from source

## Building the Application

1. Clone this repository:
```bash
git clone <repository-url>
cd <repository-directory>
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Generate build files with CMake:
```bash
cmake ..
```

4. Build the application:
```bash
cmake --build .
```

## Running the Application

1. Run the executable:
```bash
./audio_recorder
```

2. Follow the on-screen instructions:
   - Press Enter to start recording
   - Press Enter again to stop recording
   - The recording will be saved as 'recording.wav' in the current directory

## Features

- Records audio from the default input device
- Stereo recording (2 channels)
- 44.1kHz sampling rate
- Saves in WAV format
- Cross-platform compatibility

## Troubleshooting

If you encounter permission issues accessing the audio device:

- On Linux, ensure your user is part of the 'audio' group:
```bash
sudo usermod -a -G audio $USER
```

- On macOS, grant microphone permissions to your terminal application

- On Windows, ensure your microphone is enabled in Privacy Settings