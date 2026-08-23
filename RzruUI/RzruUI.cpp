// RzruUI.cpp
//
#pragma once

#include "pch.h"
#include "RzruUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::unique_ptr<ChromaPlaying::ConfigData> localData = nullptr;
BOOL isInStartup = FALSE;

BEGIN_MESSAGE_MAP(CRzruUIApp, CWinApp)
END_MESSAGE_MAP()


// CRzruUIApp constructor: Initializes the application object
CRzruUIApp::CRzruUIApp() {
#ifdef _DEBUG
    OutputDebugString(L"RzruUI.dll: Initialization\n");
#endif
}


// CRzruUIApp destructor: Cleans up resources when the application object is destroyed
CRzruUIApp::~CRzruUIApp() {
#ifdef _DEBUG
    OutputDebugString(L"RzruUI.dll: Cleanup\n");
#endif
}



// Application instance of CRzruUIApp
CRzruUIApp theApp;

// Initializes the application instance
BOOL CRzruUIApp::InitInstance() {
    CWinApp::InitInstance();
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    return TRUE; // Indicate successful initialization
}


// Cleans up before exiting the application instance
 int CRzruUIApp::ExitInstance() {
     if (g_waitForResponse && g_responseEvent) {
#ifdef _DEBUG
         OutputDebugString(L"Waiting for response before exiting...\n");
#endif
         DWORD waitResult = WaitForSingleObject(g_responseEvent, 2000); // Wait for up to 2 seconds
#ifdef _DEBUG
         if (waitResult == WAIT_OBJECT_0) {
             OutputDebugString(L"Response received. Exiting.\n");
         }
         else if (waitResult == WAIT_TIMEOUT) {
             OutputDebugString(L"Timeout reached. Exiting.\n");
         }
         else {
             OutputDebugString(L"Unexpected wait result. Exiting.\n");
         }
#endif
     }
     // Release the event handle
     if (g_responseEvent) {
         CloseHandle(g_responseEvent);
         g_responseEvent = nullptr;
     }
     localData.reset(); // Reset the local configuration data
     
     return CWinApp::ExitInstance();
}


 // Initializes the DLL with the provided parent window handle and configuration change message
extern "C" __declspec(dllexport) void InitializeDLL(HWND parent, UINT configChangedMessage) {
#ifdef _DEBUG
    OutputDebugString(L"DLL Initialized\n");
#endif
    g_notifyWindow = parent;  // Сохраняем дескриптор окна
    WM_CHANGED = configChangedMessage;

    if (localData == nullptr) {
        localData = std::make_unique<ChromaPlaying::ConfigData>();
    }
}


// Displays the configuration dialog
extern "C" __declspec(dllexport) void ShowConfigWindow() {

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (localData != nullptr) {
        SetThreadUILanguage(MAKELCID(localData->g_currentLang, SORT_DEFAULT));
        // Создаем объект диалога
        CConfigDialog configDialog;
        configDialog.DoModal(); // Show the dialog modally
    }
}


// Loads configuration from a JSON file
extern "C" __declspec(dllexport) bool LoadConfiguration() {
#ifdef _DEBUG
    OutputDebugString(L"LoadConfiguration started\n");
#endif
    try {
        std::string path = SetExecutablePath(); // Get the executable path
     
        // Открываем файл
        std::ifstream inFile(path + "\\" + CONFIG_FILE);
        if (!inFile.is_open()) {
            return false; // Failed to open the file
        }

        json j;
        inFile >> j; // Parse the JSON file
        inFile.close();
        
        localData = std::make_unique<ChromaPlaying::ConfigData>(); // Reset the configuration data

        // Load primary and sublanguage from the JSON
        WORD primaryLang = j.contains("primary_lang") ? j["primary_lang"].get<WORD>() : LANG_ENGLISH;
        WORD subLang = j.contains("sub_lang") ? j["sub_lang"].get<WORD>() : SUBLANG_DEFAULT;
        localData->g_currentLang = MAKELANGID(primaryLang, subLang);
        
        localData->devicesAnimation = j.contains("devicesAnimation") ? j["devicesAnimation"].get<bool>() : true;
        
        // Load keyboard effects
        localData->keyboardEffect.clear();
        if (j.contains("keyboardEffect")) {
            for (const auto& effectJson : j["keyboardEffect"]) {
                ChromaPlaying::ChromaKeyboardEffect effect;
                effect.keyboardAnimation = effectJson.contains("keyboardAnimation") ? effectJson["keyboardAnimation"].get<std::string>() : "";
                effect.keyboardLayout = effectJson.contains("keyboardLayout") ? effectJson["keyboardLayout"].get<LANGID>() : 0x00000409;
                effect.frameDuration = effectJson.contains("frameDuration") ? effectJson["frameDuration"].get<int>() : 33;
                effect.frames = effectJson.contains("frames") ? effectJson["frames"].get<int>() : 0;
                effect.frameIndex = effectJson.contains("frameIndex") ? effectJson["frameIndex"].get<int>() : 0;
                effect.speed = effectJson.contains("speed") ? effectJson["speed"].get<int>() : 1;
                effect.mouseAnimation = effectJson.contains("mouseAnimation") ? effectJson["mouseAnimation"].get<std::string>() : "";
                effect.mousePadAnimation = effectJson.contains("mousePadAnimation") ? effectJson["mousePadAnimation"].get<std::string>() : "";
                effect.chromaLinkAnimation = effectJson.contains("chromaLinkAnimation") ? effectJson["chromaLinkAnimation"].get<std::string>() : "";
                effect.headSetAnimation = effectJson.contains("headSetAnimation") ? effectJson["headSetAnimation"].get<std::string>() : "";
                effect.keyPadAnimation = effectJson.contains("keyPadAnimation") ? effectJson["keyPadAnimation"].get<std::string>() : "";
                localData->keyboardEffect.push_back(effect);
            }
        }
        
        // Load key effect
        if (j.contains("keyEffect")) {
            localData->keyEffect.name = j["keyEffect"].contains("name") ? j["keyEffect"]["name"].get<EFFECT_TYPE>() : EFFECT_TYPE::CHROMA_NONE;
            localData->keyEffect.primaryColor = j["keyEffect"].contains("primaryColor") ? j["keyEffect"]["primaryColor"].get<int>() : 255;
            localData->keyEffect.frameDuration = j["keyEffect"].contains("frameDuration") ? j["keyEffect"]["frameDuration"].get<int>() : 33;
            localData->keyEffect.framesOfKeyAnimation = j["keyEffect"].contains("framesOfKeyAnimation") ? j["keyEffect"]["framesOfKeyAnimation"].get<int>() : 33;
        }
        
        // Load backlight effect
        if (j.contains("backlightEffect")) {
            localData->backlightEffect.backLightOn = j["backlightEffect"].contains("backLightOn") ? j["backlightEffect"]["backLightOn"].get<bool>() : false;
            localData->backlightEffect.blend = j["backlightEffect"].contains("blend") ? j["backlightEffect"]["blend"].get<EChromaSDKSceneBlend>() : EChromaSDKSceneBlend::SB_None;
            localData->backlightEffect.mode = j["backlightEffect"].contains("mode") ? j["backlightEffect"]["mode"].get<EChromaSDKSceneMode>() : EChromaSDKSceneMode::SM_Replace;
            
            // Load colorsKeyboard effect
            localData->backlightEffect.colorsKeyboard.clear();
            if (j["backlightEffect"].contains("colorsKeyboard")) {
                for (const auto& line : j["backlightEffect"]["colorsKeyboard"]) {
                    std::stringstream ss(line.get<std::string>());
                    std::string value;
                    while (std::getline(ss, value, ',')) {
                        try {
                            // Convert the string value to an integer and add it to the array
                            localData->backlightEffect.colorsKeyboard.push_back(std::stoi(value));
                        }
                        catch (const std::exception&) {
                            // Ignore invalid values and continue processing the next
                            // Invalid data may occur due to malformed input or unexpected characters
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception&) {
        // Handle possible errors gracefully
        return false;
    }
    localData->backlightEffect.colorsKeyboard.resize(ChromaPlaying::SIZEKEYBOARD, 0); // Resize and fill by zeros

#if _DEBUG
    OutputDebugString(L"LoadConfiguration ended\n");
#endif

    return true;
}


// Exported function to save the configuration to a JSON file
extern "C" __declspec(dllexport) bool SaveConfiguration() { // Ensure that the configuration data exists
    if (localData != nullptr) {
        try {
            // Create a JSON object to store the configuration
            json j;
#if _DEBUG
            OutputDebugString(L"Saving started\n");
#endif //_DEBUG
            // Save the primary and sub-language settings
            j["primary_lang"] = PRIMARYLANGID(localData ->g_currentLang);
            j["sub_lang"] = SUBLANGID(localData->g_currentLang);
            // Save the devicesAnimation flag
            j["devicesAnimation"] = localData->devicesAnimation;

            // Save the keyboard effects
            for (const auto& effect : localData->keyboardEffect) {
                j["keyboardEffect"].push_back({
                    {"keyboardAnimation", effect.keyboardAnimation},
                    {"keyboardLayout", effect.keyboardLayout},
                    {"frameDuration", effect.frameDuration},
                    {"frames", effect.frames},
                    {"frameIndex", effect.frameIndex},
                    {"speed", effect.speed},
                    {"mouseAnimation", effect.mouseAnimation},
                    {"mousePadAnimation", effect.mousePadAnimation},
                    {"chromaLinkAnimation", effect.chromaLinkAnimation},
                    {"headSetAnimation", effect.headSetAnimation},
                    {"keyPadAnimation", effect.keyPadAnimation}
                    });
            }

            // Save the key effect settings
            j["keyEffect"] = {
                {"name", localData->keyEffect.name},
                {"primaryColor", localData->keyEffect.primaryColor},
                {"frameDuration", localData->keyEffect.frameDuration},
                {"framesOfKeyAnimation", localData->keyEffect.framesOfKeyAnimation}
            };

            // Save the backlight effect settings
            j["backlightEffect"] = {
                {"backLightOn", localData->backlightEffect.backLightOn},
                {"blend", localData->backlightEffect.blend},
                {"mode", localData->backlightEffect.mode}
            };

            // Format the `colorsKeyboard` array as strings with up to 22 values per line
            std::vector<std::string> formattedColors;
            std::string currentLine;
            int valueCount = 0;
            for (size_t i = 0; i < localData->backlightEffect.colorsKeyboard.size(); ++i) {
                currentLine += std::to_string(localData->backlightEffect.colorsKeyboard[i]);
                valueCount++;

                if (i < localData->backlightEffect.colorsKeyboard.size() - 1) {
                    currentLine += ",";
                }
                // If the line has reached 22 values or it's the last value, finalize the line
                if (valueCount == 22 || i == localData->backlightEffect.colorsKeyboard.size() - 1) {
                    formattedColors.push_back(currentLine);
                    currentLine.clear();
                    valueCount = 0;
                }
            }
            // Add the formatted colors to the JSON object
            j["backlightEffect"]["colorsKeyboard"] = formattedColors;

            // Write the JSON object to the configuration file
            std::string path = SetExecutablePath(); // Get the path to the executable's directory
            
            std::ofstream outFile(path + "\\" + CONFIG_FILE, std::ios::out | std::ios::trunc);
            if (!outFile.is_open()) {
                return false; // Return false if the file could not be opened
            }
            outFile << j.dump(4); // Write the JSON with indentation for readability
            outFile.close(); // Close the file
        }
        catch (const std::exception&) {
            // Handle possible errors gracefully
            return false;
        }
#if _DEBUG
        OutputDebugString(L"Saving ended\n");
#endif //_DEBUG
        return true; // Indicate that the configuration was saved successfully
    }
    return false; // Return false if the configuration data is null
}


// Exported function to get a pointer to the current configuration data
extern "C" __declspec(dllexport) ChromaPlaying::ConfigData* GetConfiguration() {
    return localData.get(); // Return the unique pointer to the configuration data
}


// Exported function to set the configuration data
extern "C" __declspec(dllexport) void SetConfiguration(ChromaPlaying::ConfigData* configData) {
    if (configData != nullptr) {
        // Deep copy the provided configuration data into localData
        localData = std::make_unique<ChromaPlaying::ConfigData>(*configData);
    }
    else {
        // Reset localData if null is passed
        localData.reset();
    }
}


// Exported function to signal that a response is ready
extern "C" __declspec(dllexport) void SignalResponseReady() {
    if (g_responseEvent) {
        SetEvent(g_responseEvent); // Trigger the event to notify waiting threads
    }
}


BEGIN_MESSAGE_MAP(CConfigDialog, CDialogEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CONTROL, &CConfigDialog::OnTcnSelchangeTabControl)
    ON_BN_CLICKED(IDC_LOAD, &CConfigDialog::OnBnClickedLoad)
    ON_BN_CLICKED(IDC_APPLY, &CConfigDialog::OnBnClickedApply)
    ON_BN_CLICKED(IDC_SAVE, &CConfigDialog::OnBnClickedSave)
    ON_BN_CLICKED(IDCANCEL, &CConfigDialog::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &CConfigDialog::OnBnClickedOk)

END_MESSAGE_MAP()


// Constructor for the configuration dialog
CConfigDialog::CConfigDialog(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_CONFIG_DIALOG, pParent) {
    
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // Load the main application icon
    // Initialize all tab page pointers to nullptr
    for (int i = 0; i < PAGE_NUMBERS_IN_TAB; ++i) {
        m_tabPages[i] = nullptr;
    }
}
 

// Destructor for the configuration dialog
CConfigDialog::~CConfigDialog() {
    for (int i = 0; i < PAGE_NUMBERS_IN_TAB; ++i) {
        if (m_tabPages[i]) {
            delete m_tabPages[i]; // Delete dynamically allocated tab pages
            m_tabPages[i] = nullptr;
        }
    }
}


void CConfigDialog::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}


// Initialization handler for the dialog
BOOL CConfigDialog::OnInitDialog() {

    CDialogEx::OnInitDialog();
    // Check if the application is set to start at login
    isInStartup = IsInStartup();
    // Make a backup of the current configuration data
    backupLocalData = std::make_unique<ChromaPlaying::ConfigData>(*localData);

    // Set the icon for this dialog.  The framework does this automatically
//  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // Load the dialog title from resources and set it
    CString strTitle; 
    if (strTitle.LoadString(IDS_TITLE)) {
        SetWindowText(strTitle);
    }
    // Initialize tab control
    pTabCtrl = (CTabCtrl*)GetDlgItem(IDC_TAB_CONTROL);
    // Add tabs to the tab control
    AddTab(0, new CTAB1(this), IDD_TAB_1, IDS_TAB1_TITLE);
    AddTab(1, new CTAB2(this), IDD_TAB_2, IDS_TAB2_TITLE);
    AddTab(2, new CTAB3(this), IDD_TAB_3, IDS_TAB3_TITLE);
    AddTab(3, new CTAB4(this), IDD_TAB_4, IDS_TAB4_TITLE);

    // Show the first tab by default
    m_tabPages[0]->ShowWindow(SW_SHOW);

    // Create a response event
    g_responseEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
#ifdef _DEBUG
    if (!g_responseEvent) {
        OutputDebugString(L"Failed to create response event\n");
    }
#endif
    return FALSE; // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


// Adds a new tab to the tab control
void CConfigDialog::AddTab(int tabIndex, CDialogEx* pDlg, UINT dlgResID, UINT strResID)
{
    m_tabPages[tabIndex] = pDlg; // Assign the dialog pointer to the tab array
    
    // Create the dialog
    pDlg->Create(dlgResID, pTabCtrl);

    // Load the tab title
    CString title;
    if (!title.LoadString(strResID)) {
        return;
    }

    // Set up the tab item
    TCITEM item = {};
    item.mask = TCIF_TEXT | TCIF_PARAM;
    item.lParam = (LPARAM)pDlg;
    item.pszText = title.GetBuffer();
    // Insert the tab into the control
    pTabCtrl->InsertItem(tabIndex, &item);

    // Position the tab
    CRect rc;
    pTabCtrl->GetClientRect(&rc);
    ::SendMessage(pTabCtrl->GetSafeHwnd(), TCM_ADJUSTRECT, FALSE, (LPARAM)&rc);
    pDlg->SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_HIDEWINDOW | SWP_NOZORDER);
}


// Handler for switching between tabs
void CConfigDialog::OnTcnSelchangeTabControl(NMHDR* pNMHDR, LRESULT* pResult) {

    int selectedTab = pTabCtrl->GetCurSel(); // Get the index of the selected tab

    // Hide all tab pages
    for (int i = 0; i < PAGE_NUMBERS_IN_TAB; ++i) {
        if (m_tabPages[i]) {
            m_tabPages[i]->ShowWindow(SW_HIDE);
        }
    }
    // Show the selected tab page
    if (m_tabPages[selectedTab]) {
        m_tabPages[selectedTab]->ShowWindow(SW_SHOW);
    }
    // Set the result to indicate success
    *pResult = 0;
}


// Sends a notification message to update the configuration
void CConfigDialog::SendUpdateMessage() { // Check if the target window and message are valid

    if (g_notifyWindow && WM_CHANGED) {
        if (g_responseEvent) {
            ResetEvent(g_responseEvent); // Reset the response event
        }
        ::PostMessage(g_notifyWindow, WM_CHANGED, 0, 0); // Send the update message
        g_waitForResponse = true; // Set the flag indicating a response is expected
#ifdef _DEBUG
            OutputDebugString(L"SendUpdateMessage\n");
#endif
    }
}


// Handler for the "Load" button click event
void CConfigDialog::OnBnClickedLoad() {
    
    if (LoadConfiguration()) {
        ::MessageBox(NULL, _T("Configuration successfully loaded\nfrom the file `Razeru.json`"), _T("Info"), MB_OK);
        UpdateAllTabs(); // Notify all tabs about data changes
    }
    else {
        ::MessageBox(NULL, _T("Failed to load configuration\nfrom the file`Razeru.json`"), _T("Info"), MB_OK | MB_ICONERROR);
    }
}


// Handler for the "Apply" button click event
void CConfigDialog::OnBnClickedApply() {

    CConfigDialog::SendUpdateMessage(); // Notify about the update
    AddToStartup(); // Add the application to startup if needed
    UpdateAllTabs(); // Notify all tabs about data changes
}


// Handler for the "Save" button click event
void CConfigDialog::OnBnClickedSave() {
    
    if (SaveConfiguration()) {
        ::MessageBox(NULL, _T("Configuration successfully saved\nto the file `Razeru.json`"), _T("Info"), MB_OK);
    }
    else {
        ::MessageBox(NULL, _T("Failed to save configuration\nto the file`Razeru.json`"), _T("Info"), MB_OK | MB_ICONERROR);
    }
    CConfigDialog::SendUpdateMessage(); // Notify about the update
    AddToStartup(); // Add the application to startup if needed
}


// Handler for the "Cancel" button click event
void CConfigDialog::OnBnClickedCancel() {
    
    // Restore the backup configuration data
    localData = std::make_unique<ChromaPlaying::ConfigData>(*backupLocalData);
    
    CConfigDialog::SendUpdateMessage(); // Notify about the cancellation
    
    // Hack to force processing the message queue, 
    // as CDialogEx::OnCancel() blocks message dispatch
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    CDialogEx::OnCancel();  // Call the base class handler
}


// Handler for the "OK" button click event
void CConfigDialog::OnBnClickedOk() { // Notify about the update
    
    CConfigDialog::SendUpdateMessage(); // Add the application to startup if needed
    AddToStartup();
    
    // Hack to force processing the message queue, 
    // as CDialogEx::OnOK() blocks message dispatch
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    CDialogEx::OnOK();  // Call the base class handler
}


// Adds or removes the application from Windows startup
void CConfigDialog::AddToStartup() {
    
    bool success = false;
    
    // Check if the CTAB4 object exists
    if (m_tabPages[3] != nullptr) {

        CTAB4* pTab4 = dynamic_cast<CTAB4*>(m_tabPages[3]);
        if (pTab4 != nullptr) {
            if (pTab4->m_CheckAutoload == isInStartup) {
                success = true;
            }
            else {
                HKEY hKey;
                std::string regPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

                if (pTab4->m_CheckAutoload) { // Add to startup

                    std::string appPath = "\"" + SetExecutableFile() + "\"";

                    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                        if (RegSetValueExA(hKey, APP_NAME, 0, REG_SZ,
                            reinterpret_cast<const BYTE*>(appPath.c_str()),
                            static_cast<DWORD>(appPath.size() + 1)) == ERROR_SUCCESS) {
                            isInStartup = true;
                            success = true;
                        }
                        RegCloseKey(hKey);
                    }
                }
                else { // Remove from startup
                    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                        if (RegDeleteValueA(hKey, APP_NAME) == ERROR_SUCCESS) {
                            isInStartup = false;
                            success = true;
                        }
                        RegCloseKey(hKey);
                    }
                }
            }
        }
    }
    if (!success) {
        if (m_tabPages[3] != nullptr) {
            CTAB4* pTab4 = dynamic_cast<CTAB4*>(m_tabPages[3]);
            if (pTab4 != nullptr) {
                pTab4->m_CheckAutoload = isInStartup; // Sync checkbox state
                pTab4->UpdateData(FALSE); // Update the interface
            }
        }
        ::MessageBox(NULL, _T("Failed to add/remove the application in Windows registry. Check permissions or registry settings."), _T("Error"), MB_OK | MB_ICONERROR);
    }
}


// Checks if the application is set to start at login
BOOL CConfigDialog::IsInStartup() {
    
    BOOL isInStartup = FALSE;
    HKEY hKey;
    std::string regPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    // Open the registry key
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        std::vector<char> value(MAX_PATH);
        DWORD valueSize = static_cast<DWORD>(value.size());
        LONG result = RegQueryValueExA(hKey, APP_NAME, nullptr, nullptr, reinterpret_cast<LPBYTE>(value.data()), &valueSize);

        if (result == ERROR_MORE_DATA) {
            value.resize(valueSize);  // Resize buffer if needed
            result = RegQueryValueExA(hKey, APP_NAME, nullptr, nullptr, reinterpret_cast<LPBYTE>(value.data()), &valueSize);
        }
        if (result == ERROR_SUCCESS) {
            isInStartup = TRUE;
        }
        RegCloseKey(hKey);
    }
    return isInStartup;
}

// Retrieves the full path to the executable file
std::string SetExecutableFile() {
    
    std::vector<char> path(MAX_PATH); // Buffer initialization
    DWORD size = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));

    // If the buffer is too small, increase its size
    while (size == path.size()) {
        path.resize(path.size() * 2);
        size = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
    }
    path.resize(size); // Trim the buffer to the exact size
    std::string appPath(path.begin(), path.end());// Convert to std::string
    return appPath;
}


// Retrieves the directory path of the executable
std::string SetExecutablePath() {
    
    std::string path = SetExecutableFile();
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return path.substr(0, lastSlash); // Remove the executable name, leaving only the directory
    }
    return ""; // Return an empty string if no slash is found
}


void CConfigDialog::UpdateAllTabs() {
    for (int i = 0; i < PAGE_NUMBERS_IN_TAB; ++i) {
        if (m_tabPages[i] != nullptr) {
            m_tabPages[i]->SendMessage(WM_TAB_UPDATE, 0, 0); // Send a message to update data
        }
    }
}