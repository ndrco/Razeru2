#include "ChromaMouseFileReader.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <utility>

namespace {
constexpr std::int32_t SupportedVersion = 1;
constexpr std::uint8_t DeviceType2D = 1;
constexpr std::uint8_t DeviceMouse = 2;
constexpr std::size_t HeaderSize = sizeof(std::int32_t) + sizeof(std::uint8_t) * 2 +
    sizeof(std::uint32_t);
constexpr std::uint32_t MaximumFrameCount = 100000;

template <typename T>
bool ReadValue(std::ifstream& input, T& value) {
    return static_cast<bool>(input.read(reinterpret_cast<char*>(&value), sizeof(value)));
}
}

bool ChromaMouseFileReader::Load(const std::string& path) {
    Clear();

    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) {
        _lastError = "Cannot open Chroma mouse animation: " + path;
        return false;
    }

    const std::streamoff streamSize = input.tellg();
    if (streamSize < 0 || static_cast<std::uint64_t>(streamSize) < HeaderSize) {
        _lastError = "Chroma mouse animation is truncated: " + path;
        return false;
    }
    input.seekg(0, std::ios::beg);

    std::int32_t version = 0;
    std::uint8_t deviceType = 0;
    std::uint8_t device = 0;
    std::uint32_t frameCount = 0;
    if (!ReadValue(input, version) || !ReadValue(input, deviceType) ||
        !ReadValue(input, device) || !ReadValue(input, frameCount)) {
        _lastError = "Cannot read Chroma mouse animation header: " + path;
        return false;
    }
    if (version != SupportedVersion || deviceType != DeviceType2D || device != DeviceMouse) {
        _lastError = "The animation is not a supported 9x7 mouse animation: " + path;
        return false;
    }
    if (frameCount == 0 || frameCount > MaximumFrameCount) {
        _lastError = "Invalid Chroma mouse animation frame count: " +
            std::to_string(frameCount);
        return false;
    }

    const std::uint64_t frameSize = sizeof(float) +
        static_cast<std::uint64_t>(ColorCount) * sizeof(std::int32_t);
    const std::uint64_t expectedSize = HeaderSize +
        static_cast<std::uint64_t>(frameCount) * frameSize;
    if (expectedSize != static_cast<std::uint64_t>(streamSize)) {
        _lastError = "Chroma mouse animation size does not match its frame count: " + path;
        return false;
    }

    std::vector<Frame> frames;
    frames.reserve(frameCount);
    for (std::uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        Frame frame;
        if (!ReadValue(input, frame.durationSeconds)) {
            _lastError = "Cannot read Chroma mouse frame duration: " + path;
            return false;
        }
        if (!std::isfinite(frame.durationSeconds) || !(frame.durationSeconds > 0.0f)) {
            frame.durationSeconds = 0.033f;
        }
        frame.durationSeconds = (std::max)(0.001f, frame.durationSeconds);

        for (std::size_t colorIndex = 0; colorIndex < ColorCount; ++colorIndex) {
            std::int32_t storedColor = 0;
            if (!ReadValue(input, storedColor)) {
                _lastError = "Cannot read Chroma mouse frame colors: " + path;
                return false;
            }
            if (colorIndex == LogoIndex) {
                frame.logoColor = storedColor & 0x00FFFFFF;
            }
        }
        frames.push_back(frame);
    }

    _path = path;
    _frames = std::move(frames);
    _lastError.clear();
    return true;
}

void ChromaMouseFileReader::Clear() {
    _path.clear();
    _lastError.clear();
    _frames.clear();
}

bool ChromaMouseFileReader::IsLoaded() const noexcept {
    return !_frames.empty();
}

std::size_t ChromaMouseFileReader::FrameCount() const noexcept {
    return _frames.size();
}

const ChromaMouseFileReader::Frame* ChromaMouseFileReader::GetFrame(
    std::size_t index) const noexcept {
    return index < _frames.size() ? &_frames[index] : nullptr;
}

const std::string& ChromaMouseFileReader::Path() const noexcept {
    return _path;
}

const std::string& ChromaMouseFileReader::LastError() const noexcept {
    return _lastError;
}
