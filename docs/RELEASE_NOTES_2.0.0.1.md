# Razeru 2.0.0.1

This patch adds direct input-language indication on the Razer Viper logo
(`1532:0078`) without Synapse, the Chroma SDK, or Razer services and drivers.

## Highlights

- Added a Viper hardware profile for HID interface `MI_00`, usage `0001:0002`,
  91-byte feature reports, and validated firmware `1.7`.
- Added a bounds-checked Chroma Mouse 9x7 reader. The Viper's single lighting
  element uses the standard logo cell at `(7,3)`.
- Synchronized the logo with the active layout frame: green for RU and pink
  for EN in the default profile.
- Extended static commands use `NOSTORE=0`, so animation frames are never
  written to the mouse's onboard storage.
- The Viper remains optional; an absent mouse does not prevent startup or
  direct Huntsman V2 TKL control.

## Validation

- Release x64 build completed successfully.
- All 163 keyboard and 22 mouse animations passed; malformed input was rejected.
- Firmware query, static color, and synchronized RU/EN switching were confirmed
  on the connected Razer Viper.
- The application remained responsive and accumulated no measurable CPU time
  during a ten-second playback sample.

## Release asset

- File: `Razeru-Setup-2.0.0.1-x64.exe`
- SHA-256: `908CA17244EEE82B0FA2858937AC6D2737DF0CB3A774E9D81E7EE41267F880D0`
- Authenticode: unsigned; Windows may show a SmartScreen warning.
