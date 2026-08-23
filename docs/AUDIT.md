# Razeru technical audit

Audit date: 2026-08-22  
Source baseline: `main` at `6ee343a5fe6a4714ea812b36bc28f6bdeff72e81`  
Scope: first-party C++ sources, Visual Studio projects, bundled native binaries,
configuration, dependencies, build reproducibility, packaging and documentation.

## Executive summary

The baseline was a small, understandable native Windows application but was not
reproducibly buildable on a clean machine and was not ready for end-user
installation. The audit fixed the known build blockers, several memory/threading
defects, invalid-input crashes, a startup-path issue and broken license text. A
reproducible build entry point, bilingual documentation and a per-user x64 Inno
Setup package were added.

`Release|x64` now rebuilds successfully with MSVC v143. MSVC code analysis also
completes; its remaining three warnings are confined to the vendored
nlohmann/json single header (two intentional switch fall-through diagnostics and
one object-initialization diagnostic), not first-party sources.

The result is suitable for development and controlled testing. A public release
should wait for real-device tests and code signing. There is no automated test
suite, and several defense-in-depth improvements remain.

## Method

- inspected the solution, C++ code, resources, JSON persistence and installer;
- reproduced clean `Release|x64` compilation and link;
- ran MSVC native code analysis with C++ Core Check;
- inspected PE architecture, imports, file versions and Authenticode status;
- verified the tracked Razer x64 DLL has a valid Razer USA Ltd. signature;
- verified the packaged VC++ Redistributable by pinned SHA-256 and Microsoft
  signature;
- reviewed threading, keyboard-hook lifecycle, DLL loading, registry startup,
  configuration boundaries, licensing and upgrade/uninstall behavior;
- compiled and smoke-tested the installer;
- installed Razer Chroma App, removed standalone Synapse and Axon with their
  signed vendor uninstallers, restarted the SDK services, and launched Razeru
  again in the resulting minimum product configuration.

Without Synapse installed, `InitSDK2` returned `0`, Chroma reported online with
access `1`, and Razeru loaded the bridge plus both system SDK DLLs. All
`VID_1532` devices were physically disconnected during the final check, so an
actual lighting effect could not be validated; Razeru reached its later,
expected no-keyboard diagnostic instead. This hardware limitation remains
material, but Synapse independence is verified.

## Remediated findings

| Severity | Finding in baseline | Remediation |
| --- | --- | --- |
| High | `GetModuleFileNameW` received a byte count instead of a wide-character element count, permitting an out-of-bounds write. | Replaced with `_countof` and rebuilt/analyzed. |
| High | Build depended on an absolute nlohmann/json path from another developer's profile. | Vendored nlohmann/json 3.12.0 with its MIT license and used `$(SolutionDir)third_party`. |
| High | `RzruUI/resource.h` was absent and accidentally ignored, blocking resource compilation. | Reconstructed stable resource IDs and corrected `.gitignore`. |
| Medium | Allocation results in Razer signature/version helpers could be dereferenced before a null check. | Corrected guards and version-info calls. |
| Medium | The key-state vector was concurrently mutated and iterated by the hook/main and worker threads. | Protected mutation, clearing and snapshot creation with the existing mutex and eliminated duplicate entries. |
| Medium | Zero/invalid animation frame counts or durations could produce modulo-by-zero and invalid waits. | Added positive fallbacks, clamped speed/count values and normalized the frame index. |
| Medium | Startup command stored an unquoted executable path. | Stored a quoted path in the current-user Run value. |
| Medium | Post-build wildcard copying mixed x86/x64 DLLs and attempted to copy nonexistent PDBs, causing successful compiles to report failure. | Replaced it with platform-specific exact DLL copying. |
| Medium | Solution/project platform names disagreed (`x86` versus `Win32`), and secondary EXE configurations used the Console subsystem while defining only `wWinMain`; Win32 UI builds also lacked the required C++ standard. | Added platform mapping in the script, selected the Windows subsystem consistently, and enabled C++20 for all UI configurations. |
| Medium | Setup and documentation treated Synapse as a prerequisite even though current Synapse may omit the SDK core and the standalone Chroma App supplies it. This both caused error 6023 and encouraged unnecessary software installation. | Added explicit system-runtime detection, official Chroma App handoff, launch suppression while missing, actionable errors, and a tested minimum of Chroma App without Synapse/Axon. |
| Medium | Root `LICENSE` contained committed merge-conflict markers and two license bodies with no boundary. | Removed conflict markers, retained GPLv3 for the project and labeled the Razer MIT component section. |
| Low | Two temporary `CMFCColorButton` constructions had no effect and generated analyzer noise. | Removed them. |
| Low | Locale/module buffer lengths used ambiguous byte-sized expressions. | Replaced audited calls with `_countof`. |

## Open findings and recommendations

### Medium: first-party binaries and installer are unsigned

`Razeru.exe`, `RzruUI.dll` and the generated setup have no project-controlled
Authenticode signature. Windows reputation warnings and supply-chain ambiguity
are therefore expected. Obtain a protected code-signing certificate, sign the
EXE/DLL before packaging, configure Inno Setup to sign setup/uninstaller, add
timestamping, and verify signatures in the release pipeline.

### Medium: configuration handoff is not transactionally synchronized

The application pauses its animation loop before replacing configuration, but
pause is an atomic request rather than an acknowledged barrier. Under an unlucky
schedule, the worker may still be inside Chroma/configuration code while the
main thread updates it. Add a mutex dedicated to `ChromaPlaying` state or a
pause/acknowledge protocol and cover reload-under-load with a stress test.

### Medium: configuration saves are non-atomic

`RzruUI.dll` writes directly to `Razeru.json` with truncation. A crash, power
loss, parse/serialization exception, or disk error can leave an empty/partial
file. Serialize to a same-directory temporary file, flush it, then atomically
replace the target. Catch and report JSON and numeric conversion exceptions.

### Medium: DLL loading should use an explicit trusted path

The UI module is loaded by the bare name `RzruUI.dll`. Normal installed startup
uses the application directory, but an explicit absolute application-directory
path plus restricted DLL search policy would better prevent search-order
hijacking. Use `SetDefaultDllDirectories`/`LoadLibraryEx` as appropriate and test
the Razer runtime's loading assumptions.

### Low: path handling uses narrow streams

Configuration and animation paths cross `std::string`/ANSI APIs. Installation
uses an ASCII directory by default, but manually selected assets in paths not
representable in the active code page may fail. Move filesystem boundaries to
UTF-16 Win32 paths or `std::filesystem::path` and validate non-Latin profiles.

### Low: the keyboard hook is process-wide input-sensitive functionality

The hook is technically justified and the audited code does not persist or
transmit keystrokes, but it increases privacy and security sensitivity. Keep the
callback minimal, document the behavior, avoid logging virtual keys in release
builds, and consider ignoring injected events if not required.

### Low: startup and configuration UX has rough edges

The process waits for a splash window before its singleton check, settings/menu
localization is incomplete, and invalid configuration recovery is modal. Move
the singleton check to the earliest startup point and improve localized,
actionable error reporting.

### Quality gap: no automated tests or CI

There are no unit tests, integration tests, CI builds, dependency-update checks,
or repeatable real-device test records. Extract pure layout/config/effect logic
for unit tests; add an x64 build/analyze/installer workflow; test install,
upgrade, uninstall, lock/unlock and configuration reload. Keep hardware-in-loop
testing as a signed release gate.

## Dependency and binary inventory

| Component | Source/status | Notes |
| --- | --- | --- |
| nlohmann/json 3.12.0 | Vendored single header, MIT | Hash and license recorded in `third_party/README.md`. |
| Razer Chroma API wrapper | Source under `Razer/`, MIT component notice | Signature verification is enabled in normal builds. |
| `CChromaEditorLibrary64.dll` 1.0.1.2 | Tracked animation bridge, valid Razer USA Ltd. signature | Installer includes x64 only; Razer Chroma App supplies the separate system SDK core. |
| Razer Chroma App 4.0.699 | External vendor application, valid signed installer | Minimum user-selectable Razer product; installs App Engine/Common, SDK core/services, and required device support. |
| Microsoft Visual C++ x64 runtime 14.44.35211.0 | Downloaded at packaging, valid Microsoft signature and pinned SHA-256 | Installed only when the detected runtime build is older. |
| Windows/MFC/GDI+ APIs | Windows/Visual Studio platform dependencies | MFC is linked statically in `RzruUI.dll`; the standard C++ runtime remains dynamic. |

No first-party network client, telemetry endpoint, updater, service or driver was
found. Razer's Chroma App, SDK/runtime, shared services, and device drivers are
external trusted components and were not reverse engineered by this audit.
Synapse, Axon, Cortex, and THX Spatial Audio are not Razeru dependencies.

## Release disposition

Recommended status: **development/controlled beta**. The build and installer are
reproducible, and the concrete high-severity source/build issues found here are
fixed. Promote to a public production release after:

1. resolving or accepting the configuration synchronization risk;
2. completing Windows 10/11 tests with actual Razer devices and Chroma App,
   with Synapse absent from at least one test image;
3. signing all first-party binaries and the installer;
4. adding at least build/analyzer/package CI and core configuration unit tests.
