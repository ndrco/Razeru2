/*
================================================================================
Razeru 2 Keyboard Layout Indicator and Event Handler

Description:
This program is designed to monitor and handle keyboard layout changes and key
press events and provide visual feedback through the keyboard's standard HID
interface. It manages keyboard animations, key states, and interacts
with the system tray for user control. The application also includes features
to respond to session changes (lock/unlock), configuration updates, and provides
a mechanism for launching a Chroma configuration editor.

Key Features:
- Monitors active keyboard layout and triggers corresponding visual effects.
- Handles key press events, maintaining a buffer of pressed keys with durations.
- Sends validated volatile HID lighting reports to a Huntsman V2 TKL.
- Provides system tray functionality for user interaction.
- Responds to session changes (e.g., Windows lock/unlock).
- Supports a configuration update mechanism via custom Windows messages.
- Includes debugging and error-handling mechanisms for smooth operation.

Dependencies:
- Windows API (Win32)
- Windows HID and SetupAPI
- Standard libraries: thread, mutex, condition_variable, etc.

Build Requirements:
- Microsoft Visual Studio or compatible compiler.
- Linked libraries: hid.lib, setupapi.lib, dwmapi.lib, wtsapi32.lib.

Author:
[NDR Co]

License:
[GNU General Public License 2025-2026]

================================================================================
*/


#pragma once

#include <windows.h>
#include "resource.h"
#include <wtsapi32.h>
#include <dwmapi.h>
#include <thread>
#include <chrono>
#include <vector>
#include <tchar.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "RazerKeyboardMapping.h"
#include "ChromaPlaying.h"


#define ID_TIMER 1 // Timer identifier for checking keyboard layout change
#define WM_TRAYICON (WM_USER + 1) // Message for interacting with tray icon

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "wtsapi32.lib")


UINT WM_CONFIG_CHANGED{ 0 };   // Custom message from RzruUI.dll about configuration update

HINSTANCE hInst;
NOTIFYICONDATA nid;
HMENU hMenu;

// Global variable to store the main window handle
HWND hwndMain{ NULL };

// Keyboard hook
HHOOK hHook{ NULL };

// Animation compositor and direct keyboard transport
ChromaPlaying chroma;

// Queue for key states
std::vector<std::pair<int, int>> keyStateMap; // Buffer for pressed keys vkCode -> duration
constexpr size_t MAX_SIZE{ 50 }; // Reserved buffer size

std::mutex queueMutex; // Mutex for accessing the queue
std::condition_variable queueCondition; // Condition variable for notifications about new events
   
std::thread processingThread; // Thread variable for processing key presses
std::atomic<bool> pauseProcessing{ false }; // Pause key process flag
std::atomic<bool> isProcessingThreadRunning(false); // Key process is running flag
std::atomic<bool> isHookSet{ false }; // Hook setup flag

LANGID activeLayout{ 0x000 }; // Current keyboard layout
std::atomic<bool> keysEvent{ false }; // Keyboard event flag
constexpr int LAYOUT_CHECK_POLLING{ 200 }; // Regular interval for checking the current layout, ms




// Function to output the keyboard layout name to OutputDebugString
void OutputKeyboardLayoutName(DWORD layout) {
    
    char lang[KL_NAMELENGTH];
    std::string layoutName = "";
    if (GetLocaleInfoA(layout, LOCALE_SISO639LANGNAME, lang, sizeof(lang))) {
        layoutName = lang;
    }
    std::string message = "Current keyboard layout: " + layoutName + "\n";
    OutputDebugStringA(message.c_str());
 
}


// Function to retrieve and process the current active keyboard layout
void GetActiveLayout() {
    
    HWND hwndActive = GetForegroundWindow(); // Get layout for the window's thread
    
    if (hwndActive) {
        DWORD threadId = GetWindowThreadProcessId(hwndActive, NULL);
        HKL currentHkl = GetKeyboardLayout(threadId);
        LANGID newLayout = LOWORD(reinterpret_cast<UINT_PTR>(currentHkl)); // Get layout for the window's thread
        
        if (newLayout != activeLayout) {
            activeLayout = newLayout;
            chroma.SetActiveSceneEffect(activeLayout);
            chroma.PlayingAutoDevices();
            // If all keyboard effects are disabled, start keyboard animation in automatic mode
            if (!chroma.IsKeyAnimationOn() && !chroma.IsColorKeyboardOn()) {
                chroma.PlayingAutoKeyboard();
            }
        }
    }
# if _DEBUG
   //OutputKeyboardLayoutName(activeLayout);
#endif //_DEBUG
}


// Callback function to check the active keyboard layout on a timer
void CALLBACK CheckActiveLayout(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    GetActiveLayout();
}


// Function to get the Razer key code corresponding to a given virtual key code
int GetRazerKeyCode(int vkCode) {
    
    // Retrieve the instance of RazerKeyboardMapping
    RazerKeyboardMapping* mapping = RazerKeyboardMapping::GetInstance();
    
    // Map vkCode to RZKEY
    int rzKey = mapping->GetRZKEY(vkCode);
    if (rzKey != Keyboard::RZKEY::RZKEY_INVALID) {
        return rzKey;
    }
    else {
        return 0;
    }
}


// Populate the keyboard buffer with key events
void ProcessKeyEvent(int vkCode, int duration) {
    
    RazerKeyboardMapping* mapping = RazerKeyboardMapping::GetInstance();
    int rzKey = mapping->GetRZKEY(vkCode);
    if (rzKey != Keyboard::RZKEY::RZKEY_INVALID) {
        std::lock_guard<std::mutex> lock(queueMutex);
        bool found = false;

        // Check if the key code already exists in the buffer
        for (auto& pair : keyStateMap) {
            if (pair.first == rzKey) {
                // Если код найден, обновляем срок хранения
                pair.second = duration;
                found = true;
                break;
            }
        }
        // If not found, add it to the buffer
        if (!found) {
            keyStateMap.emplace_back(rzKey, duration);
        }
        keysEvent = true;
        queueCondition.notify_one();
    }
}

// Process the keyboard buffer, updating the state of each key
void ProcessKeyStateMap(std::vector<std::pair<int, int>>& keyStateMap) {
     
    // Process events in the queue, using an iterator for safe removal
    auto it = keyStateMap.begin();
    while (it != keyStateMap.end()) {
        if (it->second == STILL_PRESSING) {
            // If the key is still being pressed, skip it
            ++it;
        }
        else {
            // Decrease the duration for released keys
            it->second -= 1;
            // If the duration reaches 0, remove the key from the buffer
            if (it->second == 0) {
                it = keyStateMap.erase(it); // Erase returns the next iterator
            }
            else {
                ++it;
            }
        }
    }
}


// Thread for processing key events
void EventProcessingThread() {

    double elapsedTimeForKeyFrame = 0.0; // Time for the current key frame
    double elapsedTimeForAnimation = 0.0; // Time for the overall animation
    auto previousTime = std::chrono::steady_clock::now();

    while (isProcessingThreadRunning.load()) {
        
        int keyFrameDuration = chroma.GetKeyFrameDuration();
        int animationFrameDuration = chroma.GetAnimationFrameDuration();
        int minTimeout = (std::min)(keyFrameDuration, animationFrameDuration);

        std::unique_lock<std::mutex> lock(queueMutex);

        // Wait for an event or exit from pause state
        queueCondition.wait_for(lock, std::chrono::milliseconds(minTimeout), [] {
            return (keysEvent || !isProcessingThreadRunning.load()) && !pauseProcessing;});

        // Check if the thread should terminate
        if (!isProcessingThreadRunning.load()) {
            break;
        }

        // Check if the thread is in a paused state
        if (pauseProcessing) {
            previousTime = std::chrono::steady_clock::now();
            continue;
        }

        lock.unlock(); // Unlock the mutex for the main work

        // Measure the elapsed time
        auto currentTimePoint = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = currentTimePoint - previousTime;
        previousTime = currentTimePoint;

        double deltaTime = elapsed.count();
        elapsedTimeForKeyFrame += deltaTime;
        elapsedTimeForAnimation += deltaTime;

        bool advanceAnimation = false;
        std::vector<std::pair<int, int>> keyStateSnapshot;
        {
            std::lock_guard<std::mutex> stateLock(queueMutex);
            // Process events in keyStateMap
            if (!keyStateMap.empty() && chroma.IsKeyAnimationOn()) {
                if (elapsedTimeForKeyFrame >= keyFrameDuration) {
                    ProcessKeyStateMap(keyStateMap);
                    elapsedTimeForKeyFrame -= keyFrameDuration;
                }
            }
            keyStateSnapshot = keyStateMap;
            keysEvent = false; // Reset the event flag
        }
        // Update overall animation
        if (elapsedTimeForAnimation >= animationFrameDuration) {
            advanceAnimation = true;
            elapsedTimeForAnimation -= animationFrameDuration;
        }
        chroma.PlayingFrameKeyboard(keyStateSnapshot, advanceAnimation);
    }
}



// Keyboard hook callback for handling key press and release events
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {

    // Buffer for storing key states 
    static bool keyStates[256] = { false };

    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyBoard->vkCode;

        // Check for key press
        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !keyStates[vkCode]) {
            keyStates[vkCode] = true;
            // Add the event to the queue
            ProcessKeyEvent(vkCode, STILL_PRESSING);
        }
        // Check for key release
        else if ((wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && keyStates[vkCode]) {
            keyStates[vkCode] = false;
            // Add the event to the queue
            ProcessKeyEvent(vkCode, chroma.GetFramesOFKeyAnimation());
        }
#if _DEBUG        
        // Exit when Escape is pressed
        if (vkCode == VK_ESCAPE && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            PostQuitMessage(0);
        }
#endif //_DEBUG
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}


// Initialize ChromaPlaying and set up the handlers
int InitChromaHandlers() {

    int err = chroma.InitChroma(hwndMain, WM_CONFIG_CHANGED);

    LPCTSTR message = TEXT("");
    switch (err) {
    case 0:
        break; // Initialization successful
    case 1:
        message = TEXT("Failed to initialize the Razeru 2 HID transport.");
        break;
    case 100:
        message = TEXT("No supported direct-HID devices are connected.");
        break;
    case 101:
        message = TEXT("Razer Huntsman V2 Tenkeyless (1532:026B) was not found. Check its USB connection.");
        break;
    case 102:
        message = TEXT("The keyboard HID interface opened, but the device did not answer the firmware probe.");
        break;
    default:
        message = TEXT("Failed to initialize direct keyboard control.");
    }
    if (err) {
        MessageBox(NULL, message, TEXT("Error"), MB_ICONERROR);
        return 1; // Initialization failed
    }
    else {
         return 0; // Initialization successful
    }
}


// Pause all activities: stop the timer, animations, and processing thread
void pauseAll() {
    
    KillTimer(hwndMain, ID_TIMER); // Stop the keyboard layout check timer
    if (isProcessingThreadRunning.load()) {
        pauseProcessing.store(true); // Pause the processing thread before releasing its effect
    }
    chroma.StopAutoKeyboard(); // Stop automatic playback and release any manual keyboard effect
    if (isHookSet.load()) {
        std::lock_guard<std::mutex> lock(queueMutex);
        keyStateMap.clear(); // Clear the key buffer
    }
}


// Start or resume all activities: set up the timer, animations, and hooks
void startAll() {
    chroma.ResumeLighting(); // Revalidate brightness before resumed output.
    activeLayout = 0x000; // Reset the active layout
    GetActiveLayout(); // Check the current keyboard layout
    if (chroma.IsKeyAnimationOn() || chroma.IsColorKeyboardOn()) {
        if (!isProcessingThreadRunning.load()) {
            isProcessingThreadRunning.store(true);
            processingThread = std::thread(EventProcessingThread); // Start the processing thread
        }
        if (chroma.IsKeyAnimationOn()) {
            if (!isHookSet.load()) {
                hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInst, 0); // Set the keyboard hook
                if (!hHook) { // Handle hook setup failure
                }
                isHookSet.store(true);
            }
        }
        else {
            if (isHookSet.load()) {
                isHookSet.store(false);
                UnhookWindowsHookEx(hHook); // Remove the hook
            }
        }
        pauseProcessing.store(false); // Resume the processing thread
        queueCondition.notify_all(); // Notify the thread to continue
    }
    else {
        if (isHookSet.load()) {
            isHookSet.store(false);
            UnhookWindowsHookEx(hHook); // Remove the hook
        }
        if (isProcessingThreadRunning.load()) {
            isProcessingThreadRunning.store(false);
            pauseProcessing.store(false);
            queueCondition.notify_all();
            processingThread.join(); // Wait for the thread to finish
        }
    }
    SetTimer(hwndMain, ID_TIMER, LAYOUT_CHECK_POLLING, (TIMERPROC)CheckActiveLayout); // Start the timer
# if _DEBUG
    OutputDebugString(TEXT("RESTART!"));
#endif //_DEBUG
}


// Launch the .chroma files editor
void StartEditor() {
    
    pauseAll(); // Pause all ongoing activities
    chroma.LaunchOpenEditor(); // Launch the editor
    startAll(); // Restart all activities
}


// Cleanup and uninitialize the program
int Uninit() {
    return (chroma.Cleanup());
}


// Main message processing function for handling window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP) {
            // Display the context menu
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Handle "Settings"
            chroma.StartSettings();
            break;
        case 2: // Handle "Editor"
            StartEditor();
            break;
        case 3: // Handle "Exit"
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_WTSSESSION_CHANGE: // Handle session change events
        if (wParam == WTS_SESSION_LOCK) {
            pauseAll();
        }
        else if (wParam == WTS_SESSION_UNLOCK) {
            startAll();
        }
        break;
    case WM_DESTROY:
        WTSUnRegisterSessionNotification(hwnd); // Unregister session notifications
        Shell_NotifyIcon(NIM_DELETE, &nid); // Remove the tray icon
        PostQuitMessage(0); // Exit the message loop
        break;
    default:
        // Handle custom WM_CONFIG_CHANGED messages
        if (uMsg == WM_CONFIG_CHANGED) {
            pauseAll();
#ifdef _DEBUG
            OutputDebugString(L"UpdateMessage Received\n");
#endif
            if (!chroma.UpdateConfig()) {
                MessageBox(hwnd, L"Error of the configuration update!", L"Info", MB_OK);
            }
            startAll();
            return 0; // Message processed
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam); // Default message handling
    }
    return 0;
}


// Set up the hidden main window
int SetupMainWindow() {

    // Register the window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc; // Set the window procedure
    wc.hInstance = hInst;
    wc.lpszClassName = TEXT("Razeru2");
    if (!RegisterClass(&wc)) {
        return -1; // Error during registration
    }
    // Create the main hidden window
    hwndMain = CreateWindow(TEXT("Razeru2"), TEXT("Razeru 2 Lang indicator"), 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
    if (!hwndMain) {
        return -1; // Error during window creation
    }
    // Register for session notifications
    if (!WTSRegisterSessionNotification(hwndMain, NOTIFY_FOR_THIS_SESSION)) {

        return -1; // Error registering notifications
    }
    // Register custom message for configuration updates
    WM_CONFIG_CHANGED = RegisterWindowMessage(L"WM_CONFIG_CHANGED");

#if _DEBUG
    std::string message = "CreateWindow " + std::to_string(reinterpret_cast<uintptr_t>(hwndMain)) + ": started\n";
    OutputDebugStringA(message.c_str());
    message = "UINT WM_CONFIG_CHANGED =  " + std::to_string(WM_CONFIG_CHANGED) + ": started\n";
    OutputDebugStringA(message.c_str());
#endif //_DEBUG
    
    return 0; // Success
}


// Set up the tray menu
int SetupMenu() {
    
    // Load strings from the resource file
    wchar_t settings[32], editor[32], exit[32];
    LoadString(hInst, IDS_MENU_SETTINGS, settings, 32);
    LoadString(hInst, IDS_MENU_EDITOR, editor, 32);
    LoadString(hInst, IDS_MENU_EXIT, exit, 32);

    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, settings);
    AppendMenu(hMenu, MF_STRING, 2, editor);
    AppendMenu(hMenu, MF_STRING, 3, exit);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwndMain;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; // Configure the tray icon
    nid.uCallbackMessage = WM_TRAYICON; // Assign the message for tray interactions
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)); // Load the icon

    if (!nid.hIcon) {
        return -1; // Error loading icon
    }
    lstrcpy(nid.szTip, TEXT("Razeru 2 Lang indicator")); // Set the tray icon tooltip

    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        return -1; // Error adding tray icon
    }
    return 0; // Success
}


// Function to display a splash screen
void ShowSplashScreen() {
    // Load the image from resources
    HBITMAP hBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
    if (hBitmap) {
        // Get the dimensions of the image
        BITMAP bitmap;
        GetObject(hBitmap, sizeof(BITMAP), &bitmap);

        // Get the screen dimensions
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Calculate coordinates to center the image
        int posX = (screenWidth - bitmap.bmWidth) / 2;
        int posY = (screenHeight - bitmap.bmHeight) / 2;

        // Create a window to display the image
        HWND hwnd = CreateWindowEx(
            WS_EX_TOOLWINDOW, // Exclude from the taskbar
            L"STATIC", NULL, WS_VISIBLE | WS_POPUP | SS_BITMAP,
            posX, posY, bitmap.bmWidth, bitmap.bmHeight,
            NULL, NULL, GetModuleHandle(NULL), NULL);

        if (hwnd) {
            SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

            // Display the image for 3 seconds
            std::this_thread::sleep_for(std::chrono::seconds(3));

            // Close the window and release resources
            DestroyWindow(hwnd);
        }
        DeleteObject(hBitmap); // Delete the loaded image
    }
}


// Main entry point of the application
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
    
    ShowSplashScreen();

    // Create a mutex to ensure a single instance of the application
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"Global\\Razeruv10AppMutex");
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"The application is already running.", L"Error", MB_ICONERROR);
        return 500; // The application is already running
    }

    // Set the working directory
    std::vector<wchar_t> path(MAX_PATH);
    DWORD size = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    // If the buffer is insufficient, increase its size
    while (size == path.size()) {
        path.resize(path.size() * 2); // Double the buffer size
        size = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    }
    path.resize(size); // Set the exact size
    // Get the directory of the executable file
    std::wstring directoryPath = std::wstring(path.data());
    directoryPath = directoryPath.substr(0, directoryPath.find_last_of(L"\\/"));
    if (!SetCurrentDirectoryW(directoryPath.c_str())) {
#if _DEBUG
        OutputDebugStringW(L"Failed to set working directory.\n");
#endif //_DEBUG
    }
 
    hInst = hInstance; // Store the application instance

    // Set up the main window
    if (SetupMainWindow()) {
        return 2000; // Error setting up the window
    }

    if (InitChromaHandlers()) {
        return 1000; //Error initializing Chroma handlers
    }

    if (!chroma.LoadConfig()) {
        int result = MessageBox(nullptr,
            L"Failed to update configuration. The configuration file is missing or corrupted.\n\n"
            L"Choose an action:\n"
            L"Yes - Use default settings\n"
            L"No - Exit the program.",
            L"Error",
            MB_ICONERROR | MB_YESNO);

        if (result == IDNO) {
            return 1; // Exit the program
        }
        chroma.LoadDefaultConfig(); // Load default configuration
        MessageBox(nullptr, L"Default settings loaded.", L"Information", MB_ICONINFORMATION);
    }

    SetThreadUILanguage(MAKELCID(chroma.g_currentLang, SORT_DEFAULT)); // Set the language

    if (SetupMenu()) {
        return 3000; // Error setting up the menu
    }
    
    GetActiveLayout(); // Check the initial keyboard layout
    // Start the layout check timer
    SetTimer(hwndMain, ID_TIMER, LAYOUT_CHECK_POLLING, (TIMERPROC)CheckActiveLayout);
    
    keyStateMap.reserve(MAX_SIZE); // Reserve buffer size for key states

    if (chroma.IsKeyAnimationOn() || chroma.IsColorKeyboardOn()) {
        isProcessingThreadRunning.store(true);
        // Start the event processing thread
        processingThread = std::thread(EventProcessingThread);
    }
    if (chroma.IsKeyAnimationOn()) {
        // Set up the keyboard hook
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInst, 0);
        if (!hHook) {
#if _DEBUG
            OutputDebugString(TEXT("Failed to set hook!"));
#endif //_DEBUG            
            return 4000; // Error setting the hook
        }
        isHookSet.store(true);
    } 

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
   
    if (isHookSet.load()) {
        // Remove the hook before exiting
        UnhookWindowsHookEx(hHook);
    }
    
    if (isProcessingThreadRunning.load()) {
        isProcessingThreadRunning.store(false);
        queueCondition.notify_all();
        processingThread.join(); // Wait for the thread to finish
    }

    int rezult = Uninit(); // Perform cleanup
    
    // Release the mutex before exiting
    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return rezult; // Exit with result code
}
