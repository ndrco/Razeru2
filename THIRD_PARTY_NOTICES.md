# Third-party notices

This file is an inventory, not a replacement for the applicable license texts.

## nlohmann/json 3.12.0

Copyright © 2013-2025 Niels Lohmann.

Licensed under the MIT License. The exact license included with the vendored
single header is at `third_party/nlohmann/LICENSE.MIT`; source provenance and
the audited SHA-256 are recorded in `third_party/README.md`.

Project: https://github.com/nlohmann/json

## Razer Chroma SDK components

The Razer API wrapper sources under `Razer/`, associated headers/assets, and
the bundled `CChromaEditorLibrary32.dll` / `CChromaEditorLibrary64.dll` originate
from Razer's Chroma SDK materials. The repository's `LICENSE` includes the Razer
MIT License notice applicable to those imported components.

Upstream sample lineage: https://github.com/razerofficial/CSDK_SampleApp

The installer packages only the `CChromaEditorLibrary64.dll` animation bridge
and verifies its valid Razer USA Ltd. Authenticode signature before compilation.
The separate Razer Chroma App and its system SDK core are not redistributed.
Chroma App is the minimum user-installed Razer product. Its vendor-managed App
Engine, services, and drivers remain external dependencies; Synapse and other
Razer applications are not required by Razeru.

## Microsoft Visual C++ Redistributable

The x64 Visual C++ runtime installer is downloaded from Microsoft's official
Visual Studio 17 release endpoint during packaging. It is not stored in Git.
The build script accepts only the pinned SHA-256 and a valid Microsoft
Authenticode signature. Redistribution and use remain subject to the applicable
Microsoft Visual Studio license terms.

Download endpoint: https://aka.ms/vs/17/release/vc_redist.x64.exe

## Windows platform components

Razeru uses Windows, MFC, ATL, GDI+, and Windows SDK headers/libraries supplied
with Windows or Visual Studio Build Tools. Their use and redistribution are
governed by the corresponding Microsoft terms.
