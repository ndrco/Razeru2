# Minimum Razer components for Razeru

## Short answer

The user needs to select and install one Razer product: **Razer Chroma App** for
64-bit Windows 10/11. The app must be running and **Chroma Apps** must be
enabled. Razer Synapse is not a Razeru prerequisite.

A compatible Chroma keyboard must also be physically connected. Razeru checks
for a keyboard during startup and exits when none is available. A mouse, mouse
pad, headset, keypad, and Chroma Link devices are optional.

Official references:

- [install Razer Chroma App](https://mysupport.razer.com/app/answers/detail/a_id/14664/);
- [Chroma App features and requirements](https://mysupport.razer.com/app/answers/detail/a_id/13698/);
- [set up the Chroma SDK](https://developer.razer.com/works-with-chroma/setting-up/);
- [Chroma SDK troubleshooting](https://developer.razer.com/works-with-chroma/chroma-faq/).

## Components that must remain installed

| Component | Status | Purpose |
| --- | --- | --- |
| **Razer Chroma App** | Required | Runs Chroma Apps, manages devices, and installs the system SDK core. |
| **Razer App Engine / Common** | Required as part of Chroma App | Shared host used by Chroma App. Razer installs and updates it automatically. |
| **Razer Chroma SDK Core** | Required as part of Chroma App | Supplies `RzChromatic64.dll`, `RzChromaSDK64.dll`, and the background SDK services. |
| **Razer Chroma SDK Service** and related Server/Diagnostic services | Required | Receive effects from Razeru and forward them to devices. |
| Device-specific Razer drivers | Required for connected hardware | Allow the keyboard to be detected and its lighting to be controlled. |
| `CChromaEditorLibrary64.dll` | Bundled with Razeru | Signed animation bridge; it is not a separate program or a substitute for the system SDK. |

Do not manually remove App Engine, shared services, SDK directories, or device
drivers. Chroma App installs them as a coordinated unit, and their names and
composition may change in future updates. A shared Razer component is not by
itself an extra user-selectable application.

## Software Razeru does not require

- **Razer Synapse** is optional; keep it only for macros, profiles, firmware
  updates, and device functions outside Razeru.
- **Razer Axon** is not required; it is a wallpaper/personalization product.
- **Razer Cortex** is not required.
- **THX Spatial Audio** is not required.
- Other promotional applications offered by the installer are not required.

## Minimal installation procedure

1. Connect a compatible Razer Chroma keyboard.
2. Download the [official Razer Chroma App](https://www.razer.com/chroma).
3. Select Chroma App and decline offers for Axon, Cortex, or other optional
   products.
4. Launch Chroma App, complete sign-in/activation if required by the current
   Razer release, and enable **Chroma Apps**.
5. Leave Chroma App automatic startup enabled, then install Razeru.

Razer lists 64-bit Windows 10/11, a valid e-mail address, license acceptance,
and an internet connection as Chroma App requirements. Razeru itself has no
network client; the external Razer application uses the connection for install,
full-feature activation, and updates.

## Verification

At minimum, **Razer Chroma SDK Service** and its related SDK services should be
running in `services.msc`. The current runtime places `RzChromatic64.dll` and
`RzChromaSDK64.dll` in `System32`. Open **Chroma Apps** in Chroma App and verify
that application control is enabled.

After starting Razeru:

- error `6023` means that the system SDK core is absent or damaged;
- `RESOURCE_DISABLED` means Chroma Apps is disabled;
- `Chroma keyboard is not connected` means SDK startup succeeded but Windows
  cannot see a compatible keyboard; check the cable, USB port, driver, and the
  device in Chroma App. Reinstalling Razeru does not fix this condition.

## Validated minimal configuration

On 22 August 2026, Razer Synapse 4.0.699 and Razer Axon 2.9.1.0 were removed
with their signed vendor uninstallers from the test computer. Razer Chroma
4.0.699 remained as the only user-selectable Razer product, together with its
automatically managed shared components. After stopping user processes and
restarting the three Chroma SDK services:

- Chroma App started with no Synapse process;
- `InitSDK2` returned `0`;
- the SDK reported Chroma online and access `1`;
- Razeru loaded `CChromaEditorLibrary64.dll`, `RzChromatic64.dll`, and
  `RzChromaSDK64.dll`.

On 23 August, a Razer Huntsman V2 TKL (`PID026B`) and Razer Viper (`PID0078`)
were connected to the same minimal configuration. Automatic keyboard and mouse
animations worked without Synapse, and a direct low-level keyboard effect
remained visible for a continuous 15-second test. The compatibility
`SetEffectCustom2D` call returned success but the current Chroma App did not
retain its visible keyboard output. Starting with Razeru 1.2.0.4, the manual
compositor (static layer and key-press effects) therefore uses the validated
`CoreCreateKeyboardEffect` / `CoreSetEffect` path. This verifies Razeru with
connected hardware and no Synapse installation.

The small `C:\ProgramData\Razer\Synapse3\Uninstall` directory was deliberately
retained because it contains the vendor uninstall helpers for the installed
`PID0078` and `PID026b` device drivers. The Synapse product, App Engine module,
processes, and automatic-start entries are absent.
