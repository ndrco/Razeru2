#include "../ChromaFileReader.h"

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

    std::size_t loaded = 0;
    for (const fs::directory_entry& entry : fs::directory_iterator(animationDirectory)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".chroma") {
            continue;
        }
        const std::string filename = entry.path().filename().string();
        if (filename.size() < 16 ||
            filename.rfind("_Keyboard.chroma") != filename.size() - 16) {
            continue;
        }

        ChromaFileReader reader;
        if (!reader.Load(entry.path().string())) {
            std::cerr << reader.LastError() << '\n';
            return 2;
        }
        if (reader.FrameCount() == 0 || reader.GetFrame(0) == nullptr) {
            std::cerr << "Loaded animation contains no frames: " << entry.path() << '\n';
            return 3;
        }
        ++loaded;
    }

    if (loaded == 0) {
        std::cerr << "No keyboard animations were tested.\n";
        return 4;
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
        return 5;
    }
    fs::remove(malformed);

    std::cout << "Loaded and validated " << loaded
              << " keyboard animations; malformed input was rejected.\n";
    return 0;
}
