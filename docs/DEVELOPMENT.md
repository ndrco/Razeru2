# Razeru development guide

## Repository layout

| Path | Purpose |
| --- | --- |
| `main.cpp`, `resource.*` | Win32 tray process, layout polling, keyboard hook and lifecycle |
| `ChromaPlaying.*` | Chroma scene selection, playback, compositing and configuration bridge |
| `Razer/` | Imported Razer Chroma API wrapper and signature verification |
| `RzruUI/` | MFC configuration DLL, dialogs, resources and JSON persistence |
| `Animations/` | Bundled `.chroma` assets |
| `third_party/` | Vendored header-only dependencies and their licenses |
| `config/` | Default configuration packaged by the installer |
| `scripts/` | Reproducible local build entry points |
| `installer/` | Inno Setup source and packaging script |
| `.vscode/` | Recommended extensions, encoding rules and build tasks |

The Visual Studio solution produces two first-party binaries:

- `Razeru.exe` owns the hidden message window, tray icon, timer, global keyboard
  hook and animation worker thread.
- `RzruUI.dll` owns the settings UI and reads/writes `Razeru.json`.

They exchange `ChromaPlaying::ConfigData` through exported DLL functions. A
registered Windows message (`WM_CONFIG_CHANGED`) tells the process to pause,
reload configuration and resume. The executable calls Razer's signed
`CChromaEditorLibrary64.dll` animation bridge through the API wrapper. This is
not the system SDK core; current installations provide `RzChromatic64.dll`
through the separate Razer Chroma App.

## Toolchain

The supported local toolchain is:

- Visual Studio Build Tools 2022, v143 MSVC x86/x64 tools;
- Windows 10/11 SDK;
- ATL and MFC for the selected architecture;
- PowerShell 5.1 or newer;
- Inno Setup 6.7 or newer for packaging;
- Git and optional Visual Studio Code.

Open the repository folder in VS Code. Accept the recommendations in
`.vscode/extensions.json`; tasks are available through **Terminal → Run Task**.
Some legacy C++/resource sources use a Windows code page, so
`files.autoGuessEncoding` is enabled. Do not perform an unreviewed bulk encoding
conversion because it can corrupt localized resources.

The nlohmann/json 3.12.0 single header is vendored under `third_party` to keep
builds independent of machine-specific include paths.

## Build configurations

The solution has `Debug`, `Release`, `Debug_NoSignatureCheck`, and
`Release_NoSignatureCheck` configurations for x86 (project platform `Win32`) and x64. Normal builds
validate the Authenticode signature of the Razer runtime. `*_NoSignatureCheck`
must be limited to controlled debugging and must not be distributed.

The supported release target and installer payload are x64:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

Examples:

```powershell
.\scripts\build.ps1 -Configuration Debug -Platform x64
.\scripts\build.ps1 -Configuration Release -Platform x86
.\scripts\build.ps1 -Configuration Release -Platform x64 -Analyze
```

The script locates MSBuild through `vswhere.exe`; it does not rely on a Visual
Studio developer prompt. Without `-NoRebuild`, it performs `Rebuild`. Build
outputs are under `<platform>\<configuration>` for x64 and `<configuration>` for
x86. `Win32` is accepted as an x86 alias. The post-build step copies only the
Razer DLL matching the architecture.

## Debugging

For debugger startup, set the working directory to the build output containing
the executable, `RzruUI.dll`, the matching Razer DLL, `Razeru.json`, and the
`Animations` directory. Normal startup requires the standalone Razer Chroma App
with Chroma Apps enabled and a compatible keyboard. Synapse is optional and is
not a test prerequisite. Chroma initialization cannot be fully exercised without
the external SDK core; physical effects require connected hardware.

Useful areas to instrument:

- `CheckActiveLayout` / `GetActiveLayout` for layout selection;
- `KeyboardHookProc` and `EventProcessingThread` for key effects;
- `ChromaPlaying::SetActiveSceneEffect` and `PlayingFrameKeyboard` for playback;
- the exported functions in `RzruUI.cpp` for configuration exchange.

The low-level hook callback must remain fast and non-blocking. Keep shared state
behind its mutex or atomics, and avoid filesystem or Chroma calls from the hook.

## Installer

Build the complete per-user x64 setup with:

```powershell
.\installer\build-installer.ps1 -AppVersion 1.2.0.4
```

The packaging script:

1. rebuilds `Release|x64` unless `-SkipBuild` is supplied;
2. verifies the required payload;
3. downloads the pinned Microsoft VC++ x64 Redistributable when absent;
4. checks its SHA-256 and Microsoft Authenticode signature;
5. verifies the bundled Razer DLL is signed by Razer USA Ltd.;
6. invokes Inno Setup and prints the setup SHA-256.

The Microsoft redistributable URL is mutable. When Microsoft publishes a newer
file, deliberately update the pinned digest and minimum runtime build together,
then validate a clean install. Do not simply bypass the hash check.

The Razer Chroma App is an external interactive prerequisite and is not
redistributed. Setup detects `RzChromatic64.dll` (and the legacy
`RzChromaSDK64.dll`), opens Razer's official download when neither exists, and
does not launch Razeru prematurely.

At the product level, Chroma App is the minimum Razer dependency. Its App
Engine/Common host, SDK services, and device drivers are vendor-managed parts of
that product and must not be removed individually. Synapse, Axon, Cortex, and
THX Spatial Audio are not required. See `docs/RAZER_REQUIREMENTS.md` and test a
release both with no Synapse installed and with the target keyboard connected.

Production releases should sign `Razeru.exe`, `RzruUI.dll`, the setup and the
uninstaller with an organization-controlled code-signing certificate. Pass a
configured Inno Setup sign-tool name through `-SignToolName`.

## Release checklist

1. Review `git diff`, dependency licenses and version metadata.
2. Build and analyze `Release|x64`; ensure warnings are understood.
3. Test layout changes, settings, lock/unlock and startup on real Razer hardware.
4. Build the installer and verify signatures and SHA-256.
5. Smoke-test install, upgrade preserving settings, and uninstall on Windows 10
   and Windows 11 under a non-administrator account.
6. Sign all first-party PE files and publish the checksum with the release.
7. Tag the exact source revision used for the binaries.

There is currently no automated unit/integration test suite or CI workflow; add
both before accepting broad refactors or unattended releases.
