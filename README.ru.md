# Razeru

Razeru — приложение для области уведомлений Windows, которое визуально
показывает активный язык ввода на устройствах с поддержкой Razer Chroma. Для
раскладки активного окна выбирается соответствующая анимация `.chroma`; поверх
неё можно включить постоянную подсветку клавиш и реактивные эффекты нажатия.

English documentation: [README.md](README.md)

## Требования

- Windows 10 или Windows 11, x64;
- физически подключённая клавиатура с поддержкой Razer Chroma;
- запущенное приложение Razer Chroma с включённым модулем Chroma Apps;
  Razer Synapse необязателен;
- для разработки: Visual Studio 2022 Build Tools с MSVC, ATL/MFC, Windows SDK
  и CMake; Git; VS Code — по желанию.

## Установка и запуск

Скачайте `Razeru-Setup-<версия>-x64.exe` из релиза, запустите его и откройте
Razeru из меню «Пуск». Установка выполняется для текущего пользователя и не
требует прав администратора. После запуска приложение находится в области
уведомлений. Щёлкните значок правой кнопкой мыши, чтобы открыть настройки,
редактор Chroma или завершить работу.

Инсталлятор содержит подписанную x64-библиотеку анимаций Razer и подписанный
Microsoft пакет Visual C++ Redistributable x64. Системное ядро SDK предоставляет
отдельное приложение Razer Chroma: если его нет, Setup откроет официальную
страницу загрузки и не станет преждевременно запускать Razeru. При обновлении и
удалении `Razeru.json` и `Animations` сохраняются.

Из выбираемых пользователем продуктов Razer нужен только Chroma App. Общая
среда App Engine, службы Chroma SDK и драйверы устройств устанавливаются Razer
автоматически и должны остаться. Synapse, Axon, Cortex и THX Spatial Audio
Razeru не требуются. Точный состав и проверка описаны в разделе
[«Минимальные компоненты Razer»](docs/RAZER_REQUIREMENTS.ru.md).

## Сборка

В PowerShell:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

Запуск статического анализа MSVC:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64 -Analyze
```

Сборка Windows-инсталлятора:

```powershell
.\installer\build-installer.ps1
```

Бинарные файлы приложения появятся в `x64\Release`, инсталлятор — в `dist`.

## Документация

- [Руководство пользователя](docs/README.ru.md)
- [User guide in English](docs/README.md)
- [Руководство разработчика](docs/DEVELOPMENT.ru.md)
- [Development guide in English](docs/DEVELOPMENT.md)
- [Отчёт аудита](docs/AUDIT.ru.md)
- [Audit report in English](docs/AUDIT.md)
- [Минимальные компоненты Razer](docs/RAZER_REQUIREMENTS.ru.md)
- [Minimum Razer components](docs/RAZER_REQUIREMENTS.md)
- [Уведомления о сторонних компонентах](THIRD_PARTY_NOTICES.md)

## Лицензия

Проект распространяется по GNU GPL v3. Импортированные компоненты Razer
сохраняют лицензию MIT. См. [LICENSE](LICENSE) и
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
