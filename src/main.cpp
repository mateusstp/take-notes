#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <vector>
#include <cstring>
#include <thread>
#include <atomic>
#include <algorithm>
#include <string>

class AudioRecorder {
private:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int FRAMES_PER_BUFFER = 256;
    static constexpr int INPUT_CHANNELS = 1;  // Mono input
    static constexpr int OUTPUT_CHANNELS = 1;  // Mono output
    
    PaStream* stream;
    std::vector<float> recordedData;
    std::atomic<bool> isRecording;
    bool recordInputAndOutput;

    static int recordCallback(const void* inputBuffer, void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData) {
        AudioRecorder* recorder = static_cast<AudioRecorder*>(userData);
        const float* input = static_cast<const float*>(inputBuffer);
        float* output = static_cast<float*>(outputBuffer);

        if (recorder->isRecording.load()) {
            size_t numSamples = framesPerBuffer * INPUT_CHANNELS;
            size_t currentSize = recorder->recordedData.size();
            recorder->recordedData.resize(currentSize + numSamples);

            if (input) {
                std::memcpy(recorder->recordedData.data() + currentSize, input, numSamples * sizeof(float));
            }
        }

        // Fill output buffer with silence to prevent feedback
        if (output) {
            std::memset(output, 0, framesPerBuffer * OUTPUT_CHANNELS * sizeof(float));
        }

        return paContinue;
    }

public:
    AudioRecorder() : stream(nullptr), isRecording(false), recordInputAndOutput(true) {
        Pa_Initialize();
    }

    ~AudioRecorder() {
        if (stream) {
            Pa_CloseStream(stream);
        }
        Pa_Terminate();
    }

    void listAudioDevices() {
        int numDevices = Pa_GetDeviceCount();
        std::cout << "Available audio devices:\n";
        
        for (int i = 0; i < numDevices; i++) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
            std::cout << "Device " << i << ": " << deviceInfo->name << "\n";
            std::cout << "  Max input channels: " << deviceInfo->maxInputChannels << "\n";
            std::cout << "  Max output channels: " << deviceInfo->maxOutputChannels << "\n";
        }
        
        std::cout << "Default input device: " << Pa_GetDefaultInputDevice() << "\n";
        std::cout << "Default output device: " << Pa_GetDefaultOutputDevice() << "\n";
    }

    bool startRecording() {
        // Get default devices
        PaStreamParameters inputParameters;
        inputParameters.device = Pa_GetDefaultInputDevice();
        if (inputParameters.device == paNoDevice) {
            std::cerr << "No default input device found\n";
            return false;
        }
        inputParameters.channelCount = INPUT_CHANNELS;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        PaStreamParameters outputParameters;
        outputParameters.device = Pa_GetDefaultOutputDevice();
        if (outputParameters.device == paNoDevice) {
            std::cerr << "No default output device found\n";
            return false;
        }
        outputParameters.channelCount = OUTPUT_CHANNELS;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_OpenStream(&stream,
                                  &inputParameters,
                                  &outputParameters,
                                  SAMPLE_RATE,
                                  FRAMES_PER_BUFFER,
                                  paClipOff,
                                  recordCallback,
                                  this);
        
        if (err != paNoError) {
            std::cerr << "Error opening stream: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        recordedData.clear();
        isRecording = true;
        err = Pa_StartStream(stream);
        
        if (err != paNoError) {
            std::cerr << "Error starting stream: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        return true;
    }

    void stopRecording() {
        if (stream && isRecording.load()) {
            isRecording = false;
            Pa_StopStream(stream);
        }
    }

    bool saveToFile(const std::string& filename) {
        SF_INFO sfInfo;
        std::memset(&sfInfo, 0, sizeof(sfInfo));
        sfInfo.samplerate = SAMPLE_RATE;
        sfInfo.channels = INPUT_CHANNELS;  // Save with the same number of channels as input
        sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

        SNDFILE* file = sf_open(filename.c_str(), SFM_WRITE, &sfInfo);
        if (!file) {
            std::cerr << "Error opening file for writing: " << sf_strerror(file) << std::endl;
            return false;
        }

        sf_count_t written = sf_write_float(file, recordedData.data(), recordedData.size());
        sf_close(file);

        return written == static_cast<sf_count_t>(recordedData.size());
    }
};

int main() {
    AudioRecorder recorder;
    std::cout << "Audio Recorder (Input + Output)\n";
    std::cout << "Available audio devices:\n";
    recorder.listAudioDevices();
    std::cout << "\nPress Enter to start recording...\n";
    std::cin.get();

    if (!recorder.startRecording()) {
        std::cerr << "Failed to start recording\n";
        return 1;
    }

    std::cout << "Recording... Press Enter to stop.\n";
    std::cin.get();

    recorder.stopRecording();
    
    std::cout << "Recording stopped. Saving to 'recording.wav'...\n";
    if (recorder.saveToFile("recording.wav")) {
        std::cout << "Recording saved successfully!\n";
    } else {
        std::cerr << "Error saving recording\n";
        return 1;
    }

    return 0;
}
