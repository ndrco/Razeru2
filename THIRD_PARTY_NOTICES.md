# Third-party notices

This file is an inventory, not a replacement for the applicable license texts.

## nlohmann/json 3.12.0

Copyright © 2013-2025 Niels Lohmann.

Licensed under the MIT License. The exact license included with the vendored
single header is at `third_party/nlohmann/LICENSE.MIT`; source provenance and
the audited SHA-256 are recorded in `third_party/README.md`.

Project: https://github.com/nlohmann/json

## Razer Chroma SDK reference material

The legacy API wrapper sources under `Razer/` and their associated
headers/assets originate from Razer's Chroma SDK materials. The repository's
`LICENSE` includes the Razer MIT License notice applicable to those imported
components. The obsolete `CChromaEditorLibrary*.dll` copies were removed from
the Razeru 2 release branch.

Upstream sample lineage: https://github.com/razerofficial/CSDK_SampleApp

Razeru 2 excludes the legacy sources from its executable and installer. The
version-1 `.chroma` reader was independently implemented from the documented
serialized layout and checked for compatibility with the MIT-licensed editor
source. No Razer Chroma runtime binary is stored or redistributed by the
Razeru 2 release.

## OpenRazer and OpenRGB protocol references

The direct Huntsman V2 TKL HID implementation was written for this project and
cross-checked against the public device/protocol research in OpenRazer and
OpenRGB. Their source code is not vendored into `RazerHidDevice.*`; their own
copyright and license terms continue to govern the upstream projects.

- OpenRazer: https://github.com/openrazer/openrazer
- OpenRGB: https://gitlab.com/CalcProgrammer1/OpenRGB

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
