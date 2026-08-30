# Razer software requirements and removal

## Runtime requirements

Razeru 2 does not require Razer Synapse, Chroma App, App Engine, Chroma SDK
services, `RzChromatic64.dll`, `RzChromaSDK64.dll`, Razer filter drivers, or
`CChromaEditorLibrary64.dll`. It uses Windows SetupAPI and HID APIs directly.

The first profile is intentionally narrow:

| Item | Supported value |
| --- | --- |
| Keyboard | Razer Huntsman V2 Tenkeyless |
| USB ID | `VID_1532&PID_026B` |
| HID interface | `MI_03`, usage page `0x000C`, usage `0x0001` |
| Lighting matrix | 6x17; logical columns 1-17 map to device 0-16 |
| Required Windows drivers | `usbccgp`, `HidUsb`, `kbdhid` |
| Firmware tested during development | `1.0` |

The keyboard keeps normal typing and lighting on the standard Microsoft HID
drivers. Razeru 2 sends volatile feature reports; it does not flash firmware or
save profiles to device storage. Other Razer models are rejected instead of
receiving an unverified report format.

The **Editor** command opens Razer's official web editor at
<https://chroma.razer.com/ChromaEditor>. It needs a browser and internet access,
but the running tray application does not.

## Safe validation before removing Razer software

Do not make driver removal the first test. Keep a working rollback path and
validate the new transport in this order:

1. Back up `Razeru.json` and `Animations`.
2. Exit the old Razeru and start Razeru 2 from its own test directory.
3. Run `RazeruHidTest info`; it must report the keyboard path and firmware.
4. Send temporary red, green and blue effects, then restore `spectrum`.
5. Confirm that Razeru 2 changes animation when the foreground input language
   changes and that typing remains normal.
6. Disable Razer programs from startup and stop their user processes/services,
   reboot, and repeat steps 3-5. Administrative rights may be needed to stop
   protected services.
7. Only after this service-free test succeeds, uninstall Razer products using
   their registered uninstallers and reboot again.

Razer's official clean-install procedure recommends uninstalling all selected
Razer applications first, removing the documented residual Razer folders, and
then restarting Windows:
<https://mysupport.razer.com/app/answers/detail/a_id/1708>. If vendor driver
packages remain after that process, use Razer's signed Driver Clean-up Tool or
identify and remove only the exact published INF packages:
<https://mysupport.razer.com/app/answers/detail/a_id/20339>.

The system may currently show a composite path with `RZCONTROL`/`RzCommon` and
a virtual `RzDev_026b` device. Those are Razer additions, not requirements of
Razeru 2. After uninstall and reboot, the physical keyboard should still expose
its normal `usbccgp` -> `HidUsb` -> `kbdhid` path.

Running Razer App Engine or its `razerwdl.exe` LampArray bridge together with
Razeru was confirmed to produce competing, apparently random colors. Windows
Dynamic Lighting being disabled does not by itself stop the Razer bridge.

## After uninstall

In Device Manager, verify that:

- the keyboard remains present as `USB\VID_1532&PID_026B`;
- its HID interface uses Microsoft's `HidUsb`;
- keyboard input uses `kbdhid`;
- there are no problem codes or repeated disconnects;
- `RazeruHidTest info` and Razeru 2 still work.

Do not manually delete driver packages or registry filter entries before the
vendor uninstall/reboot cycle. If a Razer filter remains, identify its exact
published INF with `pnputil /enum-drivers` and confirm no live device uses it
before removal. Removing an active input filter can temporarily disable the
keyboard and requires administrator rights.

If direct HID access fails after cleanup, reconnect the keyboard to another USB
port and check the Microsoft HID drivers first. Reinstalling Razer software is
the rollback while collecting the exact device/interface and error details.
