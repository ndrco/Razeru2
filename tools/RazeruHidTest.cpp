#include "../RazerHidDevice.h"

#include <Windows.h>
#include <hidsdi.h>
#include <setupapi.h>

#include <algorithm>
#include <cwctype>
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

bool ParseHexWord(const std::wstring& text, std::uint16_t& value) {
    try {
        std::size_t consumed = 0;
        const unsigned long parsed = std::stoul(text, &consumed, 16);
        if (consumed != text.size() || parsed > 0xFFFF) {
            return false;
        }
        value = static_cast<std::uint16_t>(parsed);
        return true;
    }
    catch (...) {
        return false;
    }
}

int ListHidCollections(std::uint16_t productId) {
    GUID hidGuid{};
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO deviceInfo = SetupDiGetClassDevsW(
        &hidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfo == INVALID_HANDLE_VALUE) {
        std::wcerr << L"SetupDiGetClassDevsW failed: " << GetLastError() << L'\n';
        return 2;
    }

    int matches = 0;
    for (DWORD index = 0;; ++index) {
        SP_DEVICE_INTERFACE_DATA interfaceData{};
        interfaceData.cbSize = sizeof(interfaceData);
        if (!SetupDiEnumDeviceInterfaces(deviceInfo, nullptr, &hidGuid, index, &interfaceData)) {
            break;
        }

        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(
            deviceInfo, &interfaceData, nullptr, 0, &requiredSize, nullptr);
        if (requiredSize < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) {
            continue;
        }
        std::vector<std::uint8_t> storage(requiredSize);
        auto* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if (!SetupDiGetDeviceInterfaceDetailW(
            deviceInfo, &interfaceData, detail, requiredSize, nullptr, nullptr)) {
            continue;
        }

        HANDLE handle = CreateFileW(
            detail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr);
        if (handle == INVALID_HANDLE_VALUE) {
            continue;
        }

        HIDD_ATTRIBUTES attributes{};
        attributes.Size = sizeof(attributes);
        PHIDP_PREPARSED_DATA preparsed = nullptr;
        HIDP_CAPS caps{};
        const bool attributesOk = HidD_GetAttributes(handle, &attributes) != FALSE;
        const bool preparsedOk = HidD_GetPreparsedData(handle, &preparsed) != FALSE;
        const bool capsOk = preparsedOk && HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS;
        if (preparsed != nullptr) {
            HidD_FreePreparsedData(preparsed);
        }

        if (attributesOk && capsOk &&
            attributes.VendorID == RazerHidDevice::VendorId &&
            attributes.ProductID == productId) {
            ++matches;
            std::wcout << L"Path: " << detail->DevicePath << L'\n'
                << L"  Usage page/usage: 0x" << std::hex << std::setw(4)
                << std::setfill(L'0') << caps.UsagePage << L" / 0x"
                << std::setw(4) << caps.Usage << std::dec << L'\n'
                << L"  Reports input/output/feature: " << caps.InputReportByteLength
                << L" / " << caps.OutputReportByteLength
                << L" / " << caps.FeatureReportByteLength << L'\n';

            HANDLE readWriteHandle = CreateFileW(
                detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
            if (readWriteHandle == INVALID_HANDLE_VALUE) {
                std::wcout << L"  Read/write open error: " << GetLastError() << L'\n';
            }
            else {
                std::wcout << L"  Read/write open: OK\n";
                CloseHandle(readWriteHandle);
            }
        }
        CloseHandle(handle);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
    if (matches == 0) {
        std::wcerr << L"No matching Razer HID collections found.\n";
        return 3;
    }
    return 0;
}
}

int wmain(int argc, wchar_t** argv) {
    if (argc == 3 && _wcsicmp(argv[1], L"list") == 0) {
        std::uint16_t productId = 0;
        if (!ParseHexWord(argv[2], productId)) {
            std::wcerr << L"Expected a hexadecimal USB product ID.\n";
            return 1;
        }
        return ListHidCollections(productId);
    }

    std::wstring command = argc > 1 ? argv[1] : L"info";
    std::transform(command.begin(), command.end(), command.begin(), towlower);
    const bool mouseCommand = command.rfind(L"mouse-", 0) == 0;
    if (mouseCommand) {
        command.erase(0, 6);
    }

    RazerHidDevice device(mouseCommand
        ? RazerHidDevice::Profile::Viper
        : RazerHidDevice::Profile::HuntsmanV2Tkl);
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

    if (command == L"info") {
        return 0;
    }

    if (command == L"spectrum") {
        if (!device.SetSpectrumEffect()) {
            PrintError(device);
            return 4;
        }
        std::wcout << L"Spectrum effect sent.\n";
        return 0;
    }

    if ((command == L"static" || command == L"frame") && argc == 3) {
        int rgb = 0;
        if (!ParseColor(argv[2], rgb)) {
            std::wcerr << L"Expected a six-digit RRGGBB color.\n";
            return 5;
        }
        const auto red = static_cast<std::uint8_t>((rgb >> 16) & 0xFF);
        const auto green = static_cast<std::uint8_t>((rgb >> 8) & 0xFF);
        const auto blue = static_cast<std::uint8_t>(rgb & 0xFF);

        bool sent = false;
        if (command == L"static") {
            sent = device.SetStaticColor(red, green, blue);
        }
        else if (!mouseCommand) {
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

    std::wcerr << L"Usage: RazeruHidTest [info|spectrum|static RRGGBB|frame RRGGBB|"
        L"mouse-info|mouse-spectrum|mouse-static RRGGBB|list PPPP]\n";
    return 1;
}
