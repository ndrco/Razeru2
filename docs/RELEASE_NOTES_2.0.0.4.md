# Razeru 2.0.0.4

Restores keyboard and mouse lighting when temporary device brightness resets
to zero, including after Windows screen unlock.

## Changes

- Set temporary brightness to 100% before first output and after reconnect.
- Reinitialize brightness when lighting resumes after session unlock.
- During lighting output, check brightness approximately every two seconds;
  restore zero to 100% while preserving nonzero dimming between initializations.
- Verify brightness changes by reading back the device state. No onboard
  profile or flash writes are made.
- Add brightness diagnostics and hardware regression commands for both devices.
- Update the installer version and Russian/English documentation.

## Validation

- Release x64 build and static analysis completed without errors; three existing
  warnings remain in the third-party JSON header.
- 163 keyboard and 22 mouse animations validated; malformed input rejected.
- Hardware regressions passed on Huntsman V2 TKL and Viper: initial recovery,
  periodic recovery on an existing connection, preservation of brightness 128,
  and resume reinitialization.
- Installation preserved configuration and startup settings. Three synthetic
  session cycles passed; the user subsequently confirmed working lighting
  after the requested real Windows lock/unlock check.
- Sleep, reboot and power-loss cycles were not retested for this version.

## Installer

- File: `Razeru-Setup-2.0.0.4-x64.exe`
- SHA-256: `B8F41AC70EA7DFDD48FCF9467AA05EDFCC85E0455D8BD4BECDF371CF1FC7BDB7`
