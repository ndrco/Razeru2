# Razeru 2

Razeru 2 — приложение для области уведомлений Windows, которое показывает
активный язык ввода на клавиатуре Razer Huntsman V2 Tenkeyless и логотипе мыши
Razer Viper. Оно само читает существующие анимации `.chroma` и передаёт кадры
подсветки устройствам через стандартный HID-стек Windows.

Приложения, службы, среда SDK, фильтрующие драйверы и библиотека анимаций Razer
не требуются. Проверены USB-профили Huntsman V2 TKL (`1532:026B`) и Viper
(`1532:0078`).

English documentation: [README.md](README.md)

## Требования

- Windows 10 или Windows 11 x64;
- подключённая по USB Razer Huntsman V2 Tenkeyless (`1532:026B`);
- необязательно: Razer Viper (`1532:0078`) для индикации на логотипе мыши;
- штатные драйверы Windows `HidUsb`, `kbdhid` и `mouhid`;
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
входящие в проект анимации клавиатуры и мыши:

```powershell
.\x64\Release\ChromaFileReaderTests.exe .\Animations
```

Диагностическая утилита `tools/RazeruHidTest.cpp` умеет читать версии прошивок
и временно включать статический цвет, пользовательский кадр или спектр на
клавиатуре и мыши. В постоянную память устройств она ничего не записывает.

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
- [Описание выпуска 2.0.0.2](docs/RELEASE_NOTES_2.0.0.2.ru.md)
- [Razeru 2.0.0.2 release notes](docs/RELEASE_NOTES_2.0.0.2.md)
- [Описание выпуска 2.0.0.1](docs/RELEASE_NOTES_2.0.0.1.ru.md)
- [Razeru 2.0.0.1 release notes](docs/RELEASE_NOTES_2.0.0.1.md)
- [Описание выпуска 2.0.0.0](docs/RELEASE_NOTES_2.0.0.ru.md)
- [Razeru 2.0.0.0 release notes](docs/RELEASE_NOTES_2.0.0.md)
- [Уведомления о сторонних компонентах](THIRD_PARTY_NOTICES.md)

## Лицензия

Проект распространяется по GNU GPL v3. См. [LICENSE](LICENSE) и
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
