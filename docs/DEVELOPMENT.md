# Razeru 2 development guide

## Architecture

| Component | Responsibility |
| --- | --- |
| `main.cpp` | Hidden window, tray icon, layout polling and keyboard hook |
| `ChromaPlaying.*` | Animation timing, overlays and application lifecycle |
| `ChromaFileReader.*` | Bounds-checked version-1 `.chroma` keyboard reader |
| `RazerHidDevice.*` | SetupAPI discovery and Razer HID feature reports |
| `RzruUI/` | MFC settings DLL and `Razeru.json` editing |
| `tools/RazeruHidTest.cpp` | Standalone read/write diagnostic utility |
| `tests/ChromaFileReaderTests.cpp` | Animation corpus and malformed-input test |

The executable no longer links or dynamically loads the Razer Chroma API or
`CChromaEditorLibrary64.dll`. Its runtime imports are limited to Windows APIs
and the Visual C++ runtime. The older sources under `Razer/` remain in the
repository for types, history and license provenance but are excluded from the
Razeru 2 build.

## HID profile

`RazerHidDevice` enumerates the HID class and accepts only:

- vendor `0x1532`, product `0x026B`;
- composite interface `MI_03`;
- usage page `0x000C`, usage `0x0001`.

The 91-byte Windows feature buffer contains a report ID followed by the 90-byte
Razer report. Razeru validates response status, remaining packets, command class
and command ID, and calculates the XOR checksum over payload bytes 3 through
88. Firmware queries use transaction ID `0x3F` with a read-only `0x1F`
fallback. Lighting uses transaction ID `0x3F` and volatile storage mode.

Custom frames are converted from the logical 6x22 matrix to the tested 6x17
Huntsman V2 TKL hardware matrix. Logical columns 1-17 map to hardware columns
0-16 because column zero is padding in the Chroma grid. Rows are sent before
enabling custom mode. Cleanup restores the firmware spectrum effect.

## Animation compatibility

`.chroma` version 1 is read without the Razer animation DLL. The reader checks
the device type, frame count, exact file length, finite durations and every
read. It supports standard Keyboard 6x22 and Keyboard Extended 8x24. For the
extended format it intentionally reproduces Razeru 1's
`PluginGetFrameName(..., length=132)` behavior: the first 132 row-major colors
are used and the rest are discarded.

## Build and tests

Use a Visual Studio 2022 installation containing MSVC, ATL/MFC and a Windows
SDK:

```powershell
.\scripts\build.ps1 -Configuration Release -Platform x64
```

The main executable links `hid.lib` and `setupapi.lib`. No Razer binary is
copied by the project post-build step. Validate the complete animation corpus
and malformed input with `ChromaFileReaderTests` and use `RazeruHidTest` for
hardware tests. Hardware commands are temporary and should finish by restoring
`spectrum`.

A release gate should include:

1. clean Release x64 build and dependency inspection;
2. all bundled keyboard animations accepted and malformed input rejected;
3. firmware query plus red/green/blue/custom/spectrum device tests;
4. layout changes, overlays, settings reload, lock/unlock and exit on hardware;
5. a run with Razer services stopped and then with Razer software absent;
6. installer build and upgrade from Razeru 1, confirming removal of stale
   `CChromaEditorLibrary*.dll` from the application directory.

## Installer

`installer/Razeru.iss` retains the existing AppId, install path and mutex so it
upgrades Razeru 1 in place. It packages only Razeru binaries, configuration,
animations, documentation, license and the signed Microsoft Visual C++ x64
redistributable. It neither detects nor offers Razer software.

Production executables and the setup should be Authenticode-signed. Never add a
generic device profile without validating the exact product ID, interface,
matrix geometry, commands and teardown behavior on real hardware.
