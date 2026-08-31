#ifndef AppVersion
  #define AppVersion "2.0.0.1"
#endif
#ifndef BuildOutputDir
  #define BuildOutputDir SourcePath + "..\x64\Release"
#endif
#ifndef VCRedistPath
  #define VCRedistPath SourcePath + ".cache\vc_redist.x64.exe"
#endif

#define AppName "Razeru 2"
#define AppPublisher "NDR Co"
#define AppExeName "Razeru.exe"
#define RepoRoot SourcePath + ".."

[Setup]
AppId={{100CC699-6D7B-4B87-B284-3C17859FC597}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL=https://github.com/ndrco/Razeru
AppSupportURL=https://github.com/ndrco/Razeru/issues
AppUpdatesURL=https://github.com/ndrco/Razeru/releases
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription=Razeru 2 Windows installer
VersionInfoProductName={#AppName}
DefaultDirName={localappdata}\Programs\Razeru
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir={#RepoRoot}\dist
OutputBaseFilename=Razeru-Setup-{#AppVersion}-x64
SetupIconFile={#RepoRoot}\Resources\Razeru.ico
UninstallDisplayIcon={app}\{#AppExeName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
AppMutex=Global\Razeruv10AppMutex
LicenseFile={#RepoRoot}\LICENSE
SetupLogging=yes
#ifdef SignToolName
SignTool={#SignToolName}
SignedUninstaller=yes
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[CustomMessages]
english.StartupTask=Start Razeru 2 when I sign in
russian.StartupTask=Запускать Razeru 2 при входе в систему
english.StartupGroup=Startup:
russian.StartupGroup=Автозапуск:
english.DocumentationShortcut=Documentation
russian.DocumentationShortcut=Документация
english.UninstallShortcut=Uninstall Razeru 2
russian.UninstallShortcut=Удалить Razeru 2
english.RuntimeStatus=Installing Microsoft Visual C++ Runtime...
russian.RuntimeStatus=Установка среды Microsoft Visual C++...

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startup"; Description: "{cm:StartupTask}"; GroupDescription: "{cm:StartupGroup}"; Flags: unchecked

[Files]
Source: "{#BuildOutputDir}\Razeru.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\RzruUI.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RepoRoot}\config\Razeru.json"; DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "{#RepoRoot}\Animations\*"; DestDir: "{app}\Animations"; Flags: recursesubdirs createallsubdirs onlyifdoesntexist uninsneveruninstall
Source: "{#RepoRoot}\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "{#RepoRoot}\docs\*"; DestDir: "{app}\docs"; Excludes: "RELEASE_NOTES_*"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#VCRedistPath}"; DestDir: "{tmp}"; Flags: deleteafterinstall

[InstallDelete]
Type: files; Name: "{app}\CChromaEditorLibrary.dll"
Type: files; Name: "{app}\CChromaEditorLibrary64.dll"
Type: files; Name: "{app}\docs\RELEASE_NOTES_*"

[Icons]
Name: "{group}\Razeru 2"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"
Name: "{group}\{cm:DocumentationShortcut}"; Filename: "{app}\docs\README.md"
Name: "{group}\{cm:UninstallShortcut}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Razeru 2"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "Razeru NDR Co"; ValueData: """{app}\{#AppExeName}"""; Flags: uninsdeletevalue; Tasks: startup

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "{cm:RuntimeStatus}"; Flags: waituntilterminated runhidden; Check: VCRuntimeNeedsInstall
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent unchecked

[Code]
function VCRuntimeNeedsInstall: Boolean;
var
  Installed: Cardinal;
  Build: Cardinal;
begin
  Result := not (
    RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) and
    RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Bld', Build) and
    (Installed = 1) and
    (Build >= 35211)
  );
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    RegDeleteValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', 'Razeru NDR Co');
end;
