#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <vector>
#include <cstring>
#include <thread>
#include <atomic>
#include <algorithm>
#include <string>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iomanip>
#include <sstream>

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

// Helper function to format duration
std::string formatDuration(float seconds) {
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << mins << ":" 
       << std::setfill('0') << std::setw(2) << secs;
    return ss.str();
}

int main() {
    // Initialize GLFW and OpenGL
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // GL 3.2 + GLSL 150 for macOS
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(400, 200, "Audio Recorder", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize audio recorder
    AudioRecorder recorder;
    bool isRecording = false;
    std::chrono::steady_clock::time_point startTime;
    float recordingDuration = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a window that takes up the entire GLFW window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Audio Recorder", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoCollapse);

        // Update recording duration
        if (isRecording) {
            auto now = std::chrono::steady_clock::now();
            recordingDuration = std::chrono::duration_cast<std::chrono::milliseconds>
                (now - startTime).count() / 1000.0f;
        }

        // Center the content
        float windowWidth = ImGui::GetWindowSize().x;
        float windowHeight = ImGui::GetWindowSize().y;
        
        // Recording duration display
        std::string durationText = "Duration: " + formatDuration(recordingDuration);
        float textWidth = ImGui::CalcTextSize(durationText.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::SetCursorPosY(windowHeight * 0.3f);
        ImGui::Text("%s", durationText.c_str());

        // Record/Stop button
        float buttonWidth = 120.0f;
        float buttonHeight = 40.0f;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        ImGui::SetCursorPosY(windowHeight * 0.5f);

        if (!isRecording) {
            if (ImGui::Button("Record", ImVec2(buttonWidth, buttonHeight))) {
                if (recorder.startRecording()) {
                    isRecording = true;
                    startTime = std::chrono::steady_clock::now();
                    recordingDuration = 0.0f;
                }
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Stop", ImVec2(buttonWidth, buttonHeight))) {
                recorder.stopRecording();
                isRecording = false;
                recorder.saveToFile("recording.wav");
            }
            ImGui::PopStyleColor();
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
