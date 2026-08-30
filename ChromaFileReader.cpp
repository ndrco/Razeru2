#include "ChromaFileReader.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>

namespace {
constexpr std::int32_t SupportedVersion = 1;
constexpr std::uint8_t DeviceType2D = 1;
constexpr std::uint8_t DeviceKeyboard = 0;
constexpr std::uint8_t DeviceKeyboardExtended = 3;
constexpr std::size_t ExtendedRows = 8;
constexpr std::size_t ExtendedColumns = 24;
constexpr std::size_t HeaderSize = sizeof(std::int32_t) + sizeof(std::uint8_t) * 2 +
    sizeof(std::uint32_t);
constexpr std::uint32_t MaximumFrameCount = 100000;

template <typename T>
bool ReadValue(std::ifstream& input, T& value) {
    return static_cast<bool>(input.read(reinterpret_cast<char*>(&value), sizeof(value)));
}
}

bool ChromaFileReader::Load(const std::string& path) {
    Clear();

    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        _lastError = "Cannot open Chroma animation: " + path;
        return false;
    }

    const std::streamoff streamSize = input.tellg();
    if (streamSize < 0 || static_cast<std::uint64_t>(streamSize) < HeaderSize) {
        _lastError = "Chroma animation is truncated: " + path;
        return false;
    }
    input.seekg(0, std::ios::beg);

    std::int32_t version = 0;
    std::uint8_t deviceType = 0;
    std::uint8_t device = 0;
    std::uint32_t frameCount = 0;
    if (!ReadValue(input, version) || !ReadValue(input, deviceType) ||
        !ReadValue(input, device) || !ReadValue(input, frameCount)) {
        _lastError = "Cannot read Chroma animation header: " + path;
        return false;
    }

    if (version != SupportedVersion) {
        _lastError = "Unsupported Chroma animation version: " + std::to_string(version);
        return false;
    }
    if (deviceType != DeviceType2D ||
        (device != DeviceKeyboard && device != DeviceKeyboardExtended)) {
        _lastError = "The animation is not a supported keyboard animation: " + path;
        return false;
    }
    if (frameCount == 0 || frameCount > MaximumFrameCount) {
        _lastError = "Invalid Chroma animation frame count: " + std::to_string(frameCount);
        return false;
    }

    const std::size_t storedColorCount = device == DeviceKeyboard
        ? ColorCount
        : ExtendedRows * ExtendedColumns;
    const std::uint64_t frameSize = sizeof(float) +
        static_cast<std::uint64_t>(storedColorCount) * sizeof(std::int32_t);
    const std::uint64_t expectedSize = HeaderSize +
        static_cast<std::uint64_t>(frameCount) * frameSize;
    if (expectedSize != static_cast<std::uint64_t>(streamSize)) {
        _lastError = "Chroma animation size does not match its frame count: " + path;
        return false;
    }

    std::vector<Frame> frames;
    frames.reserve(frameCount);
    for (std::uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        Frame frame;
        if (!ReadValue(input, frame.durationSeconds)) {
            _lastError = "Cannot read Chroma frame duration: " + path;
            return false;
        }
        if (!std::isfinite(frame.durationSeconds) || !(frame.durationSeconds > 0.0f)) {
            frame.durationSeconds = 0.033f;
        }
        frame.durationSeconds = (std::max)(0.001f, frame.durationSeconds);

        // This deliberately matches CChromaEditorLibrary::PluginGetFrameName:
        // Razeru 1 requested 132 colors, so extended 8x24 animations were read
        // in row-major order and truncated after the first 132 entries.
        for (std::size_t colorIndex = 0; colorIndex < storedColorCount; ++colorIndex) {
            std::int32_t storedColor = 0;
            if (!ReadValue(input, storedColor)) {
                _lastError = "Cannot read Chroma frame colors: " + path;
                return false;
            }
            if (colorIndex < frame.colors.size()) {
                frame.colors[colorIndex] = storedColor & 0x00FFFFFF;
            }
        }
        frames.push_back(std::move(frame));
    }

    _path = path;
    _frames = std::move(frames);
    _lastError.clear();
    return true;
}

void ChromaFileReader::Clear() {
    _path.clear();
    _lastError.clear();
    _frames.clear();
}

bool ChromaFileReader::IsLoaded() const noexcept {
    return !_frames.empty();
}

std::size_t ChromaFileReader::FrameCount() const noexcept {
    return _frames.size();
}

const ChromaFileReader::Frame* ChromaFileReader::GetFrame(std::size_t index) const noexcept {
    if (index >= _frames.size()) {
        return nullptr;
    }
    return &_frames[index];
}

const std::string& ChromaFileReader::Path() const noexcept {
    return _path;
}

const std::string& ChromaFileReader::LastError() const noexcept {
    return _lastError;
}
