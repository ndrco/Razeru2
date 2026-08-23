# Razeru

Razeru is a Windows tray application that visualizes the active keyboard input
language on Razer Chroma-compatible devices. It selects a `.chroma` animation
for the foreground window's keyboard layout and can layer static key lighting
and reactive key-press effects over the animation.

Russian documentation: [README.ru.md](README.ru.md)

## Requirements

- Windows 10 or Windows 11, x64
- A physically connected Razer Chroma-compatible keyboard
- Razer Chroma App running with Chroma Apps enabled; Razer Synapse is optional
- For development: Visual Studio 2022 Build Tools with MSVC, ATL/MFC, Windows
  SDK and CMake; Git; VS Code is optional

## Install and run

Download `Razeru-Setup-<version>-x64.exe` from a release, run it, and start
Razeru from the Start menu. The installer uses a per-user installation and does
not require administrator privileges. The application runs in the notification
area. Right-click its icon to open Settings, launch the Chroma Editor, or exit.

The installer includes the signed Razer x64 animation bridge and Microsoft's
signed Visual C++ x64 Redistributable. The separate Razer Chroma App supplies
the system SDK core; if it is missing, Setup opens Razer's official download
page and postpones launching Razeru. Existing `Razeru.json` and `Animations`
files are preserved on upgrades and uninstall so customizations are not lost.

Install only Chroma App as the user-selectable Razer prerequisite. App Engine,
the Chroma SDK services, and device drivers are shared components installed by
Razer and must remain. Synapse, Axon, Cortex, and THX Spatial Audio are not
required by Razeru. See the [minimum Razer components](docs/RAZER_REQUIREMENTS.md)
for the exact supported configuration and verification steps.

## Build

From PowerShell:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

Run MSVC code analysis:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64 -Analyze
```

Build the Windows installer:

```powershell
.\installer\build-installer.ps1
```

The application artifacts are written to `x64\Release`; the installer is
written to `dist`.

## Documentation

- [English user guide](docs/README.md)
- [Russian user guide](docs/README.ru.md)
- [English development guide](docs/DEVELOPMENT.md)
- [Russian development guide](docs/DEVELOPMENT.ru.md)
- [English audit report](docs/AUDIT.md)
- [Russian audit report](docs/AUDIT.ru.md)
- [Minimum Razer components](docs/RAZER_REQUIREMENTS.md)
- [Минимальные компоненты Razer](docs/RAZER_REQUIREMENTS.ru.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)

## License

The project is distributed under GNU GPL v3. Imported Razer components retain
their MIT license; see [LICENSE](LICENSE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
