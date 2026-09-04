# Razeru 2 user guide

Razeru 2 watches the input language of the foreground Windows application and
renders the configured `.chroma` keyboard animation directly on a Razer
Huntsman V2 Tenkeyless. With a Razer Viper connected, its logo synchronously
renders the logo cell from the configured mouse animation. Static language
highlights and reactive key effects can be blended over the keyboard animation.

Razeru does not record typed text. The low-level keyboard hook observes key
events only to render the configured reactive effect.

## First run

1. Connect the Huntsman V2 TKL (`1532:026B`) directly to USB.
2. Install and start Razeru 2. Razer applications are not a prerequisite.
3. If prompted, allow the default `Razeru.json` to be created.
4. Find **Razeru 2 Lang indicator** in the Windows notification area.
5. Change the input language in an active window and confirm the lighting
   animation changes.

The tray menu provides **Settings**, **Editor**, and **Exit**. **Editor** opens
the official Razer web editor in the default browser; it does not load or start
the local Chroma SDK.

## Files and upgrades

The default per-user installation directory is:

```text
%LOCALAPPDATA%\Programs\Razeru
```

It contains `Razeru.exe`, `RzruUI.dll`, `Razeru.json`, `Animations`, the license
and documentation. There is no Razer runtime DLL in the Razeru 2 payload.
Upgrades and uninstall preserve the configuration and animation directory.

`Razeru.json` is UTF-8 JSON. Relative animation paths are resolved from the
application directory. Both standard 6x22 keyboard animations and the 8x24
Keyboard Extended files already bundled with Razeru are accepted.

## Current hardware scope

Razeru 2.0 supports Razer Huntsman V2 Tenkeyless USB
`VID_1532&PID_026B`, HID interface `MI_03`, hardware matrix 6x17. Chroma's
logical columns 1-17 map to device columns 0-16. The second tested profile is
the Razer Viper `VID_1532&PID_0078`, interface `MI_00`; its logo color comes
from Chroma Mouse 9x7 coordinate `(7,3)`. Mouse pads, headsets, keypads, Chroma
Link, wireless variants, and other product IDs are not sent lighting commands.

## Troubleshooting

- **Everything is dark after power loss:** version 2.0.0.3 sets temporary
  keyboard and mouse-logo brightness to 100% before the first lighting output
  and after reopening the HID connection. Read-back confirms the setting;
  the stored device profile is not changed. Earlier versions could transmit
  valid colors while brightness remained zero.
- **Mouse goes dark after screen lock:** version 2.0.0.4 reapplies temporary
  100% brightness when lighting resumes after unlock. During effect output,
  brightness is checked approximately every 2 seconds: zero is restored to
  100%, while nonzero brightness is preserved. This also covers delayed
  brightness resets without a disconnected HID handle. Exit Razeru before
  intentionally leaving the device switched off.
- **Keyboard not found:** reconnect the keyboard directly, confirm the USB ID
  in Device Manager, and verify that `HidUsb` is active on interface `MI_03`.
- **Mouse does not change color:** verify `1532:0078`, interface `MI_00`, the
  device-animation setting, and both layout-specific `*_Mouse.chroma` paths.
- **Keyboard found but does not answer:** exit other lighting applications and
  retry. They can compete for the same feature-report channel.
- **Colors alternate or appear chaotic:** do not run Razer App Engine, Chroma
  effects, Windows Dynamic Lighting, or another RGB controller alongside
  Razeru. Only one program should own the keyboard lighting.
- **Animations do not load:** restore the original `Animations` directory and
  verify paths in `Razeru.json`.
- **Settings do not open:** keep `RzruUI.dll` beside `Razeru.exe` and install the
  current Microsoft Visual C++ x64 runtime.
- **A second instance exits:** the global Razeru mutex intentionally permits a
  single active instance.

Before removing Razer programs and filters, follow the staged verification in
[Razer software requirements and removal](RAZER_REQUIREMENTS.md).
