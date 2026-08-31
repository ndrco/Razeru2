#include "../ChromaFileReader.h"
#include "../ChromaMouseFileReader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    const fs::path animationDirectory = argc > 1 ? fs::path(argv[1]) : fs::path("Animations");
    if (!fs::is_directory(animationDirectory)) {
        std::cerr << "Animation directory does not exist: " << animationDirectory << '\n';
        return 1;
    }

    std::size_t keyboardsLoaded = 0;
    std::size_t miceLoaded = 0;
    for (const fs::directory_entry& entry : fs::directory_iterator(animationDirectory)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".chroma") {
            continue;
        }
        const std::string filename = entry.path().filename().string();
        if (filename.size() >= 16 &&
            filename.rfind("_Keyboard.chroma") == filename.size() - 16) {
            ChromaFileReader reader;
            if (!reader.Load(entry.path().string())) {
                std::cerr << reader.LastError() << '\n';
                return 2;
            }
            if (reader.FrameCount() == 0 || reader.GetFrame(0) == nullptr) {
                std::cerr << "Loaded keyboard animation contains no frames: "
                          << entry.path() << '\n';
                return 3;
            }
            ++keyboardsLoaded;
        }
        else if (filename.size() >= 13 &&
            filename.rfind("_Mouse.chroma") == filename.size() - 13) {
            ChromaMouseFileReader reader;
            if (!reader.Load(entry.path().string())) {
                std::cerr << reader.LastError() << '\n';
                return 4;
            }
            if (reader.FrameCount() == 0 || reader.GetFrame(0) == nullptr) {
                std::cerr << "Loaded mouse animation contains no frames: "
                          << entry.path() << '\n';
                return 5;
            }
            ++miceLoaded;
        }
    }

    if (keyboardsLoaded == 0 || miceLoaded == 0) {
        std::cerr << "No keyboard or mouse animations were tested.\n";
        return 6;
    }

    const fs::path malformed = fs::temp_directory_path() / "razeru2-truncated.chroma";
    {
        std::ofstream output(malformed, std::ios::binary | std::ios::trunc);
        output.write("bad", 3);
    }
    ChromaFileReader rejected;
    if (rejected.Load(malformed.string())) {
        fs::remove(malformed);
        std::cerr << "Truncated animation was accepted.\n";
        return 7;
    }
    ChromaMouseFileReader mouseRejected;
    if (mouseRejected.Load(malformed.string())) {
        fs::remove(malformed);
        std::cerr << "Truncated mouse animation was accepted.\n";
        return 8;
    }
    fs::remove(malformed);

    std::cout << "Loaded and validated " << keyboardsLoaded << " keyboard and "
              << miceLoaded << " mouse animations; malformed input was rejected.\n";
    return 0;
}
