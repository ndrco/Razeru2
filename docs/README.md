# Razeru user guide

## What the application does

Razeru polls the keyboard layout of the foreground Windows application and
maps its language identifier to a configured Chroma scene. The scene may
contain animations for the keyboard, mouse, mouse pad, headset, keypad and
Chroma Link devices. On the keyboard, an optional static color map and a
reactive or wave effect for pressed keys can be composited with the animation.

Razeru does not record typed text. Its global low-level keyboard hook receives
virtual-key press/release events only to render per-key effects. The application
runs locally and contains no network or telemetry code found in this audit.

## First start

1. Connect a compatible Razer Chroma keyboard.
2. Install and start Razer Chroma App, open **Chroma Apps**, and enable it.
   Razer Synapse is not required.
3. Run the Razeru installer and start Razeru from the Start menu.
4. If the configuration file cannot be loaded, allow Razeru to create defaults.
5. Find the Razeru icon in the Windows notification area.
6. Right-click the icon and select **Settings**.

Only one instance can run at a time. A second start displays an error and exits.
The application pauses its hook and Chroma output when the Windows session is
locked and restores them after unlock.

## Settings

The settings dialog contains three functional areas:

1. **Language scenes** — add or remove a Windows input language, select the
   `.chroma` file for each device type, and set animation speed.
2. **Keyboard backlight** — enable the static layer, paint individual keys,
   clear the map, and choose blend and scene modes.
3. **Key effect** — select no effect, Reactive, or Wave; choose its color, frame
   duration, and number of frames.

Save writes `Razeru.json` beside `Razeru.exe` and immediately tells the running
process to reload it. The Chroma Editor command opens Razer's editor for
`.chroma` assets.

## Minimum Razer software

Install only **Razer Chroma App** as the user-selectable Razer prerequisite.
Its App Engine, Chroma SDK services, and device drivers are required shared
components and must remain installed. Razer Synapse, Axon, Cortex, and THX
Spatial Audio are not required by Razeru. Decline promotional applications
when the Razer installer offers them.

See [Minimum Razer components](RAZER_REQUIREMENTS.md) for the component table,
official sources, clean-install procedure, and verification commands.

## Files and upgrades

The standard installation directory is:

```text
%LOCALAPPDATA%\Programs\Razeru
```

Important files:

- `Razeru.exe` — tray process and keyboard-layout monitor;
- `RzruUI.dll` — MFC settings UI and JSON persistence;
- `CChromaEditorLibrary64.dll` — signed Razer animation bridge; the system SDK
  is supplied separately by Chroma App;
- `Razeru.json` — user configuration;
- `Animations\` — `.chroma` assets;
- `docs\` — offline documentation.

During upgrade, the installer replaces program binaries and documentation but
does not overwrite an existing configuration or animation directory. Uninstall
also preserves those user files. Remove the remaining Razeru directory manually
if those customizations are no longer required.

## Automatic start

The installer offers an optional **Start Razeru when I sign in** task. It writes
the quoted executable path to:

```text
HKCU\Software\Microsoft\Windows\CurrentVersion\Run
```

The settings UI can also control the same current-user startup entry. No
system-wide service or driver is installed.

## Configuration reference

`Razeru.json` is UTF-8 JSON. Relative animation paths are resolved from the
application's working directory, so keep the `Animations` directory next to the
executable. Prefer changing settings through the UI.

| Field | Meaning |
| --- | --- |
| `primary_lang`, `sub_lang` | Language of the settings UI as Windows language IDs |
| `devicesAnimation` | Enables animations on non-keyboard Chroma devices |
| `keyboardEffect[]` | One scene per Windows keyboard-layout language ID |
| `keyboardLayout` | Numeric Windows `LANGID`, e.g. 1033 for English (US) |
| `*Animation` | Relative `.chroma` paths for the supported device types |
| `speed` | Frames advanced at each animation step; values below 1 are normalized |
| `keyEffect` | Press-effect type, RGB integer, frame duration and frame count |
| `backlightEffect` | Static layer switch, blend/mode values and keyboard color matrix |

Invalid or missing animation metadata is normalized to safe positive timing and
frame-count defaults, but malformed JSON can still prevent the configuration
from loading. Back up custom configurations before manual editing.

## Troubleshooting

- **Chroma DLL not found:** install or repair the separate
  [Razer Chroma App](https://www.razer.com/chroma), enable **Chroma Apps**, and
  restart Razeru. Reinstalling Razeru itself is not required.
- **Failed to load the Razeru Chroma bridge:** repair Razeru and the Microsoft
  Visual C++ x64 Redistributable.
- **No Chroma devices / no keyboard:** connect a compatible Chroma keyboard and
  verify it in Chroma App before restarting Razeru. Reinstalling Razeru does not
  fix a disconnected device.
- **Invalid DLL signature:** do not replace the bundled Razer DLL with an
  untrusted copy; reinstall from a trusted Razeru package.
- **No layout animation:** add the exact Windows input language in Settings and
  verify all referenced animation files exist.
- **The mouse animates but the keyboard is black when manual layers are
  enabled:** use Razeru 1.2.0.4 or newer. Its manual compositor uses the
  low-level SDK effect path because the current Chroma App can accept the
  compatibility `SetEffectCustom2D` call without visibly rendering it. As a
  comparison test, temporarily disable the static backlight and key effect;
  the keyboard will then use automatic playback.
- **No tray icon:** check the notification-area overflow and ensure another
  Razeru instance is not running.
- **Configuration is rejected:** restore `config\Razeru.json` from the source
  tree or let the application load defaults, then save.

For diagnostic builds, use a debugger and view `OutputDebugString` messages in
Visual Studio or Sysinternals DebugView.
