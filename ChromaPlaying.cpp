#pragma once

#include "ChromaPlaying.h"
#include <tchar.h>
#include <cmath>



// API initialization
int ChromaPlaying::InitChroma(HWND hwndMain, UINT changedMessage) {
    
    hwnd = hwndMain;
    messageCh = changedMessage;

    // Attempt to initialize the Chroma Animation API
    if (ChromaAnimationAPI::InitAPI() != RZRESULT_SUCCESS) {
        // Return error if the API initialization fails
        return 1;
    }
    // Set application metadata
    ChromaSDK::APPINFOTYPE appInfo = {};
    _tcscpy_s(appInfo.Title, 256, _T("Razeru"));
    _tcscpy_s(appInfo.Description, 1024, _T("Razer Chroma Language indicator"));
    _tcscpy_s(appInfo.Author.Name, 256, _T("NDR company"));
    _tcscpy_s(appInfo.Author.Contact, 256, _T("ndrco@yahoo.com"));
    // Supported devices and application category:
    //appInfo.SupportedDevice = 
    //    0x01 | // Keyboards
    //    0x02 | // Mice
    //    0x04 | // Headset
    //    0x08 | // Mousepads
    //    0x10 | // Keypads
    //    0x20   // ChromaLink devices
    appInfo.SupportedDevice = (0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20);
    //    0x01 | // Utility. (To specifiy this is an utility application)
    //    0x02   // Game. (To specifiy this is a game);
    appInfo.Category = 1;
    
    // Initialize the Chroma SDK with the provided metadata
    RZRESULT result = ChromaAnimationAPI::InitSDK(&appInfo);
    if (result) {
        return result;
        // RZRESULT_DLL_NOT_FOUND 6023: Chroma DLL is not found
        // RZRESULT_DLL_INVALID_SIGNATURE 6033: Chroma DLL has an invalid signature!
        // Other: Failed to initialize Chroma!
    }
    Sleep(100); //Allow time for initialization

    // Check for connected devices
    DEVICE_INFO_TYPE deviceInfo = { DEVICE_INFO_TYPE::DEVICE_ALL };
    result = ChromaAnimationAPI::CoreIsConnected(deviceInfo);

    if (result == RZRESULT_SUCCESS) {
#if _DEBUG
        TCHAR message[50] = { 0 };
        wsprintf(message, TEXT("RAZER Devices %d\n"),
            deviceInfo.Connected);
        OutputDebugString(message);
#endif //_DEBUG
        
        // Check if a keyboard is connected
        DEVICE_INFO_TYPE keyboardInfo = { DEVICE_INFO_TYPE::DEVICE_KEYBOARD };
        result = ChromaAnimationAPI::CoreIsConnected(keyboardInfo);
        if (result != RZRESULT_SUCCESS || keyboardInfo.Connected <= 0)
            // No keyboard detected
            return 101;

        // Store connected devices
        _connectedDevices.clear();
        if (deviceInfo.Connected > 0) {
            std::vector<enum DEVICE_INFO_TYPE::DeviceType> deviceTypes = {
                DEVICE_INFO_TYPE::DEVICE_MOUSE,
                DEVICE_INFO_TYPE::DEVICE_HEADSET,
                DEVICE_INFO_TYPE::DEVICE_MOUSEPAD,
                DEVICE_INFO_TYPE::DEVICE_KEYPAD,
                DEVICE_INFO_TYPE::DEVICE_CHROMALINK
            };
            // Check for each supported device type
            for (const auto& deviceType : deviceTypes) {
                DEVICE_INFO_TYPE currentDeviceInfo = { deviceType };
                result = ChromaAnimationAPI::CoreIsConnected(currentDeviceInfo);

                if (result == RZRESULT_SUCCESS && currentDeviceInfo.Connected > 0) {
                    _connectedDevices.push_back(currentDeviceInfo);
                }
            }
        }
    }
    else {
        return 100; // No Chroma devices connected
    }
    return 0; // Initialization successful
}


// ChromaSDK cleanup
int ChromaPlaying::Cleanup() {

    RZRESULT result = 0;
    if (ChromaAnimationAPI::GetIsInitializedAPI()) {
        if (ChromaAnimationAPI::IsInitialized()) {
            _ReleaseKeyboardEffect();
            ChromaAnimationAPI::StopAll();
            ChromaAnimationAPI::CloseAll();
            result = ChromaAnimationAPI::Uninit();
            ChromaAnimationAPI::UninitAPI();
        }
    }
    return result;
}


// Launch settings dialog using RzruUI.dll
void ChromaPlaying::StartSettings() {

    HMODULE hModule = LoadLibrary(L"RzruUI.dll");

    if (hModule) {
        auto Initialize = (SetNotifyWindowFunc)GetProcAddress(hModule, "InitializeDLL");

        if (Initialize) {
            Initialize(hwnd, messageCh);  // Pass the window handle and message ID
        }
        auto SetConfiguration = (SetConfigFunc)GetProcAddress(hModule, "SetConfiguration");
        if (SetConfiguration) {
            ConfigData configData = ChromaPlaying::GetConfig();
            SetConfiguration(&configData); // Pass current configuration
        }
        auto ShowConfigWindow = reinterpret_cast<ShowConfigFunc>(GetProcAddress(hModule, "ShowConfigWindow"));
        if (ShowConfigWindow) {
            ShowConfigWindow(); // Show configuration window
        }
        else {
            MessageBox(nullptr, L"Failed to load the configuration module (RzruUI.dll)", L"Error", MB_ICONERROR);
        }
        FreeLibrary(hModule); // Free the DLL
    }
}


// Load the configuration using RzruUI.dll
bool ChromaPlaying::LoadConfig() {

    HMODULE hModule = LoadLibrary(L"RzruUI.dll");

    bool success = false;
    
    if (hModule) {

        auto Initialize = (SetNotifyWindowFunc)GetProcAddress(hModule, "InitializeDLL");
        if (Initialize) {
            Initialize(hwnd, messageCh);  // Pass the window handle and message ID
        }
        auto LoadConfiguration = reinterpret_cast<LoadConfigFunc>(GetProcAddress(hModule, "LoadConfiguration"));
        if (LoadConfiguration) {
            success = LoadConfiguration(); // Attempt to load configuration
        }
        if (success) {
            auto GetConfiguration = reinterpret_cast<GetConfigFunc>(GetProcAddress(hModule, "GetConfiguration"));
            if (GetConfiguration) {
                ConfigData* configPointer = GetConfiguration(); // Get configuration pointer 
                if (configPointer != nullptr) {
                    ConfigData configData = *configPointer; // Make a local copy of the configuration
                    ChromaPlaying::SetConfig(configData);  // Set the loaded configuration
                }
                else success = false;
            }
        }
        FreeLibrary(hModule); // Free the DLL
    }
    return success; // Return whether the configuration was successfully loaded
}
 

// Save the current configuration using RzruUI.dll
bool ChromaPlaying::SaveConfig() {

    HMODULE hModule = LoadLibrary(L"RzruUI.dll");

    bool success = false;

    if (hModule) {

        auto Initialize = (SetNotifyWindowFunc)GetProcAddress(hModule, "InitializeDLL");
        if (Initialize) {
            Initialize(hwnd, messageCh);  // Pass the window handle and message ID
        }
        auto SetConfiguration = (SetConfigFunc)GetProcAddress(hModule, "SetConfiguration");
        if (SetConfiguration) {
            ConfigData configData = ChromaPlaying::GetConfig();
            SetConfiguration(&configData); //Pass the configuration to the DLL
        }
        auto SaveConfiguration = reinterpret_cast<SaveConfigFunc>(GetProcAddress(hModule, "SaveConfiguration"));
        if (SaveConfiguration) {
            success = SaveConfiguration(); // Attempt to save the configuration
        }
        FreeLibrary(hModule); // Free the DLL
    }
    if (!success) {
        MessageBox(nullptr, L"Failed to load the configuration module (RzruUI.dll)", L"Error", MB_ICONERROR);
    }
    return success; // Return whether the configuration was successfully saved
}


// Update the configuration using RzruUI.dll
bool  ChromaPlaying::UpdateConfig() {
    
    HMODULE hModule = GetModuleHandle(L"RzruUI.dll");

    bool success = false;

    if (hModule) {
        // Получаем конфигурацию
        auto GetConfiguration = reinterpret_cast<GetConfigFunc>(GetProcAddress(hModule, "GetConfiguration"));
        if (GetConfiguration) {
            ConfigData* configPointer = GetConfiguration(); // Retrieve the updated configuration
            if (configPointer != nullptr) {
                ConfigData configData = *configPointer; // Make a copy of the configuration
                ChromaPlaying::SetConfig(configData); // Set the updated configuration
                success = true;  // Configuration successfully updated
                auto SignalResponseReady = reinterpret_cast<SignalResponseReadyFunc>(GetProcAddress(hModule, "SignalResponseReady"));
                if (SignalResponseReady) 
                    SignalResponseReady(); // Notify that the update is complete
            }
        }
    }
    return success; // Return whether the configuration was successfully updated
}


// Play a single frame with effects of the keys animation and static backlight
void ChromaPlaying::PlayingFrameKeyboard(const std::optional<std::reference_wrapper<std::vector<std::pair<int, int>>>> keyStateMap, const bool nextframe) {
  
    if (nextframe) {
        ChromaPlaying::IncrementFrameIndex(); // Advance the frame index
    }
    auto effect = ChromaPlaying::GetActiveSceneEffect(); // Get the active scene effect
    if (effect) {

        float duration;
        // Retrieve the current animation frame
        if (ChromaAnimationAPI::GetFrameName(_memoryAnimationName, effect->frameIndex,
            &duration, &_tempColorsKeyboard[0], SIZEKEYBOARD, 0, 0) > 0) {

            if (IsColorKeyboardOn()) {
                // Blend backlight colors with the animation frame
                ChromaPlaying::_BlendAnimation(chromaBacklightEffect.colorsKeyboard, _tempColorsKeyboard);
            }
            if ((chromaKeyEffect.name != EFFECT_TYPE::CHROMA_NONE) && keyStateMap.has_value()) {
                // Apply key-specific effects to the animation frame
                ChromaPlaying::_KeyAnimation(keyStateMap.value(), chromaKeyEffect, _tempColorsKeyboard);
            }
            // Send the updated frame using the low-level effect path. Recent
            // Chroma App runtimes accept SetEffectCustom2D but may not retain
            // its keyboard output, while CoreSetEffect is rendered correctly.
            _SetKeyboardFrame(_tempColorsKeyboard);
        }
    }
}


// Automatically play the keyboard animation
void ChromaPlaying::PlayingAutoKeyboard() {
    auto effect = ChromaPlaying::GetActiveSceneEffect(); // Get the active scene effect
    if (effect) {
#if _DEBUG
        std::string message = "Animation " + effect->keyboardAnimation + ": started\n";
        OutputDebugStringA(message.c_str());
#endif //_DEBUG
        // Restart the active animation in loop mode
        ChromaPlaying::_RestartAnimation(effect->keyboardAnimation, true);
    }
}


// Stop the automatic keyboard animation
void ChromaPlaying::StopAutoKeyboard() {
    auto effect = ChromaPlaying::GetActiveSceneEffect();
    if (effect) {
        ChromaAnimationAPI::CloseAnimationName(effect->keyboardAnimation.c_str());
    }
    _ReleaseKeyboardEffect();
}


// Display a keyboard frame through a retained low-level SDK effect.
bool ChromaPlaying::_SetKeyboardFrame(const std::vector<int>& colors) {
    if (colors.size() < SIZEKEYBOARD) {
        return false;
    }

    Keyboard::CUSTOM_EFFECT_TYPE keyboardEffect = {};
    for (RZSIZE row = 0; row < Keyboard::MAX_ROW; ++row) {
        for (RZSIZE column = 0; column < Keyboard::MAX_COLUMN; ++column) {
            const size_t index = static_cast<size_t>(row) * Keyboard::MAX_COLUMN + column;
            keyboardEffect.Color[row][column] = static_cast<COLORREF>(colors[index]);
        }
    }

    RZEFFECTID nextEffectId = {};
    std::lock_guard<std::mutex> lock(_keyboardEffectMutex);

    RZRESULT result = ChromaAnimationAPI::CoreCreateKeyboardEffect(
        Keyboard::CHROMA_CUSTOM,
        &keyboardEffect,
        &nextEffectId);
    if (result != RZRESULT_SUCCESS) {
        return false;
    }

    result = ChromaAnimationAPI::CoreSetEffect(nextEffectId);
    if (result != RZRESULT_SUCCESS) {
        ChromaAnimationAPI::CoreDeleteEffect(nextEffectId);
        return false;
    }

    if (_hasKeyboardEffect) {
        ChromaAnimationAPI::CoreDeleteEffect(_keyboardEffectId);
    }
    _keyboardEffectId = nextEffectId;
    _hasKeyboardEffect = true;
    return true;
}


// Release the retained low-level keyboard effect.
void ChromaPlaying::_ReleaseKeyboardEffect() {
    std::lock_guard<std::mutex> lock(_keyboardEffectMutex);
    if (_hasKeyboardEffect) {
        ChromaAnimationAPI::CoreDeleteEffect(_keyboardEffectId);
        _keyboardEffectId = RZEFFECTID{};
        _hasKeyboardEffect = false;
    }
}

// Automatically play animations for connected other Chroma devices 
void ChromaPlaying::PlayingAutoDevices() {
    auto effect = ChromaPlaying::GetActiveSceneEffect();  // Get the active scene effect
    if (_devicesAnimation && effect) {
        for (const auto& deviceType : _connectedDevices) {
            switch (deviceType.DeviceType) {
            case DEVICE_INFO_TYPE::DEVICE_MOUSE:
                _RestartAnimation(effect->mouseAnimation, true);
                break;
            case DEVICE_INFO_TYPE::DEVICE_HEADSET:
                _RestartAnimation(effect->headSetAnimation, true);
                break;
            case DEVICE_INFO_TYPE::DEVICE_MOUSEPAD:
                _RestartAnimation(effect->mousePadAnimation, true);
                break;
            case DEVICE_INFO_TYPE::DEVICE_KEYPAD:
                _RestartAnimation(effect->keyPadAnimation, true);
                break;
            case DEVICE_INFO_TYPE::DEVICE_CHROMALINK:
                _RestartAnimation(effect->chromaLinkAnimation, true);
                break;
            default:
                // Unsupported device type (optional: log a warning)
                break;
            }
        }
    }
}


// Launch editor of .chroma files
int ChromaPlaying::LaunchOpenEditor() {

    const char* testLayer = "Animations/Blank_Keyboard.chroma";
    // Start with a blank animation
    _RestartAnimation(testLayer, false);
    int result = ChromaAnimationAPI::OpenEditorDialog(testLayer);
    if (result) {
        return result; // Return any errors
    }
    // Wait until the editor dialog is closed
    while (ChromaAnimationAPI::IsDialogOpen()) {
        Sleep(100);
    }
    return result; // Return the result of the editor operation
}


//Return the frame duration for the key animation, ms
const int& ChromaPlaying::GetKeyFrameDuration() {
    return chromaKeyEffect.frameDuration;
}


//Return the numbers of frames for the key animation
const int& ChromaPlaying::GetFramesOFKeyAnimation() {
    return chromaKeyEffect.framesOfKeyAnimation;
}


//Return the frame duration for the keybord animation, ms
const int& ChromaPlaying::GetAnimationFrameDuration() {
    auto effect = ChromaPlaying::GetActiveSceneEffect();
    if (effect) {
        return effect->frameDuration;
    }
    else {
        return chromaKeyEffect.frameDuration; // Fallback
    }
}


//Return true if the keys animation is on
bool ChromaPlaying::IsKeyAnimationOn() {
    return (chromaKeyEffect.name != EFFECT_TYPE::CHROMA_NONE) ? true : false;
}


//Return true if the keys backlighting is on
bool ChromaPlaying::IsColorKeyboardOn() {
    return (chromaBacklightEffect.backLightOn) ? true : false;
}


//Set active keyboard effect index
void ChromaPlaying::SetActiveSceneEffect(const LANGID& activeLayout) {
    auto it = std::find_if(chromaKeyboardEffect.begin(),
        chromaKeyboardEffect.end(),
        [&activeLayout](const ChromaKeyboardEffect& keybordEffect) {
            return keybordEffect.keyboardLayout == activeLayout;
        });

    if (it != chromaKeyboardEffect.end()) {
        _activeSceneEffectIndex = static_cast<int>(std::distance(chromaKeyboardEffect.begin(), it));
        if (chromaBacklightEffect.backLightOn || chromaKeyEffect.name != EFFECT_TYPE::CHROMA_NONE) {
            auto effect = ChromaPlaying::GetActiveSceneEffect();
            if (effect) {
                ChromaAnimationAPI::CopyAnimationName(effect->keyboardAnimation.c_str(), _memoryAnimationName);
            }
        }
    }
    else {
        _activeSceneEffectIndex = -1; // No matching effect found
    }
}


//Get active keyboard effect index
const ChromaPlaying::ChromaKeyboardEffect* ChromaPlaying::GetActiveSceneEffect() const {
    
    if (_activeSceneEffectIndex < 0 ||
        _activeSceneEffectIndex >= static_cast<int>(chromaKeyboardEffect.size()) ||
        chromaKeyboardEffect[_activeSceneEffectIndex].keyboardAnimation.empty()) {
        return nullptr;
    }
        return &chromaKeyboardEffect[_activeSceneEffectIndex];
}


// Increment the frame index for the active scene effect
void ChromaPlaying::IncrementFrameIndex() {
    if (_activeSceneEffectIndex >= 0 && _activeSceneEffectIndex < static_cast<int>(chromaKeyboardEffect.size())) {
        ChromaKeyboardEffect& effect = chromaKeyboardEffect[_activeSceneEffectIndex];
        if (effect.frames > 0) {
            const int nextFrame = (effect.frameIndex + effect.speed) % effect.frames;
            effect.frameIndex = (nextFrame + effect.frames) % effect.frames;
        }
    }
}


// Prepare configurate data for DLL
ChromaPlaying::ConfigData ChromaPlaying::GetConfig() {
  
    ConfigData configData;
    configData.g_currentLang = g_currentLang; // System language
    configData.devicesAnimation = _devicesAnimation; // Device animation flag
    configData.connectedDevices = _connectedDevices; // List of connected devices
    configData.keyboardEffect = chromaKeyboardEffect; // Keyboard effects
    configData.keyEffect = chromaKeyEffect; // Key-specific effect
    configData.backlightEffect = chromaBacklightEffect; // Backlight settings
    return configData;
}

// Update configure data from DLL
void ChromaPlaying::SetConfig(const ConfigData& configData) {

    g_currentLang = configData.g_currentLang; // Update language
    _devicesAnimation = configData.devicesAnimation; // Update device animation flag
   
    chromaKeyEffect = ChromaKeyEffect(); 
    chromaKeyEffect = configData.keyEffect; // Update key-specific effect
    chromaKeyEffect.frameDuration = (std::max)(10, (std::min)(999, chromaKeyEffect.frameDuration));
    chromaKeyEffect.framesOfKeyAnimation = (std::max)(1, (std::min)(999, chromaKeyEffect.framesOfKeyAnimation));
    
    chromaBacklightEffect = ChromaBacklightEffect();
    chromaBacklightEffect = configData.backlightEffect; // Update backlight settings
    
    chromaKeyboardEffect.clear();
    chromaKeyboardEffect = configData.keyboardEffect; // Update keyboard effects
       
    ChromaPlaying::SetFramesParameters(); // Recalculate frame parameters

    _activeSceneEffectIndex = -1; // Reset the active scene effect index
    _tempColorsKeyboard.clear(); // Clear temporary color data

    // Prepare color buffers if effects are active
    if (IsColorKeyboardOn() || (chromaKeyEffect.name != EFFECT_TYPE::CHROMA_NONE)) {
        ChromaPlaying::_AddZeroElementsToVector(_tempColorsKeyboard);

        if (IsColorKeyboardOn()) {
            chromaBacklightEffect.colorsKeyboard.resize(SIZEKEYBOARD, 0);
        }
    }
}


// Load default configuration settings
void ChromaPlaying::LoadDefaultConfig() {

    g_currentLang = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT); // Default system language
    _devicesAnimation = true; // Enable device animations
    chromaKeyEffect = ChromaKeyEffect(); // Reset key effects
    chromaBacklightEffect = ChromaBacklightEffect(); // Reset backlight settings
    chromaKeyboardEffect.clear(); // Clear existing keyboard effects
    _activeSceneEffectIndex = -1; // Reset active effect index
    _tempColorsKeyboard.clear(); // Clear temporary color buffer

    // Configure default key animation effect
    chromaKeyEffect.name = EFFECT_TYPE::CHROMA_REACTIVE;
    chromaKeyEffect.frameDuration = 33; // 33 ms per frame
    chromaKeyEffect.framesOfKeyAnimation = 33; // Total 33 frames
    chromaKeyEffect.primaryColor = 65535; // Yellow color
    
    // Configure default backlight effect
    chromaBacklightEffect.backLightOn = true; // Enable backlighting
    chromaBacklightEffect.blend = EChromaSDKSceneBlend::SB_None;
    chromaBacklightEffect.mode = EChromaSDKSceneMode::SM_Replace;

    if (IsColorKeyboardOn() || (chromaKeyEffect.name != EFFECT_TYPE::CHROMA_NONE)) {
        ChromaPlaying::_AddZeroElementsToVector(_tempColorsKeyboard);

        if (IsColorKeyboardOn()) {
            chromaBacklightEffect.colorsKeyboard = {
                0, 255, 0, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 65280, 255, 255, 255, 0, 0, 0, 0,
                0, 8388736, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8388736, 255, 255, 255, 0, 0, 0, 0,
                0, 8388736, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8388736, 255, 255, 255, 0, 0, 0, 0,
                0, 8388736, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8388736, 0, 0, 0, 0, 0, 0, 0,
                0, 8388736, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8388736, 0, 16711680, 0, 0, 0, 0, 0,
                0, 8388736, 8388736, 8388736, 0, 0, 0, 8388736, 0, 0, 0, 8388736, 8388736, 8388736, 8388736, 16711680, 16711680, 16711680, 0, 0, 0, 0
            };
        }
    }
    // Configure default keyboard effects
    ChromaPlaying::ChromaKeyboardEffect effect;

    effect = ChromaKeyboardEffect();
    effect.keyboardAnimation = "Animations/Green_Gradient_Keyboard.chroma";
    effect.keyboardLayout = 0x00000419; // Russian layout
    effect.speed = 1;
    effect.mouseAnimation = "Animations/Green_Gradient_Mouse.chroma";
    effect.mousePadAnimation = "Animations/Green_Gradient_Mousepad.chroma";
    effect.chromaLinkAnimation = "Animations/Green_Gradient_ChromaLink.chroma";
    effect.headSetAnimation = "Animations/Green_Gradient_Headset.chroma";
    effect.keyPadAnimation = "Animations/Green_Gradient_Keypad.chroma";
    chromaKeyboardEffect.push_back(effect);

    effect = ChromaKeyboardEffect();
    effect.keyboardAnimation = "Animations/Pink_Gradient_Keyboard.chroma";
    effect.keyboardLayout = 0x00000409; // English layout
    effect.speed = 1;
    effect.mouseAnimation = "Animations/Pink_Gradient_Mouse.chroma";
    effect.mousePadAnimation = "Animations/Pink_Gradient_Mousepad.chroma";
    effect.chromaLinkAnimation = "Animations/Pink_Gradient_ChromaLink.chroma";
    effect.headSetAnimation = "Animations/Pink_Gradient_Headset.chroma";
    effect.keyPadAnimation = "Animations/Pink_Gradient_Keypad.chroma";
    chromaKeyboardEffect.push_back(effect);
    
    ChromaPlaying::SetFramesParameters(); // Recalculate frame parameters
}


// Recalculate animation frame parameters for all keyboard effects
void ChromaPlaying::SetFramesParameters() {
    for (auto& effect : chromaKeyboardEffect) {
        const int frameDuration = static_cast<int>(1000 * ChromaAnimationAPI::GetFrameDurationName(effect.keyboardAnimation.c_str(), 0));
        const int frameCount = ChromaAnimationAPI::GetFrameCountName(effect.keyboardAnimation.c_str());
        effect.frameDuration = (std::max)(1, frameDuration);
        effect.frames = (std::max)(1, frameCount);
        effect.frameIndex = (std::max)(0, effect.frameIndex % effect.frames);
    }
}





// Set color for RZKEY
void ChromaPlaying::_SetKeyColor(std::vector<int>& colors, const int row, const int column, const int color) {
    colors[Keyboard::MAX_COLUMN * row + column] = color;
}


// Get color for RZKEY
int ChromaPlaying::_GetKeyColor(const std::vector<int>& colors, const int row, const int column) {
    return colors[Keyboard::MAX_COLUMN * row + column];
}


// Reset a color buffer to all zeros
void ChromaPlaying::_AddZeroElementsToVector(std::vector<int>& vec) {
    vec.clear();
    for (int index = 0; index < SIZEKEYBOARD; ++index) {
        vec.push_back(0);
    }
}


// Restart an animation by closing and replaying it
void ChromaPlaying::_RestartAnimation(const std::string& animationName, const bool loop) {
    ChromaAnimationAPI::CloseAnimationName(animationName.c_str());
    if (ChromaAnimationAPI::GetAnimation(animationName.c_str()) > 0) {
        ChromaAnimationAPI::PlayAnimationName(animationName.c_str(), loop);
    }
}


// Blend two color buffers (backlight and animation frame) based on blending and mode settings
void ChromaPlaying::_BlendAnimation(const std::vector<int>& colors, std::vector<int>& tempColors) {
    for (int i = 0; i < SIZEKEYBOARD; ++i)
    {
        int color1 = colors[i]; // Backlight color
        if (!color1) continue; // Skip if the color is empty
        int tempColor = tempColors[i]; //// Source color (animation frame)

        // Apply blending mode
        int color2;
        switch (chromaBacklightEffect.blend) {
        case EChromaSDKSceneBlend::SB_None:
            color2 = color1; //No blending
            break;
        case EChromaSDKSceneBlend::SB_Invert:
            color2 = ChromaPlaying::_InvertColor(color1); // Inverted color
            break;
        case EChromaSDKSceneBlend::SB_Lerp: // Linear interpolation
        default:
            color2 = ChromaAnimationAPI::LerpColor(color1, tempColor, 0.5);
            break;
        }

        // Apply scene mode
        switch (chromaBacklightEffect.mode) {
        case EChromaSDKSceneMode::SM_Max:
            tempColors[i] = ChromaPlaying::_MaxColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Min:
            tempColors[i] = ChromaPlaying::_MinColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Average:
            tempColors[i] = ChromaPlaying::_AverageColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Multiply:
            tempColors[i] = ChromaPlaying::_MultiplyColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Add:
            tempColors[i] = ChromaPlaying::_AddColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Subtract:
            tempColors[i] = ChromaPlaying::_SubtractColor(color2, tempColor);
            break;
        case EChromaSDKSceneMode::SM_Replace:
        default:
            if (color2 != 0) {
                tempColors[i] = color1;
            }
            break;
        }
    }
}


// Apply key-specific animation effects to the frame
void ChromaPlaying::_KeyAnimation(const std::reference_wrapper<std::vector<std::pair<int, int>>> keyStateMap,
    const ChromaKeyEffect& activeKeyEffect, std::vector<int>& tempColors) {
    
    for (auto& pair : keyStateMap.get()) {
        int row = HIBYTE(pair.first); // Extract row from key code
        int column = LOBYTE(pair.first); // Extract column from key code

# if _DEBUG
       // std::wstring debugMessage = L"Key pressed row, column: " +
       //     std::to_wstring(row) + L", " + std::to_wstring(column) + L"\n";
       // OutputDebugString(debugMessage.c_str());
#endif //_DEBUG

        if (pair.second == STILL_PRESSING) {
            // Set color for keys that are still being pressed
            ChromaPlaying::_SetKeyColor(tempColors, row, column, activeKeyEffect.primaryColor);
        }
        else {
            // Apply effect based on the key's animation type
            switch (activeKeyEffect.name) {
            case EFFECT_TYPE::CHROMA_REACTIVE: {
                double t = static_cast<double> (pair.second) / activeKeyEffect.framesOfKeyAnimation;
                ChromaPlaying::_SetLerpKeyColorToVector(tempColors, row, column, t, activeKeyEffect.primaryColor);
                break;
            }
            case EFFECT_TYPE::CHROMA_WAVE:
                ChromaPlaying::_Wave(tempColors, row, column, pair.second, activeKeyEffect.framesOfKeyAnimation, activeKeyEffect.primaryColor);
                break;
            default:
                break;
            }
        }
    }
}


// Apply a wave effect centered at the specified key
void ChromaPlaying::_Wave(std::vector<int>& tempColors, const int row, const int col, const int frame, const int frames, const int color) {
    // Wave parameters
    const double ALPHA = 0.1;   // Exponential decay coefficient
    const double OMEGA = 0.14; // Frequency (adjustable)
    const double K = 1.0;      // Wave number (related to wavelength)

    const int MAX_RADIUS = 10; // Maximum propagation radius
    const double initialAmplitude = 1;

    for (int r = 0; r < Keyboard::MAX_ROW; ++r) {
        for (int c = 0; c < Keyboard::MAX_COLUMN; ++c) {
            // Calculate Manhattan distance
            int distance = abs(r - row) + abs(c - col);

            // Skip points outside the maximum radius
            if (distance > MAX_RADIUS) continue;

            // Calculate wave amplitude
            double amplitude = 0.0;
            if (distance == 0) {
                amplitude = initialAmplitude * cos(OMEGA * (frames - frame));
            }
            else {
                double wavePhase = OMEGA * (frames - frame) - K * distance;
                double attenuation = exp(-ALPHA * distance);
                amplitude = (initialAmplitude / distance) * cos(wavePhase) * attenuation;
            }
            // Normalize amplitude to [0, 1]
            double normalizedAmplitude = max(0.0, min(1.0, amplitude));
            ChromaPlaying::_SetLerpKeyColorToVector(tempColors, r, c, normalizedAmplitude, color);
        }
    }
}


// Linearly interpolate the key color in the frame
void ChromaPlaying::_SetLerpKeyColorToVector(std::vector<int>& tempColors, const int row, const int col, const double t, int color) {
    int effectKeyColor = ChromaPlaying::_GetKeyColor(tempColors, row, col);
    int currentColor = ChromaAnimationAPI::LerpColor(effectKeyColor, color, static_cast<float>(t));
    ChromaPlaying::_SetKeyColor(tempColors, row, col, currentColor);
}


// Invert the RGB color
int ChromaPlaying::_InvertColor(const int color) {
    int red = 255 - (color & 0xFF);
    int green = 255 - ((color >> 8) & 0xFF);
    int blue = 255 - ((color >> 16) & 0xFF);

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}


// Calculate the maximum of two colors (per channel)
int ChromaPlaying::_MaxColor(const int color1, const int color2) {
    int redColor1 = color1 & 0xFF;
    int greenColor1 = (color1 >> 8) & 0xFF;
    int blueColor1 = (color1 >> 16) & 0xFF;

    int redColor2 = color2 & 0xFF;
    int greenColor2 = (color2 >> 8) & 0xFF;
    int blueColor2 = (color2 >> 16) & 0xFF;

    int red = max(redColor1, redColor2) & 0xFF;
    int green = max(greenColor1, greenColor2) & 0xFF;
    int blue = max(blueColor1, blueColor2) & 0xFF;

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}

// Calculate the minimum of two colors (per channel)
int ChromaPlaying::_MinColor(const int color1, const int color2) {
    int redColor1 = color1 & 0xFF;
    int greenColor1 = (color1 >> 8) & 0xFF;
    int blueColor1 = (color1 >> 16) & 0xFF;

    int redColor2 = color2 & 0xFF;
    int greenColor2 = (color2 >> 8) & 0xFF;
    int blueColor2 = (color2 >> 16) & 0xFF;

    int red = min(redColor1, redColor2) & 0xFF;
    int green = min(greenColor1, greenColor2) & 0xFF;
    int blue = min(blueColor1, blueColor2) & 0xFF;

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}


// Average from two colors (per channel)
int ChromaPlaying::_AverageColor(const int color1, const int color2) {
    return ChromaAnimationAPI::LerpColor(color1, color2, 0.5f);
}


// Multiply two colors (per channel) with scaling and clamping to 255
int ChromaPlaying::_MultiplyColor(const int color1, const int color2) {
    int redColor1 = color1 & 0xFF;
    int greenColor1 = (color1 >> 8) & 0xFF;
    int blueColor1 = (color1 >> 16) & 0xFF;

    int redColor2 = color2 & 0xFF;
    int greenColor2 = (color2 >> 8) & 0xFF;
    int blueColor2 = (color2 >> 16) & 0xFF;

    int red = (int)floor(255 * ((redColor1 / 255.0f) * (redColor2 / 255.0f)));
    int green = (int)floor(255 * ((greenColor1 / 255.0f) * (greenColor2 / 255.0f)));
    int blue = (int)floor(255 * ((blueColor1 / 255.0f) * (blueColor2 / 255.0f)));

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}

// Add two colors (per channel) with clamping
int ChromaPlaying::_AddColor(const int color1, const int color2) {
    int redColor1 = color1 & 0xFF;
    int greenColor1 = (color1 >> 8) & 0xFF;
    int blueColor1 = (color1 >> 16) & 0xFF;

    int redColor2 = color2 & 0xFF;
    int greenColor2 = (color2 >> 8) & 0xFF;
    int blueColor2 = (color2 >> 16) & 0xFF;

    int red = min(redColor1 + redColor2, 255) & 0xFF;
    int green = min(greenColor1 + greenColor2, 255) & 0xFF;
    int blue = min(blueColor1 + blueColor2, 255) & 0xFF;

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}


// Subtract the second color from the first (per channel) with clamping
int ChromaPlaying::_SubtractColor(const int color1, const int color2) {
    int redColor1 = color1 & 0xFF;
    int greenColor1 = (color1 >> 8) & 0xFF;
    int blueColor1 = (color1 >> 16) & 0xFF;

    int redColor2 = color2 & 0xFF;
    int greenColor2 = (color2 >> 8) & 0xFF;
    int blueColor2 = (color2 >> 16) & 0xFF;

    int red = max(redColor1 - redColor2, 0) & 0xFF;
    int green = max(greenColor1 - greenColor2, 0) & 0xFF;
    int blue = max(blueColor1 - blueColor2, 0) & 0xFF;

    return ChromaAnimationAPI::GetRGB(red, green, blue);
}



