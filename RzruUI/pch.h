// pch.h: это предварительно скомпилированный заголовочный файл.
// Перечисленные ниже файлы компилируются только один раз, что ускоряет последующие сборки.
// Это также влияет на работу IntelliSense, включая многие функции просмотра и завершения кода.
// Однако изменение любого из приведенных здесь файлов между операциями сборки приведет к повторной компиляции всех(!) этих файлов.
// Не добавляйте сюда файлы, которые планируете часто изменять, так как в этом случае выигрыша в производительности не будет.

#ifndef PCH_H
#define PCH_H

// Добавьте сюда заголовочные файлы для предварительной компиляции
#include "framework.h"
#include "afxdialogex.h" //Для CDialogEx
#include <afxcmn.h> // Для CTabCtrl
#include "afxtabctrl.h"  // Для CMFCTabCtrl
#include <afxvisualmanagerwindows.h> // For Color Button

//#include <afxtoolbarimages.h>
//#include <afxmenuimages.h>
//#include <afxwin.h>
//#include <afxext.h>
//#include <afxdisp.h>
//#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
//#include <afxcmn.h>         // MFC support for Windows Common Controls
//#include <afxcontrolbars.h> // MFC support for ribbons and control bars (Feature Pack)

#include "afxcolorbutton.h" // For Color Button
#include <Windows.h>        // Для использования HWND
#include <fstream> // Для работы с JSON
#include <sstream> // Для разбора строковых значений конфигурации
#include <nlohmann/json.hpp> // Для работы с JSON
#include <gdiplus.h>


#endif //PCH_H
