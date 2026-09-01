// Description: animation, keyboard-layout and overlay engine for Razeru 2.
// Hardware output is provided by a direct user-mode HID transport and does not
// require Razer Chroma software or services.


#pragma once

#ifndef CHROMA_PLAYING_H
#define CHROMA_PLAYING_H


#include <chrono>
#include <optional>
#include <mutex>
#include "ChromaFileReader.h"
#include "ChromaMouseFileReader.h"
#include "RazerHidDevice.h"
#include "Razer/ChromaAnimationAPI.h"


using namespace ChromaSDK;

#define STILL_PRESSING -1 // Indicate of active key pressing

class ChromaPlaying {


public:
    
    HWND hwnd{ nullptr };
    UINT messageCh{ 0 };

    //System language
    LANGID g_currentLang = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
    // Chroma keyboard animations use a device-independent 6x22 matrix.
    static const int SIZEKEYBOARD{ static_cast<int>(ChromaFileReader::ColorCount) };
    
    // Key animation data
    struct ChromaKeyEffect {
        EFFECT_TYPE name{ EFFECT_TYPE::CHROMA_NONE };
        int primaryColor{ 0 };
        int frameDuration{ 33 };
        int framesOfKeyAnimation{ 33 };
    } chromaKeyEffect;

    // Backlight keys data
    struct ChromaBacklightEffect {
        bool backLightOn{ true }; // Flag for enabling key backlighting
        std::vector<int> colorsKeyboard{}; // Array for storing keys requiring backlighting
        EChromaSDKSceneBlend blend{ EChromaSDKSceneBlend::SB_None };
        EChromaSDKSceneMode mode{ EChromaSDKSceneMode::SM_Replace };
    } chromaBacklightEffect;
    
    // Keyboard animation data
    struct ChromaKeyboardEffect {
        LANGID keyboardLayout{ 0x00000409 }; // 0x0409 (en-US) Windows keyboard layout code
        int frameDuration{ 33 }; // Animation duration, mc for 1 frame
        int frames{ 100 }; // Numbers of animation frames
        int frameIndex{ 0 }; // Current animation frame
        int speed{ 1 }; // Specifies how many frames to play at a time during manual playback
        std::string keyboardAnimation{ "" }; //Animation name
        std::string mouseAnimation{ "" };
        std::string mousePadAnimation{ "" };
        std::string chromaLinkAnimation{ "" };
        std::string headSetAnimation{ "" };
        std::string keyPadAnimation{ "" };
    };
    
    std::vector<ChromaKeyboardEffect> chromaKeyboardEffect; // Array of keyboard animation parameters

    // Structure for the data exchange with DLL 
    struct ConfigData {
        LANGID g_currentLang = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
        bool devicesAnimation{ true }; //Flag for enabling Razer device animation (except keyboard)
        std::vector<DEVICE_INFO_TYPE> connectedDevices{}; // List of connected Razer devices (except keyboard)
        std::vector<ChromaKeyboardEffect> keyboardEffect{}; // Array of keyboard animation parameters
        ChromaKeyEffect keyEffect{ EFFECT_TYPE::CHROMA_NONE , 0, 33, 33}; // Parameters for key press animations
        ChromaBacklightEffect backlightEffect{ true, {0}, EChromaSDKSceneBlend::SB_None, EChromaSDKSceneMode::SM_Replace}; // // Backlight keys data
        // Defaul constructor
        ConfigData() = default;
        // Copy constructor
        ConfigData(const ConfigData& other)
            : g_currentLang(other.g_currentLang),
            devicesAnimation(other.devicesAnimation),
            connectedDevices(other.connectedDevices),
            keyboardEffect(other.keyboardEffect),
            keyEffect(other.keyEffect),
            backlightEffect(other.backlightEffect) {}
        // Assignment operator to support deep copying
        ConfigData& operator=(const ConfigData& other) {
            if (this == &other) {
                return *this; // Protection against self-assignment
            }
            g_currentLang = other.g_currentLang;
            devicesAnimation = other.devicesAnimation;
            connectedDevices = other.connectedDevices;
            keyboardEffect = other.keyboardEffect;
            keyEffect = other.keyEffect;
            backlightEffect = other.backlightEffect;
            return *this;
        }
        // Destructor
        ~ConfigData() = default;
    };

    typedef void (*SetNotifyWindowFunc)(HWND hwnd, UINT messageCh);
    typedef bool (*ShowConfigFunc)();
    typedef bool (*LoadConfigFunc)();
    typedef bool (*SaveConfigFunc)();
    typedef ConfigData* (*GetConfigFunc)();
    typedef void (*SetConfigFunc)(ConfigData*);
    typedef void (*SignalResponseReadyFunc)();
    
    // Open the keyboard's standard Windows HID lighting interface.
    int InitChroma(HWND hwndMain, UINT changedMessage);

    // Restore a hardware lighting effect and release the HID handle.
    int Cleanup();
    
    // Launch settings dialog using RzruUI.dll
    void StartSettings();
    
    // Load the configuration using RzruUI.dll
    bool LoadConfig();

    // Save the current configuration using RzruUI.dll
    bool SaveConfig();

    // Update the configuration using RzruUI.dll
    bool UpdateConfig();

    // Play a single frame with effects of the keys animation and static backlight
    void PlayingFrameKeyboard(const std::optional<std::reference_wrapper<std::vector<std::pair<int, int>>>> keyStateMap,
        const bool nextframe);

    // Automatically play the keyboard animation
    void PlayingAutoKeyboard();

    // Stop the automatic keyboard animation
    void StopAutoKeyboard();

    // Start or switch the direct-HID animation for supported non-keyboard devices.
    void PlayingAutoDevices();

    // Launch editor of .chroma files
    int LaunchOpenEditor();

    //Return true if the keys bachlighting is on
    bool IsColorKeyboardOn();

    //Return frame duration for the key animation, ms
    const int& GetKeyFrameDuration();

    //Return the number of frames for the key animation
    const int& GetFramesOFKeyAnimation();

    //Return frame duration for the keybord animation, ms
    const int& GetAnimationFrameDuration();

    //Return `true` if key animation active
    bool IsKeyAnimationOn();
   
    //Set active keyboard effect index
    void SetActiveSceneEffect(const LANGID& activeLayout);
    
    //Get active keyboard effect index
    const ChromaKeyboardEffect* GetActiveSceneEffect() const;

    // Increment the frame index for the active scene effect
    void IncrementFrameIndex();

    // Prepare configurate data for DLL
    ConfigData GetConfig();

    // Update configure data from DLL
    void SetConfig(const ConfigData& configData);

    // Load default configuration settings
    void LoadDefaultConfig();

    // Recalculate animation frame parameters for all keyboard effects
    void SetFramesParameters();


    private:

    int _activeSceneEffectIndex{ -1 }; // Active keyboard animation index
    std::vector<int> _tempColorsKeyboard; // Temporary array for processing keyboard effects
    RazerHidDevice _keyboard;
    RazerHidDevice _mouse{ RazerHidDevice::Profile::Viper };
    ChromaFileReader _activeAnimation;
    ChromaMouseFileReader _activeMouseAnimation;
    std::mutex _animationMutex;
    std::mutex _mouseReconnectMutex;
    std::chrono::steady_clock::time_point _nextMouseOpenAttempt{};
    bool _devicesAnimation{ true }; //Flag for enabling Razer device animation (except keyboard)
    std::vector<DEVICE_INFO_TYPE> _connectedDevices; // List of connected Razer devices (except keyboard)


    // Private methods

    // Write the color of the key (row, column) to the keyboard array
    void _SetKeyColor(std::vector<int>& colors, const int row, const int column, const int color);

    // Retrieve the color of the key (row, column) from the keyboard array
    int _GetKeyColor(const std::vector<int>& colors, const int row, const int column);

    // Fill the vector `vec` with zero elements in the quantity of `size`
    void _AddZeroElementsToVector(std::vector<int>& vec);

    // Animation restart
    void _RestartAnimation(const std::string& animationName, const bool loop);

    // Display a keyboard frame through the direct HID transport.
    bool _SetKeyboardFrame(const std::vector<int>& colors);

    // Restore the keyboard's firmware spectrum effect.
    void _ReleaseKeyboardEffect();

    // Display the logo cell from the active 9x7 mouse animation.
    bool _SetMouseFrame(const ChromaKeyboardEffect& effect);

    // Reopen an optional mouse after a transient HID failure, at most once
    // every two seconds while it is unavailable.
    bool _EnsureMouseOpen();

    // Blend the current frame `tempColors` of the active animation `effect`
    // with the array of keys requiring backlighting (`colors`)
    // Result in `tempColors`
    void _BlendAnimation(const std::vector<int>& colors, std::vector<int>& tempColors);

    // Blend the current frame `tempColors` of the active animation `effect` with the array of pressed keys
    // Result in `tempColors`
    void _KeyAnimation(const std::reference_wrapper<std::vector<std::pair<int, int>>> keyStateMap,
        const ChromaKeyEffect& activeKeyEffect, std::vector<int>& tempColors);

    // Wave effect for _KeyAnimation
    void _Wave(std::vector<int>& tempColors, const int row, const int col, const int frame, const int frames, const int color);

    // Linearly interpolate the key color in the frame
    void _SetLerpKeyColorToVector(std::vector<int>& tempColors, const int row, const int col, const double t, int color);

    // Invert the RGB color
    int _InvertColor(const int color);

    // Calculate the maximum of two colors (per channel)
    int _MaxColor(const int color1, const int color2);

    // Calculate the minimum of two colors (per channel)
    int _MinColor(const int color1, const int color2);
    
    // Average from two colors (per channel)
    int _AverageColor(const int color1, const int color2);
    
    // Multiply two colors (per channel) with scaling and clamping to 255
    int _MultiplyColor(const int color1, const int color2);
    
    // Add two colors (per channel) with clamping
    int _AddColor(const int color1, const int color2);
    
    // Subtract the second color from the first (per channel) with clamping
    int _SubtractColor(const int color1, const int color2);
};

#endif // CHROMA_PLAYING_H


