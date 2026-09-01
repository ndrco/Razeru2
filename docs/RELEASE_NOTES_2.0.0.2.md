# Razeru 2.0.0.2

This maintenance release makes direct Razer Viper lighting reliable after
startup and after transient USB/HID access failures.

## Changes

- Mouse initialization no longer depends on a successful optional firmware
  query.
- A disconnected or temporarily unavailable Viper is reopened automatically.
- A failed feature report closes the stale handle and schedules a fresh retry.
- Reconnect attempts are throttled to one every two seconds.
- Keyboard operation remains independent from the optional mouse connection.

## Validation

- Release x64 build completed successfully.
- All keyboard and mouse animation files passed validation.
- RU/EN color switching was confirmed on the connected Huntsman V2 TKL and
  Razer Viper using the production code path.
- The installed application and bundled binaries match the verified release
  build.

## Release asset

- File: `Razeru-Setup-2.0.0.2-x64.exe`
- SHA-256: `76DFD4A3253C7D450780AD0AE811618A8984CFEB71ECDBDEDE3A1C4545C3A531`
- Authenticode: unsigned; Windows may show a SmartScreen warning.
