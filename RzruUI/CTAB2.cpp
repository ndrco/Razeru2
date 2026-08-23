#pragma once
#include "pch.h"
#include "RzruUI.h"
#include "CTAB2.h"


// Dialog window CTAB2
BEGIN_MESSAGE_MAP(CTAB2, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_MODE, &CTAB2::OnCbnSelchangeComboMode)
    ON_CBN_SELCHANGE(IDC_COMBO_BLEND, &CTAB2::OnCbnSelchangeComboBlend)
    ON_BN_CLICKED(IDC_CHECK_BACK_ON, &CTAB2::OnBnClickedBackOn)
    ON_BN_CLICKED(IDC_MFCCOLORBUTTON1, &CTAB2::OnBnClickedColorButton)
    ON_BN_CLICKED(IDC_BUTTON_CLN, &CTAB2::OnBnClickedClean)
    ON_WM_LBUTTONDOWN()
    ON_WM_PAINT()
    ON_MESSAGE(WM_TAB_UPDATE, &CTAB2::OnTabUpdate)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CTAB2, CDialogEx)


CTAB2::CTAB2(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_TAB_2, pParent) {
    GdiplusStartupInput gdiplusStartupInput; // Initialize GDI+ for graphics operations
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}


CTAB2::~CTAB2() {
    GdiplusShutdown(gdiplusToken); // Shut down GDI+ to release resources
}


void CTAB2::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_MODE, m_comboMode);
    DDX_Control(pDX, IDC_COMBO_BLEND, m_comboBlend);
    DDX_Check(pDX, IDC_CHECK_BACK_ON, m_bCheckBclOn);
    DDX_Control(pDX, IDC_MFCCOLORBUTTON1, m_ColorButton);
    DDX_Control(pDX, IDC_STATIC_KEYBOARD, m_StaticKeyboard);
}


BOOL CTAB2::OnInitDialog() {
    CDialogEx::OnInitDialog();
   
    // Setting up text for UI elements
    UINT controlIDs[] = { IDC_STATIC2_1, IDC_STATIC2_2, IDC_STATIC2_3,
                          IDC_CHECK_BACK_ON, IDC_BUTTON_CLN };
    UINT stringIDs[] = { IDS_TAB2_COLOR, IDS_TAB2_MODE, IDS_TAB2_BLEND,
                         IDS_TAB2_BKL_ON, IDS_TAB2_CLN };
    for (int i = 0; i < _countof(controlIDs); ++i) {
        // Load a string from the String Table
        CString strText;
        if (strText.LoadString(stringIDs[i])) {
            // Set text for the static control
            CStatic* pStatic = (CStatic*)GetDlgItem(controlIDs[i]);
            if (pStatic != nullptr) {
                pStatic->SetWindowText(strText);
            }
        }
    }
    
    // Load the keyboard image
    m_bmpKeyboard.LoadBitmap(IDB_KEYBOARD_IMAGE_EXTENDED);
    
    GetKeyboardKeys(m_keyboardKeys); // Load keyboard keys layout

    // Initialize the color button with default settings
    m_ColorButton.EnableOtherButton(_T("Other"), FALSE, TRUE);

        
    // Initialize CComboBox m_comboBlend
    for (int blend = static_cast<int>(EChromaSDKSceneBlend::SB_None);
        blend <= static_cast<int>(EChromaSDKSceneBlend::SB_Lerp); ++blend) {
        if (blend == static_cast<int>(EChromaSDKSceneBlend::SB_Threshold))
            continue;
        EChromaSDKSceneBlend sceneBlend = static_cast<EChromaSDKSceneBlend>(blend);
        CString blendName = SceneBlendToString(sceneBlend);
        int index = m_comboBlend.AddString(blendName);
        m_comboBlend.SetItemData(index, blend);
    }


    // Initialize CComboBox m_comboMode
    for (int mode = static_cast<int>(EChromaSDKSceneMode::SM_Replace);
        mode <= static_cast<int>(EChromaSDKSceneMode::SM_Subtract); ++mode) {
        EChromaSDKSceneMode sceneMode = static_cast<EChromaSDKSceneMode>(mode);
        CString modeName = SceneModeToString(sceneMode);
        int index = m_comboMode.AddString(modeName);
        m_comboMode.SetItemData(index, mode);
    }

    UpdateDataFromLocal(); // Synchronize local data with UI
    return TRUE;
}


void CTAB2::OnBnClickedBackOn() {
    // Update the variable from the interface
    UpdateData(TRUE);
    // Bind the value to localData->devicesAnimation
    localData->backlightEffect.backLightOn = m_bCheckBclOn;
}

void CTAB2::OnBnClickedColorButton() {
     m_CurrentColor = m_ColorButton.GetColor(); // Get the currently selected color
}


void CTAB2::OnLButtonDown(UINT nFlags, CPoint point) {
    // Calculate the offset (coordinates of the static element)
    CStatic* pStaticKeyboard = (CStatic*)GetDlgItem(IDC_STATIC_KEYBOARD);
    if (pStaticKeyboard) {
        CRect rectStatic;
        pStaticKeyboard->GetWindowRect(&rectStatic);
        ScreenToClient(&rectStatic);

        // Adjust the click coordinates considering the initial image offset
        CPoint pointForKeys = point;
        pointForKeys.Offset(-rectStatic.left, -rectStatic.top);
        pointForKeys.Offset(-m_offsetX, -m_offsetY); // Account for image offset

        // Check if the click falls within a key
        for (const auto& key : m_keyboardKeys) {
            if (key.rect.PtInRect(pointForKeys)) {
                int keyN = key.row * maxCol + key.col;

                if (keyN >= 0 && keyN < localData->backlightEffect.colorsKeyboard.size()) {
                    localData->backlightEffect.colorsKeyboard[keyN] = m_CurrentColor;
                }
                break;
            }
        }
    }

    Invalidate(); // Request window redraw
    UpdateWindow(); // Ensure immediate rendering
    CDialogEx::OnLButtonDown(nFlags, point);
}



void CTAB2::OnPaint() {
    CPaintDC dc(this); // Device context for painting

    // Get the client area size
    CRect rect;
    GetClientRect(&rect);

    // Create a GDI+ buffer
    Gdiplus::Bitmap bitmap(rect.Width(), rect.Height(), PixelFormat32bppARGB);
    Gdiplus::Graphics graphics(&bitmap);

    // Clear the buffer with the background color
    graphics.Clear(Gdiplus::Color(GetSysColor(COLOR_BTNFACE)));

    // Get the client area of the static control
    CRect rectStatic;
    m_StaticKeyboard.GetWindowRect(&rectStatic);
    ScreenToClient(&rectStatic);

    // Load the keyboard image
    HBITMAP hBmp = (HBITMAP)m_bmpKeyboard.GetSafeHandle();
    Bitmap bmpImage(hBmp, NULL);

    // Determine the dimensions of the original image
    UINT imageWidth = bmpImage.GetWidth();
    UINT imageHeight = bmpImage.GetHeight();

    // Center the image within rectStatic
    m_offsetX = rectStatic.left + (rectStatic.Width() - imageWidth) / 2;
    m_offsetY = rectStatic.top + (rectStatic.Height() - imageHeight) / 2;

    graphics.DrawImage(&bmpImage,
                        static_cast<INT>(m_offsetX),
                        static_cast<INT>(m_offsetY),
                        static_cast<INT>(imageWidth),
                        static_cast<INT>(imageHeight));

    // Draw the keyboard backlight
    int margin = 10; // Increase the backlight halo size relative to the key size
    const int biasX = 5; // Offset to center the backlight
    const int biasY = 3;
    const int alfaStart = 160; // Starting alpha transparency
    const int alfaEnd = 10; // Ending alpha transparency

    for (const auto& it : m_keyboardKeys) {
        int keyN = it.row * maxCol + it.col;
        int value = localData->backlightEffect.colorsKeyboard[keyN];

        if (value > 0) {
            CRect highlightRect = it.rect;

            // Offset relative to the keyboard image origin
            highlightRect.OffsetRect(m_offsetX, m_offsetY);

            // Convert to GDI+ coordinates
            Gdiplus::Rect gradientRect(highlightRect.left - margin + biasX,
                                       highlightRect.top - margin + biasY,
                                       highlightRect.Width() + margin,
                                       highlightRect.Height() + margin);

            // Create a gradient brush for the backlight
            Gdiplus::GraphicsPath path;
            path.AddEllipse(gradientRect);

            Gdiplus::PathGradientBrush gradientBrush(&path);
            Gdiplus::Color centerColor(alfaStart, GetRValue(value), GetGValue(value), GetBValue(value));
            Gdiplus::Color surroundColor(alfaEnd, GetRValue(value), GetGValue(value), GetBValue(value));

            gradientBrush.SetCenterColor(centerColor);
            int count = 1;
            gradientBrush.SetSurroundColors(&surroundColor, &count);

            graphics.FillRectangle(&gradientBrush, gradientRect);
        }
    }
    // Copy the buffer content to the screen
    Gdiplus::Graphics screenGraphics(dc);
    screenGraphics.DrawImage(&bitmap, 0, 0);
}



CString CTAB2::SceneModeToString(EChromaSDKSceneMode mode) {
    switch (mode) {
    case EChromaSDKSceneMode::SM_Replace: return _T("Replace");
    case EChromaSDKSceneMode::SM_Max: return _T("Max");
    case EChromaSDKSceneMode::SM_Min: return _T("Min");
    case EChromaSDKSceneMode::SM_Average: return _T("Average");
    case EChromaSDKSceneMode::SM_Multiply: return _T("Multiply");
    case EChromaSDKSceneMode::SM_Add: return _T("Add");
    case EChromaSDKSceneMode::SM_Subtract: return _T("Subtract");
    default: return _T("Unknown");
    }
}


CString CTAB2::SceneBlendToString(EChromaSDKSceneBlend mode) {
    switch (mode) {
    case EChromaSDKSceneBlend::SB_Invert: return _T("Invert");
    case EChromaSDKSceneBlend::SB_Lerp: return _T("Average");
    case EChromaSDKSceneBlend::SB_None: return _T("None");
    default: return _T("Unknown");
    }
}

void CTAB2::OnCbnSelchangeComboBlend() {
    // Get the currently selected index
    int selectedIndex = m_comboBlend.GetCurSel();
    if (selectedIndex != CB_ERR) {
        // Retrieve the value associated with the selected item
        int blendValue = static_cast<int>(m_comboBlend.GetItemData(selectedIndex));

        // Update the value in localData->backlightEffect.blend
        if (localData != nullptr) {
            localData->backlightEffect.blend = static_cast<EChromaSDKSceneBlend>(blendValue);
        }
    }
}

void CTAB2::OnCbnSelchangeComboMode() {
    // Get the currently selected index
    int selectedIndex = m_comboMode.GetCurSel();
    if (selectedIndex != CB_ERR) {
        // Retrieve the value associated with the selected item
        int modeValue = static_cast<int>(m_comboMode.GetItemData(selectedIndex));

        // Update the value in localData->backlightEffect.mode
        if (localData != nullptr) {
            localData->backlightEffect.mode = static_cast<EChromaSDKSceneMode>(modeValue);
        }
    }
}


void CTAB2::OnBnClickedClean() {
    // Clear the keyboard backlight colors
    localData->backlightEffect.colorsKeyboard.assign(localData->backlightEffect.colorsKeyboard.size(), 0);
    Invalidate(); // Request window redraw
    UpdateWindow(); // Ensure immediate rendering
}


LRESULT CTAB2::OnTabUpdate(WPARAM wParam, LPARAM lParam) {
    UpdateDataFromLocal(); // Reload data from localData
    Invalidate();          // Request interface redraw
    return 0;
}


void CTAB2::UpdateDataFromLocal() {
    if (localData != nullptr) {
        // Set the initial color in the color button
        m_ColorButton.SetColor(RGB(0, 255, 0)); // Default color: green
        m_CurrentColor = m_ColorButton.GetColor();

        // Set the current blend value according to localData->backlightEffect.blend
        int blendValue = static_cast<int>(localData->backlightEffect.blend);
        // Find the index of the item with the corresponding value
        for (int i = 0; i < m_comboBlend.GetCount(); ++i) {
            if (m_comboBlend.GetItemData(i) == blendValue) {
                m_comboBlend.SetCurSel(i); // Set the selected item
                break;
            }
        }
        // Trigger OnCbnSelchangeComboBlend() to synchronize state
        OnCbnSelchangeComboBlend();

        // Set the current mode value according to localData->backlightEffect.mode
        int modeValue = static_cast<int>(localData->backlightEffect.mode);
        // Find the index of the item with the corresponding value
        for (int i = 0; i < m_comboMode.GetCount(); ++i) {
            if (m_comboMode.GetItemData(i) == modeValue) {
                m_comboMode.SetCurSel(i); // Set the selected item
                break;
            }
        }
        // Trigger OnCbnSelchangeComboMode() to synchronize state
        OnCbnSelchangeComboMode();
        
        // Synchronize the backlight checkbox state with local data
        m_bCheckBclOn = localData->backlightEffect.backLightOn;

        UpdateData(FALSE); // Synchronize the interface with data
    }
}
