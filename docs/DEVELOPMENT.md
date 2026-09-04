# Razeru 2 development guide

## Architecture

| Component | Responsibility |
| --- | --- |
| `main.cpp` | Hidden window, tray icon, layout polling and keyboard hook |
| `ChromaPlaying.*` | Animation timing, overlays and application lifecycle |
| `ChromaFileReader.*` | Bounds-checked version-1 `.chroma` keyboard reader |
| `ChromaMouseFileReader.*` | Bounds-checked Mouse 9x7/logo-cell reader |
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

`RazerHidDevice` enumerates the HID class and accepts two exact profiles:

- Huntsman V2 TKL: `1532:026B`, `MI_03`, usage `000C:0001`;
- Viper: `1532:0078`, `MI_00`, usage `0001:0002`.

The 91-byte Windows feature buffer contains a report ID followed by the 90-byte
Razer report. Razeru validates response status, remaining packets, command class
and command ID, and calculates the XOR checksum over payload bytes 3 through
88. Firmware queries use transaction IDs `0x3F`/`0x1F` for the keyboard and
`0xFF`/`0x3F` for the Viper. Lighting uses transaction ID `0x3F` and volatile
storage mode.

Custom frames are converted from the logical 6x22 matrix to the tested 6x17
Huntsman V2 TKL hardware matrix. Logical columns 1-17 map to hardware columns
0-16 because column zero is padding in the Chroma grid. Rows are sent before
enabling custom mode. Cleanup restores the firmware spectrum effect.

The Viper has one lighting element, its logo. Razeru takes Chroma Mouse 9x7
coordinate `(7,3)` and sends it with extended static command `0F:02` for
`LOGO_LED=0x04` and `NOSTORE=0x00`. No frame is written to onboard profile
storage. Windows protects the top-level mouse collection from generic
read/write access, so the handle is opened without those flags while
`HidD_SetFeature`/`HidD_GetFeature` remain available.

## Animation compatibility

Starting with 2.0.0.3, the first lighting output on each new HID handle sets
brightness to 255 with `0F:04` and verifies it using `0F:84`. Both commands
use `NOSTORE=0`, `BACKLIGHT_LED=5` / `LOGO_LED=4`, and transaction IDs `1F`
for the keyboard / `3F` for the mouse. Closing the handle clears initialization
state. Verification failures close the handle so the next output retries.
Plain `Open`, `info`, and brightness queries do not change the lighting.

In 2.0.0.4, `startAll()` calls `ResumeLighting()` to invalidate initialization
for both devices before resuming output after `WTS_SESSION_UNLOCK`, even
when the HID handle remains open. Additionally, `EnsureBrightnessUnlocked()`
checks brightness during effect output at most once every 2 seconds using a
steady clock. Zero is replaced with 255; nonzero brightness is preserved
until the next initialization. Queries share the frame-output mutex; there
is no separate polling while output is paused.

Recovery regression: set temporary brightness to zero during animation and
verify that it returns to 255 after the next two-second interval. Repeat with
128: the periodic check must preserve it. Separately test lock/unlock with
zero brightness before resuming. A synthetic `WM_WTSSESSION_CHANGE` message
tests the application handler but does not replace a manual, real Windows
screen-lock test on the hardware.

For automated hardware regression, stop Razeru and other RGB controllers,
then run `RazeruHidTest brightness-recovery-test` and
`RazeruHidTest mouse-brightness-recovery-test`. They check first output,
periodic zero recovery, preservation of brightness 128, and resume
invalidation. These commands temporarily change lighting and finish at
brightness 255 with the spectrum effect; restart Razeru afterwards.

Diagnostics: `RazeruHidTest brightness [0..255]` and
`RazeruHidTest mouse-brightness [0..255]`; omit the number for read-only access.
Regression: stop Razeru, set temporary brightness to 0, exit the test, send
`frame FF00FF` / `mouse-static FF00FF` from a new process, and verify brightness
255. Then restart Razeru. The `VARSTORE=1` profile is never written.

`.chroma` version 1 is read without the Razer animation DLL. The reader checks
the device type, frame count, exact file length, finite durations and every
read. It supports standard Keyboard 6x22 and Keyboard Extended 8x24. For the
extended format it intentionally reproduces Razeru 1's
`PluginGetFrameName(..., length=132)` behavior: the first 132 row-major colors
are used and the rest are discarded.
Mouse 9x7 files receive the same structural checks; only the logo cell is kept
for each Viper frame.

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
2. all bundled keyboard and mouse animations accepted and malformed input rejected;
3. firmware query and color tests on both the keyboard and Viper;
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
