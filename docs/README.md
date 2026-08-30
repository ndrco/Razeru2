# Razeru 2 user guide

Razeru 2 watches the input language of the foreground Windows application and
renders the configured `.chroma` keyboard animation directly on a Razer
Huntsman V2 Tenkeyless. Static language highlights and reactive key effects can
be blended over the animation.

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

Razeru 2.0 supports one tested profile: Razer Huntsman V2 Tenkeyless USB
`VID_1532&PID_026B`, HID interface `MI_03`, hardware matrix 6x17. Chroma's
logical columns 1-17 map to device columns 0-16. Mouse, mouse pad, headset, keypad,
Chroma Link, wireless variants, and other keyboard product IDs are not sent
lighting commands even if their older configuration fields remain visible.

## Troubleshooting

- **Keyboard not found:** reconnect the keyboard directly, confirm the USB ID
  in Device Manager, and verify that `HidUsb` is active on interface `MI_03`.
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
