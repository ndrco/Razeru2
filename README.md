# Razeru 2

Razeru 2 is a Windows tray application that visualizes the active keyboard
input language on a Razer Huntsman V2 Tenkeyless and the logo of a Razer Viper
mouse. It reads the existing `.chroma` animations itself and sends lighting
frames straight to both devices through the standard Windows HID stack.

No Razer application, service, SDK runtime, filter driver, or animation DLL is
required. Tested USB profiles are Huntsman V2 TKL (`1532:026B`) and Viper
(`1532:0078`).

Russian documentation: [README.ru.md](README.ru.md)

## Requirements

- Windows 10 or Windows 11, x64
- Razer Huntsman V2 Tenkeyless (`1532:026B`) connected over USB
- Optional Razer Viper (`1532:0078`) for mouse-logo indication
- The standard Windows `HidUsb`, `kbdhid`, and `mouhid` drivers
- For development: Visual Studio 2022 Build Tools with MSVC, ATL/MFC and a
  Windows SDK

## Install and run

Download `Razeru-Setup-<version>-x64.exe`, install it for the current user, and
start Razeru 2. The notification-area menu opens Settings, the official Razer
web editor, or exits the application. Existing `Razeru.json` and `Animations`
are preserved during an upgrade or uninstall.

Keep installed Razer software until direct HID control has been verified on
your keyboard. The safe validation and later removal order is documented in
[Razer software requirements and removal](docs/RAZER_REQUIREMENTS.md).

## Build and test

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

The application artifacts are written to `x64\Release`. The independent file
reader test validates every bundled keyboard and mouse animation:

```powershell
.\x64\Release\ChromaFileReaderTests.exe .\Animations
```

The diagnostic utility in `tools/RazeruHidTest.cpp` can query firmware and send
temporary static, frame, or spectrum effects on the keyboard and mouse without
writing device storage.

Build the per-user Windows installer with:

```powershell
.\installer\build-installer.ps1
```

## Documentation

- [English user guide](docs/README.md)
- [Russian user guide](docs/README.ru.md)
- [English development guide](docs/DEVELOPMENT.md)
- [Russian development guide](docs/DEVELOPMENT.ru.md)
- [Razer software requirements and removal](docs/RAZER_REQUIREMENTS.md)
- [ПО Razer: требования и удаление](docs/RAZER_REQUIREMENTS.ru.md)
- [Razeru 2.0.0.1 release notes](docs/RELEASE_NOTES_2.0.0.1.md)
- [Описание выпуска 2.0.0.1](docs/RELEASE_NOTES_2.0.0.1.ru.md)
- [Razeru 2.0.0.0 release notes](docs/RELEASE_NOTES_2.0.0.md)
- [Описание выпуска 2.0.0.0](docs/RELEASE_NOTES_2.0.0.ru.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)

## License

Razeru is distributed under GNU GPL v3. See [LICENSE](LICENSE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
