# Razeru 2

Razeru 2 — приложение для области уведомлений Windows, которое показывает
активный язык ввода на клавиатуре Razer Huntsman V2 Tenkeyless. Оно само читает
существующие анимации `.chroma` и передаёт кадры подсветки прямо клавиатуре
через стандартный HID-стек Windows.

Приложения, службы, среда SDK, фильтрующие драйверы и библиотека анимаций Razer
не требуются. Первый аппаратный профиль поддерживает только USB-устройство
`VID_1532&PID_026B`.

English documentation: [README.md](README.md)

## Требования

- Windows 10 или Windows 11 x64;
- подключённая по USB Razer Huntsman V2 Tenkeyless (`1532:026B`);
- штатные драйверы Windows `HidUsb` и `kbdhid`;
- для разработки: Visual Studio 2022 Build Tools с MSVC, ATL/MFC и Windows SDK.

## Установка и запуск

Скачайте `Razeru-Setup-<версия>-x64.exe`, установите для текущего пользователя
и запустите Razeru 2. Через меню значка в области уведомлений можно открыть
настройки, официальный веб-редактор Razer или завершить работу. При обновлении
и удалении сохраняются `Razeru.json` и каталог `Animations`.

Не удаляйте установленное ПО Razer, пока прямое HID-управление не проверено на
вашей клавиатуре. Безопасная последовательность проверки и последующего
удаления описана в документе
[«ПО Razer: требования и удаление»](docs/RAZER_REQUIREMENTS.ru.md).

## Сборка и тестирование

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

Результат появится в `x64\Release`. Отдельный тест загрузчика проверяет все
входящие в проект клавиатурные анимации:

```powershell
.\x64\Release\ChromaFileReaderTests.exe .\Animations
```

Диагностическая утилита `tools/RazeruHidTest.cpp` умеет читать версию прошивки
и временно включать статический цвет, пользовательский кадр или спектр. В
постоянную память клавиатуры она ничего не записывает.

Сборка пользовательского инсталлятора:

```powershell
.\installer\build-installer.ps1
```

## Документация

- [Руководство пользователя](docs/README.ru.md)
- [User guide in English](docs/README.md)
- [Руководство разработчика](docs/DEVELOPMENT.ru.md)
- [Development guide in English](docs/DEVELOPMENT.md)
- [ПО Razer: требования и удаление](docs/RAZER_REQUIREMENTS.ru.md)
- [Razer software requirements and removal](docs/RAZER_REQUIREMENTS.md)
- [Уведомления о сторонних компонентах](THIRD_PARTY_NOTICES.md)

## Лицензия

Проект распространяется по GNU GPL v3. См. [LICENSE](LICENSE) и
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
