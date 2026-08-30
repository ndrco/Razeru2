# Razeru 2.0.0.0

Razeru 2 replaces the local Razer Chroma SDK dependency with direct USB HID
control for the Razer Huntsman V2 Tenkeyless (`1532:026B`). It runs through the
standard Windows HID stack and does not require Synapse, Chroma App, App Engine,
Razer services, vendor filter drivers, or `CChromaEditorLibrary*.dll`.

## Highlights

- Direct, validated control of HID interface `MI_03`, usage page `0x000C`,
  usage `0x0001`.
- Corrected 6x17 device mapping: logical Chroma columns 1-17 map to physical
  columns 0-16, including Tab, Caps Lock and pressed-key animations.
- Built-in reader for standard and extended `.chroma` animation files.
- The Editor command opens the official web editor at
  <https://chroma.razer.com/ChromaEditor>.
- Per-user x64 setup preserves existing `Razeru.json` and `Animations` while
  removing obsolete Chroma animation DLLs from the application directory.

## Validation

- Release x64 build completed successfully.
- All 163 bundled keyboard animations passed the independent reader test.
- Hardware query opened the exact `MI_03` interface and reported firmware
  `1.0`.
- The executable import table contains only Windows and Visual C++ runtime
  dependencies; no Razer DLL is loaded.
- The tested keyboard continues to work after removal of the Razer HID driver
  packages and falls back to Microsoft `input.inf` / `HidUsb`.

## Release asset

- File: `Razeru-Setup-2.0.0.0-x64.exe`
- SHA-256: `B1E26C719ABCD7AC3DB3B236F0EFF217496E15B12CA1E8A4CD85E4C3C512F63B`
- Authenticode: unsigned; Windows may show a SmartScreen warning.

The first hardware profile intentionally supports only the Huntsman V2
Tenkeyless with USB ID `1532:026B`. Other Razer models are rejected until their
report layout has been independently validated.
