#pragma once

#include <Windows.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

// Direct user-mode HID transport for the supported Razer devices. The normal
// input collections remain on the Microsoft HID stack; no Razer SDK service
// or vendor kernel driver is used here.
class RazerHidDevice final {
public:
    enum class Profile {
        HuntsmanV2Tkl,
        Viper,
    };

    static constexpr std::uint16_t VendorId = 0x1532;
    static constexpr std::uint16_t HuntsmanV2TklProductId = 0x026B;
    static constexpr std::uint16_t ViperProductId = 0x0078;
    static constexpr std::size_t LogicalRows = 6;
    static constexpr std::size_t LogicalColumns = 22;
    static constexpr std::size_t DeviceRows = 6;
    static constexpr std::size_t DeviceColumns = 17;
    static constexpr std::size_t LogicalColumnOffset = 1;
    static constexpr std::size_t LogicalColorCount = LogicalRows * LogicalColumns;

    explicit RazerHidDevice(Profile profile = Profile::HuntsmanV2Tkl);
    ~RazerHidDevice();

    RazerHidDevice(const RazerHidDevice&) = delete;
    RazerHidDevice& operator=(const RazerHidDevice&) = delete;

    bool Open();
    void Close();
    bool IsOpen() const;

    bool QueryFirmware(std::string& version);
    // Temporary brightness only; querying/opening a device does not change it.
    bool QueryBrightness(std::uint8_t& brightness);
    bool SetBrightness(std::uint8_t brightness);
    void InvalidateBrightness();
    bool SetStaticColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue);
    bool SetSpectrumEffect();
    bool SendLogicalFrame(const std::vector<int>& colors);

    std::wstring DevicePath() const;
    std::wstring LastError() const;

private:
#pragma pack(push, 1)
    struct FeatureReport {
        std::uint8_t reportId{};
        std::uint8_t status{};
        std::uint8_t transactionId{};
        std::uint16_t remainingPackets{};
        std::uint8_t protocolType{};
        std::uint8_t dataSize{};
        std::uint8_t commandClass{};
        std::uint8_t commandId{};
        std::array<std::uint8_t, 80> arguments{};
        std::uint8_t crc{};
        std::uint8_t reserved{};
    };
#pragma pack(pop)

    static_assert(sizeof(FeatureReport) == 91,
        "Windows HID report must contain a report ID plus the 90-byte Razer payload");

    bool OpenUnlocked();
    void CloseUnlocked();
    bool SendUnlocked(FeatureReport& report);
    bool ReceiveUnlocked(FeatureReport& report);
    bool ExchangeUnlocked(FeatureReport& request, FeatureReport& response);
    bool QueryBrightnessUnlocked(std::uint8_t& brightness);
    bool SetBrightnessUnlocked(std::uint8_t brightness);
    bool EnsureBrightnessUnlocked();
    FeatureReport MakeReport(std::uint8_t commandClass, std::uint8_t commandId,
        std::uint8_t dataSize) const;
    static std::uint8_t CalculateCrc(const FeatureReport& report);
    void SetLastErrorUnlocked(const wchar_t* operation, DWORD errorCode);
    void SetLastErrorUnlocked(const std::wstring& message);

    mutable std::mutex _mutex;
    HANDLE _handle{ INVALID_HANDLE_VALUE };
    std::wstring _devicePath;
    std::wstring _lastError;
    Profile _profile;
    std::uint8_t _transactionId{ 0x3F };
    bool _brightnessInitialized{ false };
    std::chrono::steady_clock::time_point _nextBrightnessCheck{};
};
