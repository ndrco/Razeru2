#pragma once
#include "pch.h"
#include "RzruUI.h"
#include "CTAB3.h"


// Dialog box for CTAB3
BEGIN_MESSAGE_MAP(CTAB3, CDialogEx)
    ON_CBN_SELCHANGE(IDC_COMBO_KEY, &CTAB3::OnCbnSelchangeComboMode)
    ON_EN_UPDATE(IDC_EDIT_FR_DUR, &CTAB3::OnEnUpdateEditNumber)
    ON_EN_UPDATE(IDC_EDIT_FRAMES, &CTAB3::OnEnUpdateEditNumber)
    ON_BN_CLICKED(IDC_MFCCOLORBUTTON2, &CTAB3::OnBnClickedColorButton)
    ON_MESSAGE(WM_TAB_UPDATE, &CTAB3::OnTabUpdate)
END_MESSAGE_MAP()
IMPLEMENT_DYNAMIC(CTAB3, CDialogEx)

CTAB3::CTAB3(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_TAB_3, pParent) {
}


CTAB3::~CTAB3() {
}


void CTAB3::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MFCCOLORBUTTON2, m_ColorButton);
}


BOOL CTAB3::OnInitDialog()
{
    CDialogEx::OnInitDialog(); // Call the base method
    
    // Setting texts
    UINT controlIDs[] = { IDC_STATIC3_1, IDC_STATIC3_2, IDC_STATIC3_3, IDC_STATIC3_4,
                          IDC_STATIC3_5, IDC_STATIC3_6, IDC_STATIC3_7, IDC_STATIC3_8 };
    UINT stringIDs[] = { IDS_TAB3_1, IDS_TAB3_2, IDS_TAB3_3, IDS_TAB3_4,
                         IDS_TAB3_5, IDS_TAB3_6, IDS_TAB3_7, IDS_TAB3_8 };
    for (int i = 0; i < _countof(controlIDs); ++i) {
        // Load string from the String Table
        CString strText;
        if (strText.LoadString(stringIDs[i])) {
            // Set text in the static control
            CStatic* pStatic = (CStatic*)GetDlgItem(controlIDs[i]);
            if (pStatic != nullptr) {
                pStatic->SetWindowText(strText);
            }
        }
    }

    // Initializing button
    m_ColorButton.EnableOtherButton(_T("Other"), FALSE, TRUE);

    // Initializing CComboBox
    CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEY);
    if (pComboBox) {
        // Add items to the combobox
        int index = pComboBox->AddString(_T("-----"));
        if (index != CB_ERR)
            pComboBox->SetItemData(index, EFFECT_TYPE::CHROMA_NONE);

        index = pComboBox->AddString(_T("Wave"));
        if (index != CB_ERR)
            pComboBox->SetItemData(index, EFFECT_TYPE::CHROMA_WAVE);

        index = pComboBox->AddString(_T("Reactive"));
        if (index != CB_ERR)
            pComboBox->SetItemData(index, EFFECT_TYPE::CHROMA_REACTIVE);
    }

    UpdateDataFromLocal();
    return TRUE; // Return TRUE if the focus does not change manually
}


void CTAB3::OnEnUpdateEditNumber() {
    
    // Get a pointer to the control that triggered the event
    CWnd* pWnd = GetFocus();
    if (!pWnd)
        return;
    
    int controlID = pWnd->GetDlgCtrlID();

    // Get text from the Edit Control
    CString strValue;
    pWnd->GetWindowText(strValue);

    // Allow empty text
    if (strValue.IsEmpty())
        return;

    // Convert text to a number
    int number = _ttoi(strValue);

    // Restrict numbers outside the range
    if (number > 999)
    {
        // Remove the last character if the number is too large
        strValue = strValue.Left(strValue.GetLength() - 1);
        GetDlgItem(IDC_EDIT_FR_DUR)->SetWindowText(strValue);

        // Set the cursor at the end of the text
        CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FR_DUR);
        if (pEdit)
        {
            pEdit->SetSel(strValue.GetLength(), strValue.GetLength());
        }
        return;
    }
    if (localData != nullptr) {
        // Update the corresponding variable
        if (controlID == IDC_EDIT_FR_DUR) {
            if (number >= 10) {
                localData->keyEffect.frameDuration = number;
            }
        }
        else if (controlID == IDC_EDIT_FRAMES) {
            if (number >= 1) { // Minimum value for frames = 1
                localData->keyEffect.framesOfKeyAnimation = number;
            }
        }
    }
}

void CTAB3::ShowFrameDuration() {
    // Convert the current value to a string
    CString strValue;
    strValue.Format(_T("%d"), localData->keyEffect.frameDuration);
    // Set the text in the edit field
    GetDlgItem(IDC_EDIT_FR_DUR)->SetWindowText(strValue);
}

void CTAB3::ShowFrames() {
    // Convert the current value to a string
    CString strValue;
    strValue.Format(_T("%d"), localData->keyEffect.framesOfKeyAnimation);
    // Set the text in the edit field
    GetDlgItem(IDC_EDIT_FRAMES)->SetWindowText(strValue);
}

void CTAB3::OnBnClickedColorButton() {
    m_CurrentColor = m_ColorButton.GetColor();
    if (localData != nullptr) {
        localData->keyEffect.primaryColor = m_CurrentColor;
    }
}

void CTAB3::OnCbnSelchangeComboMode() {
    // Get a pointer to the combobox
    CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEY);
    if (pComboBox) {
        // Get the index of the selected item
        int selectedIndex = pComboBox->GetCurSel();
        if (selectedIndex != CB_ERR) {
            // Get the value associated with the item
            EFFECT_TYPE selectedEffect = (EFFECT_TYPE)pComboBox->GetItemData(selectedIndex);

            // Update the value in localData
            if (localData != nullptr) {
                localData->keyEffect.name = selectedEffect;
            }
        }
    }
}


LRESULT CTAB3::OnTabUpdate(WPARAM wParam, LPARAM lParam) {
    UpdateDataFromLocal(); // Reload data from localData
    Invalidate();          // Redraw the interface
    return 0;
}


void CTAB3::UpdateDataFromLocal() {
    if (localData != nullptr) {
        
        m_ColorButton.SetColor(localData->keyEffect.primaryColor);
        m_CurrentColor = m_ColorButton.GetColor();

        ShowFrameDuration();
        ShowFrames();

        int modeValue = static_cast<int>(localData->keyEffect.name);
        CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_KEY);
        for (int i = 0; i < pComboBox->GetCount(); ++i) {
            if (pComboBox->GetItemData(i) == modeValue) {
                pComboBox->SetCurSel(i); // Set the selected item
                break;
            }
        }
        OnCbnSelchangeComboMode();
        

        double result = static_cast<double>(localData->keyEffect.frameDuration) *
            static_cast<double>(localData->keyEffect.framesOfKeyAnimation) / 1000.0;
        // Convert to a string with two decimal places
        CString strValue;
        strValue.Format(_T("%.2f"), result);
        // Set the value in the Edit Control
        GetDlgItem(IDC_EDIT_RESULT)->SetWindowText(strValue);

        UpdateData(FALSE); // Synchronize the interface
    }
}
