#pragma once
#include "pch.h"
#include "RzruUI.h"
#include "CTAB1.h"



// Dialog box CTAB1
BEGIN_MESSAGE_MAP(CTAB1, CDialogEx)
    // Handler for changing selection in language combo box
    ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CTAB1::OnCbnSelchangeComboLang)
    // Handlers for various button clicks
    ON_BN_CLICKED(IDC_BUTTON_KEYBOARD_AN, &CTAB1::OnBnClickedButton)
    ON_BN_CLICKED(IDC_BUTTON_MOUSE_AN, &CTAB1::OnBnClickedButton)
    ON_BN_CLICKED(IDC_BUTTON_MOUSEPD_AN, &CTAB1::OnBnClickedButton)
    ON_BN_CLICKED(IDC_BUTTON_CHRONA_AN, &CTAB1::OnBnClickedButton)
    ON_BN_CLICKED(IDC_BUTTON_HS_AN, &CTAB1::OnBnClickedButton)
    ON_BN_CLICKED(IDC_BUTTON_KEYPD_AN, &CTAB1::OnBnClickedButton)
    // Handlers for editing text in different fields
    ON_EN_CHANGE(IDC_EDIT_KEYBOARD_AN, &CTAB1::OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_MOUSE_AN, &CTAB1::OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_MOUSEPD_AN, &CTAB1::OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_CHROMA_AN, &CTAB1::OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_HS_AN, &CTAB1::OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_KEYPD_AN, &CTAB1::OnEditChange)
    // Handler for checkbox click
    ON_BN_CLICKED(IDC_CHECK_DEV_AN, &CTAB1::OnBnClickedCheckDevAn)
    // Handler for slider changes
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SPEED, &CTAB1::OnSliderSpeedChange)
    ON_WM_PAINT() // Handler for paint events
    ON_MESSAGE(WM_TAB_UPDATE, &CTAB1::OnTabUpdate)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CTAB1, CDialogEx)


// Constructor for CTAB1
CTAB1::CTAB1(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_TAB_1, pParent) {
}


// Destructor for CTAB1
CTAB1::~CTAB1() {
}


void CTAB1::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_LANG, m_comboLangs);
    DDX_Control(pDX, IDC_SLIDER_SPEED, m_sliderSpeed);
    DDX_Check(pDX, IDC_CHECK_DEV_AN, m_bCheckDevAn);
}


BOOL CTAB1::OnInitDialog() {
    
    CDialogEx::OnInitDialog();
    // Set text labels
    UINT controlIDs[] = { IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC4, IDC_STATIC5, IDC_STATIC6, IDC_STATIC7, IDC_STATIC8,
                         IDC_CHECK_DEV_AN, IDC_STATIC9 };
    UINT stringIDs[] = { IDS_TAB1_LAYOUT, IDS_TAB1_KEYB_AN, IDS_TAB1_MOUSE_AN, IDS_TAB1_MOUSPD_AN, IDS_TAB1_CHROMA_AN, IDS_TAB1_HS_AN,
                         IDS_TAB1_KEYPD_AN, IDS_TAB1_ANIM_SPEED, IDS_TAB1_DEV_AN, IDS_TAB1_INFO };
    for (int i = 0; i < _countof(controlIDs); ++i) {
        // Load string from String Table
        CString strText;
        if (strText.LoadString(stringIDs[i])) {
            // Set text in static control
            CStatic* pStatic = (CStatic*)GetDlgItem(controlIDs[i]);
            if (pStatic != nullptr) {
                pStatic->SetWindowText(strText);
            }
        }
    }
    
    // Set icons for buttons
    HICON icon = (HICON)::LoadImage(AfxGetInstanceHandle(),
        MAKEINTRESOURCE(IDI_FOLDER_NORM),
        IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

    UINT buttonIDs[] = { IDC_BUTTON_KEYBOARD_AN, IDC_BUTTON_MOUSE_AN, IDC_BUTTON_MOUSEPD_AN, IDC_BUTTON_CHRONA_AN, IDC_BUTTON_HS_AN, IDC_BUTTON_KEYPD_AN };

    for (int i = 0; i < _countof(buttonIDs); ++i) {

        CButton* pButton = (CButton*)GetDlgItem(buttonIDs[i]);
        if (pButton) {
            pButton->SetIcon(icon);
        }
    }

    // Get the number of keyboard layouts (HKL)
    int nLayouts = GetKeyboardLayoutList(0, NULL);
    if (nLayouts <= 0)
        return FALSE; // No languages? Very strange, but exit

    // Allocate memory for HKL array
    HKL* pLayouts = new HKL[nLayouts];
    nLayouts = GetKeyboardLayoutList(nLayouts, pLayouts); // Fill the array with real HKL

    // Vector to store current LANGIDs
    std::vector<LANGID> currentLangIDs;

    // Iterate through the layouts and populate the combo box
    for (int i = 0; i < nLayouts; i++)
    {
        // Extract LANGID
        LANGID langID = LOWORD(pLayouts[i]);
        currentLangIDs.push_back(langID); // Add to the vector of current LANGIDs

        // Create LCID
        LCID locale = MAKELCID(langID, SORT_DEFAULT);

        // Get human-readable language name
        // Can use LOCALE_SLANGUAGE or LOCALE_SLANGDISPLAYNAME
        // LOCALE_SLANGDISPLAYNAME gives a more "complete" name but isn't available in all Windows versions.
        wchar_t langName[128] = { 0 };
        if (GetLocaleInfoW(locale, LOCALE_SLANGDISPLAYNAME, langName, _countof(langName)) == 0) {
            // If LOCALE_SLANGDISPLAYNAME fails, try LOCALE_SLANGUAGE
            GetLocaleInfoW(locale, LOCALE_SLANGUAGE, langName, _countof(langName));
        }
        // Add the string to the list
        int newIndex = m_comboLangs.AddString(langName);
        // Associate LANGID with combo box item
        m_comboLangs.SetItemData(newIndex, langID);
    }
    // Release temporary buffer
    delete[] pLayouts;

    editControls = {
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_KEYBOARD_AN)),
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_MOUSE_AN)),
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_MOUSEPD_AN)),
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_CHROMA_AN)),
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_HS_AN)),
        static_cast<CEdit*>(GetDlgItem(IDC_EDIT_KEYPD_AN))
    };

    // Map buttons to indexes
    buttonToEditIndex = {
        {IDC_BUTTON_KEYBOARD_AN, 0},
        {IDC_BUTTON_MOUSE_AN, 1},
        {IDC_BUTTON_MOUSEPD_AN, 2},
        {IDC_BUTTON_CHRONA_AN, 3},
        {IDC_BUTTON_HS_AN, 4},
        {IDC_BUTTON_KEYPD_AN, 5}
    };

    if (m_comboLangs.GetCount() > 0) {
        m_comboLangs.SetCurSel(0); // Select the first item
        SelectLayoutData(m_comboLangs.GetCurSel());
    }

    for (const auto& deviceType : localData->connectedDevices) {
        switch (deviceType.DeviceType) {
        case DEVICE_INFO_TYPE::DEVICE_MOUSE:
            GetDlgItem(IDC_STATIC_MOUSE_AN)->SetWindowText(L"✔ ");
            break;
        case DEVICE_INFO_TYPE::DEVICE_MOUSEPAD:
            GetDlgItem(IDC_STATIC_MOUSEPD_AN)->SetWindowText(L"✔ ");
            break;
        case DEVICE_INFO_TYPE::DEVICE_CHROMALINK:
            GetDlgItem(IDC_STATIC_CHROMA_AN)->SetWindowText(L"✔ ");
            break;
        case DEVICE_INFO_TYPE::DEVICE_HEADSET:
            GetDlgItem(IDC_STATIC_HS_AN)->SetWindowText(L"✔ ");
            break;

        case DEVICE_INFO_TYPE::DEVICE_KEYPAD:
            GetDlgItem(IDC_STATIC_KEYPD_AN)->SetWindowText(L"✔ ");
            break;

        default:
            // Optional: Handle unsupported device types or log a warning
            break;
        }
    }
    m_sliderSpeed.SetRange(-5, 5);       // Set the range
    m_sliderSpeed.SetTicFreq(1);        // Tick frequency
   
    UpdateDataFromLocal();

    return FALSE; // Return TRUE if the focus is not on a control
}


void CTAB1::SynchronizeKeyboardEffects() {
    
    // Get the number of elements in m_comboLangs
    int comboCount = m_comboLangs.GetCount();
    if (comboCount == CB_ERR || comboCount == 0) {
        return; // No data in the combo box
    }
    
    // 1. Add new elements if the number of layouts is greater than the current effects
    for (int i = 0; i < comboCount; ++i) {
        LANGID langID = static_cast<LANGID>(m_comboLangs.GetItemData(i));
        bool exists = false;
        for (const auto& effect : localData->keyboardEffect) {
            if (effect.keyboardLayout == langID) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            ChromaPlaying::ChromaKeyboardEffect newEffect;
            newEffect.keyboardLayout = langID;
            localData->keyboardEffect.push_back(newEffect);
        }
    }
    // 2. Remove extra elements if the number of layouts is less than the current effects
    localData->keyboardEffect.erase(
        std::remove_if(
            localData->keyboardEffect.begin(),
            localData->keyboardEffect.end(),
            [this](const ChromaPlaying::ChromaKeyboardEffect& effect) {
                int comboCount = m_comboLangs.GetCount();
                for (int i = 0; i < comboCount; ++i) {
                    LANGID langID = static_cast<LANGID>(m_comboLangs.GetItemData(i));
                    if (effect.keyboardLayout == langID) {
                        return false; // Match found
                    }
                }
                return true; // Delete if not found
            }
        ),
        localData->keyboardEffect.end()
    );
}

// Handle combo box selection change
void CTAB1::OnCbnSelchangeComboLang() {
    
    LANGID selIndex = m_comboLangs.GetCurSel();
    if (selIndex != LB_ERR) {
        SelectLayoutData(selIndex);
    }
    return;
}


// Open a file dialog to select a .chroma file
CString CTAB1::OpenFileDialog() {

    CFileDialog dlg(TRUE, _T("chroma"), nullptr, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Chroma Files (*.chroma)|*.chroma|All Files (*.*)|*.*||"));

    if (dlg.DoModal() == IDOK) {
        return dlg.GetPathName();
    }
    return CString();
}


void CTAB1::OnBnClickedButton() {
    
    // Get the ID of the clicked button
    int buttonID = reinterpret_cast<CWnd*>(GetFocus())->GetDlgCtrlID();

    auto it = buttonToEditIndex.find(buttonID);
    
    if (it != buttonToEditIndex.end()) {
        size_t editIndex = it->second;

        CString filePath = OpenFileDialog();
        if (!filePath.IsEmpty() && editIndex < editControls.size()) {
            editControls[editIndex]->SetWindowText(filePath);

            // Also update localData->keyboardEffect if m_layoutIndex is valid
            if (m_layoutIndex != -1 && m_layoutIndex < localData->keyboardEffect.size()) {
                std::vector<std::string*> animations = {
                    &localData->keyboardEffect[m_layoutIndex].keyboardAnimation,
                    &localData->keyboardEffect[m_layoutIndex].mouseAnimation,
                    &localData->keyboardEffect[m_layoutIndex].mousePadAnimation,
                    &localData->keyboardEffect[m_layoutIndex].chromaLinkAnimation,
                    &localData->keyboardEffect[m_layoutIndex].headSetAnimation,
                    &localData->keyboardEffect[m_layoutIndex].keyPadAnimation
                };
                if (editIndex < animations.size()) {
                    *animations[editIndex] = std::string(CT2A(filePath));
                }
            }
        }
    }
}


void CTAB1::SelectLayoutData(LANGID selIndex) {

    // Get layout handle (or LANGID) from item data
    LANGID layout = static_cast<LANGID>(m_comboLangs.GetItemData(selIndex));

    auto it = std::find_if(localData->keyboardEffect.begin(),
        localData->keyboardEffect.end(),
        [&layout](const ChromaPlaying::ChromaKeyboardEffect& keybordEffect) {
            return keybordEffect.keyboardLayout == layout;
        });

    if (it != localData->keyboardEffect.end()) {
        m_layoutIndex = static_cast<int>(std::distance(localData->keyboardEffect.begin(), it));
        // Array of strings to link with CEdit
        std::vector<std::string> animations = {
            it->keyboardAnimation,
            it->mouseAnimation,
            it->mousePadAnimation,
            it->chromaLinkAnimation,
            it->headSetAnimation,
            it->keyPadAnimation
        };
        // Populate fields
        for (size_t i = 0; i < editControls.size(); ++i) {
            if (i < animations.size()) {
                editControls[i]->SetWindowText(CString(animations[i].c_str()));
            }
        }
    }
    else {
        m_layoutIndex = -1;
        // Clear all fields
        for (auto* editControl : editControls) {
            editControl->SetWindowText(_T(""));
        }
    }
    
    SetSlider();

# if _DEBUG
    TCHAR buffer[50];
    _stprintf_s(buffer, _T("LANGID: 0x%04X\n"), layout);
    OutputDebugString(buffer);
#endif //_DEBUG
}


// Handle changes in edit fields
void CTAB1::OnEditChange() {
    
    if (m_layoutIndex == -1 || m_layoutIndex >= localData->keyboardEffect.size()) {
        return; // Invalid index
    }

    // Array of pointers to keyboardEffect fields
    std::vector<std::string*> animations = {
        &localData->keyboardEffect[m_layoutIndex].keyboardAnimation,
        &localData->keyboardEffect[m_layoutIndex].mouseAnimation,
        &localData->keyboardEffect[m_layoutIndex].mousePadAnimation,
        &localData->keyboardEffect[m_layoutIndex].chromaLinkAnimation,
        &localData->keyboardEffect[m_layoutIndex].headSetAnimation,
        &localData->keyboardEffect[m_layoutIndex].keyPadAnimation
    };

    for (size_t i = 0; i < editControls.size(); ++i) {
        CString text;
        editControls[i]->GetWindowText(text);

        // Update the corresponding field in localData->keyboardEffect
        if (i < animations.size()) {
            *animations[i] = std::string(CT2A(text));
        }
    }
}


void CTAB1::OnBnClickedCheckDevAn() {
    
    // Update variable from the interface
    UpdateData(TRUE);
    // Link the value to localData->devicesAnimation
    localData->devicesAnimation = m_bCheckDevAn;
}


void CTAB1::SetSlider() {
    
    if (m_layoutIndex != -1 && m_layoutIndex < localData->keyboardEffect.size()) {
        int speed = localData->keyboardEffect[m_layoutIndex].speed;

        // Validate and adjust the range of values
        if (speed < -5) {
            speed = -5;
        }
        else if (speed > 5) {
            speed = 5;
        }
        // Set the slider position
        m_sliderSpeed.SetPos(speed);
    }
    else {
        m_sliderSpeed.SetPos(1); // Default value
    }
}


// Handle slider value change
void CTAB1::OnSliderSpeedChange(NMHDR* pNMHDR, LRESULT* pResult) {
    
    if (m_layoutIndex != -1 && m_layoutIndex < localData->keyboardEffect.size()) {
        int speedValue = m_sliderSpeed.GetPos();
        localData->keyboardEffect[m_layoutIndex].speed = speedValue;
    }
    *pResult = 0;
}


void CTAB1::OnPaint() {// For slider scale
    
    CPaintDC dc(this); // Context for painting
    // Get slider size
    CRect sliderRect;
    m_sliderSpeed.GetClientRect(&sliderRect);
    m_sliderSpeed.ClientToScreen(&sliderRect);
    ScreenToClient(&sliderRect);

    // Scale settings
    int min = -5;                   // Minimum slider value
    int max = 5;                    // Maximum slider value
    int tickHeight = 3;             // Vertical tick height
    int numberOffset = 3;           // Number offset below the scale
    int tickWidth = 1;              // Line thickness
    int leftMargin = 13;            // Left margin
    int rightMargin = 13;           // Right margin
    int usableWidth = sliderRect.Width() - leftMargin - rightMargin;

    // Configure line color and style
    CPen pen(PS_SOLID, tickWidth, RGB(0, 0, 0));
    CPen* oldPen = dc.SelectObject(&pen);

    // Configure font for numbers
    CFont font;
    font.CreatePointFont(80, _T("Arial")); // Шрифт Arial, размер 8
    CFont* oldFont = dc.SelectObject(&font);

    // Draw the scale
    for (int i = min; i <= max; ++i) {
        int x = sliderRect.left + leftMargin + (usableWidth * (i - min)) / (max - min);
        int yTop = sliderRect.bottom + 3;         // Bottom point
        int yBottom = yTop + tickHeight;         // Top point

        dc.MoveTo(x, yTop);
        dc.LineTo(x, yBottom);

        // Draw numbers only for odd values
        if (i % 2 != 0) {
            CString number;
            number.Format(_T("%d"), i);
            dc.TextOut(x - 5, yBottom + numberOffset, number);
        }
    }

    // Restore the old style and pen
    dc.SelectObject(oldFont);
    dc.SelectObject(oldPen);

    // Call the base method if needed
    CDialogEx::OnPaint();
}


LRESULT CTAB1::OnTabUpdate(WPARAM wParam, LPARAM lParam) {
    UpdateDataFromLocal();
    Invalidate();
    return 0;
}


void CTAB1::UpdateDataFromLocal() {
    if (localData != nullptr) {

        SynchronizeKeyboardEffects();
        OnCbnSelchangeComboLang();
        m_bCheckDevAn = localData->devicesAnimation;

        UpdateData(FALSE);
    }
}



