#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Reader for Razer Chroma SDK 9x7 mouse animations. Razer Viper exposes only
// the logo LED, so retaining that cell is sufficient for direct HID playback.
class ChromaMouseFileReader final {
public:
    static constexpr std::size_t Rows = 9;
    static constexpr std::size_t Columns = 7;
    static constexpr std::size_t ColorCount = Rows * Columns;
    static constexpr std::size_t LogoRow = 7;
    static constexpr std::size_t LogoColumn = 3;
    static constexpr std::size_t LogoIndex = LogoRow * Columns + LogoColumn;

    struct Frame {
        float durationSeconds{ 0.033f };
        int logoColor{};
    };

    bool Load(const std::string& path);
    void Clear();

    bool IsLoaded() const noexcept;
    std::size_t FrameCount() const noexcept;
    const Frame* GetFrame(std::size_t index) const noexcept;
    const std::string& Path() const noexcept;
    const std::string& LastError() const noexcept;

private:
    std::string _path;
    std::string _lastError;
    std::vector<Frame> _frames;
};
