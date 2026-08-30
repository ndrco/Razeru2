#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

class ChromaFileReader final {
public:
    static constexpr std::size_t Rows = 6;
    static constexpr std::size_t Columns = 22;
    static constexpr std::size_t ColorCount = Rows * Columns;

    struct Frame {
        float durationSeconds{ 0.033f };
        std::array<int, ColorCount> colors{};
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
