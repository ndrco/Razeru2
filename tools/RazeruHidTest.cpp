#include "../RazerHidDevice.h"

#include <Windows.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
void PrintError(const RazerHidDevice& device) {
    std::wcerr << L"Error: " << device.LastError() << L'\n';
}

bool ParseColor(const std::wstring& text, int& color) {
    if (text.size() != 6) {
        return false;
    }
    try {
        std::size_t consumed = 0;
        color = std::stoi(text, &consumed, 16);
        return consumed == text.size() && color >= 0 && color <= 0xFFFFFF;
    }
    catch (...) {
        return false;
    }
}
}

int wmain(int argc, wchar_t** argv) {
    RazerHidDevice device;
    if (!device.Open()) {
        PrintError(device);
        return 2;
    }

    std::wcout << L"Opened: " << device.DevicePath() << L'\n';
    std::string firmware;
    if (device.QueryFirmware(firmware)) {
        std::cout << "Firmware: " << firmware << '\n';
    }
    else {
        PrintError(device);
        return 3;
    }

    if (argc == 1 || _wcsicmp(argv[1], L"info") == 0) {
        return 0;
    }

    if (_wcsicmp(argv[1], L"spectrum") == 0) {
        if (!device.SetSpectrumEffect()) {
            PrintError(device);
            return 4;
        }
        std::wcout << L"Spectrum effect sent.\n";
        return 0;
    }

    if ((_wcsicmp(argv[1], L"static") == 0 || _wcsicmp(argv[1], L"frame") == 0) && argc == 3) {
        int rgb = 0;
        if (!ParseColor(argv[2], rgb)) {
            std::wcerr << L"Expected a six-digit RRGGBB color.\n";
            return 5;
        }
        const auto red = static_cast<std::uint8_t>((rgb >> 16) & 0xFF);
        const auto green = static_cast<std::uint8_t>((rgb >> 8) & 0xFF);
        const auto blue = static_cast<std::uint8_t>(rgb & 0xFF);

        bool sent = false;
        if (_wcsicmp(argv[1], L"static") == 0) {
            sent = device.SetStaticColor(red, green, blue);
        }
        else {
            // Store as Windows COLORREF, not RRGGBB.
            const int colorRef = red | (green << 8) | (blue << 16);
            std::vector<int> frame(RazerHidDevice::LogicalColorCount, colorRef);
            sent = device.SendLogicalFrame(frame);
        }
        if (!sent) {
            PrintError(device);
            return 6;
        }
        std::wcout << L"Lighting command sent.\n";
        return 0;
    }

    std::wcerr << L"Usage: RazeruHidTest [info|spectrum|static RRGGBB|frame RRGGBB]\n";
    return 1;
}
