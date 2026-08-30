#include "RazerHidDevice.h"

#include <SetupAPI.h>
#include <hidsdi.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cwctype>
#include <memory>
#include <sstream>
#include <thread>

namespace {
constexpr USHORT TargetUsagePage = 0x000C;
constexpr USHORT TargetUsage = 0x0001;
constexpr wchar_t TargetInterfaceToken[] = L"&mi_03";
constexpr std::chrono::milliseconds ReportDelay{ 1 };
constexpr std::chrono::milliseconds ResponseDelay{ 5 };

std::wstring Lowercase(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), towlower);
    return value;
}
}

RazerHidDevice::~RazerHidDevice() {
    Close();
}

bool RazerHidDevice::Open() {
    std::lock_guard<std::mutex> lock(_mutex);
    return OpenUnlocked();
}

void RazerHidDevice::Close() {
    std::lock_guard<std::mutex> lock(_mutex);
    CloseUnlocked();
}

bool RazerHidDevice::IsOpen() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _handle != INVALID_HANDLE_VALUE;
}

bool RazerHidDevice::OpenUnlocked() {
    if (_handle != INVALID_HANDLE_VALUE) {
        return true;
    }

    GUID hidGuid{};
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO deviceInfo = SetupDiGetClassDevsW(
        &hidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfo == INVALID_HANDLE_VALUE) {
        SetLastErrorUnlocked(L"SetupDiGetClassDevsW", GetLastError());
        return false;
    }

    bool found = false;
    for (DWORD index = 0;; ++index) {
        SP_DEVICE_INTERFACE_DATA interfaceData{};
        interfaceData.cbSize = sizeof(interfaceData);
        if (!SetupDiEnumDeviceInterfaces(deviceInfo, nullptr, &hidGuid, index, &interfaceData)) {
            if (GetLastError() != ERROR_NO_MORE_ITEMS) {
                SetLastErrorUnlocked(L"SetupDiEnumDeviceInterfaces", GetLastError());
            }
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

        const std::wstring path = detail->DevicePath;
        const std::wstring lowerPath = Lowercase(path);
        if (lowerPath.find(TargetInterfaceToken) == std::wstring::npos) {
            continue;
        }

        HANDLE candidate = CreateFileW(
            path.c_str(), GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (candidate == INVALID_HANDLE_VALUE) {
            continue;
        }

        HIDD_ATTRIBUTES attributes{};
        attributes.Size = sizeof(attributes);
        PHIDP_PREPARSED_DATA preparsed = nullptr;
        HIDP_CAPS caps{};
        const bool attributesOk = HidD_GetAttributes(candidate, &attributes) != FALSE;
        const bool preparsedOk = HidD_GetPreparsedData(candidate, &preparsed) != FALSE;
        const bool capsOk = preparsedOk && HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS;
        if (preparsed != nullptr) {
            HidD_FreePreparsedData(preparsed);
        }

        if (attributesOk && capsOk &&
            attributes.VendorID == VendorId &&
            attributes.ProductID == HuntsmanV2TklProductId &&
            caps.UsagePage == TargetUsagePage && caps.Usage == TargetUsage) {
            _handle = candidate;
            _devicePath = path;
            _lastError.clear();
            found = true;
            break;
        }
        CloseHandle(candidate);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
    if (!found && _lastError.empty()) {
        SetLastErrorUnlocked(L"Huntsman V2 TKL HID interface 3 was not found or could not be opened");
    }
    return found;
}

void RazerHidDevice::CloseUnlocked() {
    if (_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(_handle);
        _handle = INVALID_HANDLE_VALUE;
    }
    _devicePath.clear();
}

RazerHidDevice::FeatureReport RazerHidDevice::MakeReport(
    std::uint8_t commandClass, std::uint8_t commandId, std::uint8_t dataSize) const {
    FeatureReport report{};
    report.reportId = 0;
    report.transactionId = _transactionId;
    report.dataSize = dataSize;
    report.commandClass = commandClass;
    report.commandId = commandId;
    return report;
}

std::uint8_t RazerHidDevice::CalculateCrc(const FeatureReport& report) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&report);
    std::uint8_t crc = 0;
    // The first byte is the HID report ID.  Razer's checksum covers payload
    // bytes 2..87, which are indices 3..88 in the Windows feature buffer.
    for (std::size_t index = 3; index < 89; ++index) {
        crc ^= bytes[index];
    }
    return crc;
}

bool RazerHidDevice::SendUnlocked(FeatureReport& report) {
    if (!OpenUnlocked()) {
        return false;
    }
    report.crc = CalculateCrc(report);
    if (!HidD_SetFeature(_handle, &report, sizeof(report))) {
        const DWORD error = GetLastError();
        CloseUnlocked();
        SetLastErrorUnlocked(L"HidD_SetFeature", error);
        return false;
    }
    return true;
}

bool RazerHidDevice::ReceiveUnlocked(FeatureReport& report) {
    if (!OpenUnlocked()) {
        return false;
    }
    report = {};
    report.reportId = 0;
    if (!HidD_GetFeature(_handle, &report, sizeof(report))) {
        const DWORD error = GetLastError();
        CloseUnlocked();
        SetLastErrorUnlocked(L"HidD_GetFeature", error);
        return false;
    }
    return true;
}

bool RazerHidDevice::ExchangeUnlocked(FeatureReport& request, FeatureReport& response) {
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (!SendUnlocked(request)) {
            return false;
        }
        std::this_thread::sleep_for(ResponseDelay);
        if (!ReceiveUnlocked(response)) {
            return false;
        }
        if (response.status == 0x02 && response.transactionId == request.transactionId) {
            if (response.commandClass != request.commandClass ||
                response.commandId != request.commandId ||
                response.remainingPackets != 0 ||
                response.crc != CalculateCrc(response)) {
                SetLastErrorUnlocked(L"Razer response validation failed");
                return false;
            }
            return true;
        }
        if (response.status != 0x01) {
            std::wostringstream message;
            message << L"Razer command failed with status 0x" << std::hex
                    << static_cast<unsigned int>(response.status);
            SetLastErrorUnlocked(message.str());
            return false;
        }
        std::this_thread::sleep_for(ResponseDelay);
    }
    SetLastErrorUnlocked(L"Razer device remained busy after three attempts");
    return false;
}

bool RazerHidDevice::QueryFirmware(std::string& version) {
    std::lock_guard<std::mutex> lock(_mutex);
    FeatureReport request = MakeReport(0x00, 0x81, 0x02);
    FeatureReport response{};
    if (!ExchangeUnlocked(request, response)) {
        // OpenRazer uses 0x1F for this model while OpenRGB uses 0x3F.  Probe
        // the alternative only for a read-only command and keep the working ID.
        _transactionId = 0x1F;
        request = MakeReport(0x00, 0x81, 0x02);
        if (!ExchangeUnlocked(request, response)) {
            _transactionId = 0x3F;
            return false;
        }
    }
    version = std::to_string(response.arguments[0]) + "." +
        std::to_string(response.arguments[1]);
    _lastError.clear();
    return true;
}

bool RazerHidDevice::SetStaticColor(
    std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    std::lock_guard<std::mutex> lock(_mutex);
    FeatureReport report = MakeReport(0x0F, 0x02, 0x09);
    report.arguments[0] = 0x00; // Do not save to device storage.
    report.arguments[1] = 0x05; // Backlight LED.
    report.arguments[2] = 0x01; // Static effect.
    report.arguments[5] = 0x01;
    report.arguments[6] = red;
    report.arguments[7] = green;
    report.arguments[8] = blue;
    return SendUnlocked(report);
}

bool RazerHidDevice::SetSpectrumEffect() {
    std::lock_guard<std::mutex> lock(_mutex);
    FeatureReport report = MakeReport(0x0F, 0x02, 0x06);
    report.arguments[0] = 0x00; // Do not save to device storage.
    report.arguments[1] = 0x05; // Backlight LED.
    report.arguments[2] = 0x03; // Spectrum effect.
    return SendUnlocked(report);
}

bool RazerHidDevice::SendLogicalFrame(const std::vector<int>& colors) {
    if (colors.size() < LogicalColorCount) {
        std::lock_guard<std::mutex> lock(_mutex);
        SetLastErrorUnlocked(L"Logical keyboard frame contains fewer than 132 colors");
        return false;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    for (std::size_t row = 0; row < DeviceRows; ++row) {
        FeatureReport report = MakeReport(
            0x0F, 0x03, static_cast<std::uint8_t>(5 + DeviceColumns * 3));
        report.arguments[2] = static_cast<std::uint8_t>(row);
        report.arguments[3] = 0;
        report.arguments[4] = static_cast<std::uint8_t>(DeviceColumns - 1);

        for (std::size_t column = 0; column < DeviceColumns; ++column) {
            // Chroma's device-independent 6x22 keyboard grid reserves column
            // zero.  The Huntsman V2 TKL hardware matrix starts at Esc/`/Tab/
            // Caps/Shift/Ctrl, so logical columns 1..17 map to device 0..16.
            const std::size_t logicalColumn = column + LogicalColumnOffset;
            const int color = colors[row * LogicalColumns + logicalColumn];
            const std::size_t output = 5 + column * 3;
            // Windows COLORREF is 0x00BBGGRR.
            report.arguments[output] = static_cast<std::uint8_t>(color & 0xFF);
            report.arguments[output + 1] = static_cast<std::uint8_t>((color >> 8) & 0xFF);
            report.arguments[output + 2] = static_cast<std::uint8_t>((color >> 16) & 0xFF);
        }

        if (!SendUnlocked(report)) {
            return false;
        }
        std::this_thread::sleep_for(ReportDelay);
    }

    FeatureReport customMode = MakeReport(0x0F, 0x02, 0x0C);
    customMode.arguments[2] = 0x08;
    if (!SendUnlocked(customMode)) {
        return false;
    }
    _lastError.clear();
    return true;
}

std::wstring RazerHidDevice::DevicePath() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _devicePath;
}

std::wstring RazerHidDevice::LastError() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _lastError;
}

void RazerHidDevice::SetLastErrorUnlocked(const wchar_t* operation, DWORD errorCode) {
    wchar_t* systemMessage = nullptr;
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, 0, reinterpret_cast<wchar_t*>(&systemMessage), 0, nullptr);

    std::wostringstream message;
    message << operation << L" failed (" << errorCode << L")";
    if (length != 0 && systemMessage != nullptr) {
        message << L": " << systemMessage;
    }
    if (systemMessage != nullptr) {
        LocalFree(systemMessage);
    }
    _lastError = message.str();
}

void RazerHidDevice::SetLastErrorUnlocked(const std::wstring& message) {
    _lastError = message;
}
