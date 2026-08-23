[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'Debug_NoSignatureCheck', 'Release_NoSignatureCheck')]
    [string]$Configuration = 'Release',

    [ValidateSet('x64', 'x86', 'Win32')]
    [string]$Platform = 'x64',

    [switch]$Analyze,
    [switch]$NoRebuild
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $repoRoot 'CSDK_SampleApp.sln'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer (vswhere.exe) was not found. Install Visual Studio Build Tools 2022 with the C++ workload.'
}

$installationPath = & $vswhere `
    -latest `
    -products '*' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $installationPath) {
    throw 'MSVC x86/x64 build tools were not found.'
}

$msbuild = Join-Path $installationPath 'MSBuild\Current\Bin\MSBuild.exe'
$target = if ($NoRebuild) { 'Build' } else { 'Rebuild' }
$solutionPlatform = if ($Platform -eq 'Win32') { 'x86' } else { $Platform }
$arguments = @(
    $solution,
    '/m',
    "/t:$target",
    "/p:Configuration=$Configuration",
    "/p:Platform=$solutionPlatform",
    '/verbosity:minimal',
    '/nologo'
)

if ($Analyze) {
    $arguments += '/p:RunCodeAnalysis=true'
    $arguments += '/p:EnableCppCoreCheck=true'
}

Write-Host "Building $Configuration|$solutionPlatform with $msbuild"
& $msbuild @arguments
if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE."
}

$platformDirectory = if ($solutionPlatform -eq 'x86') { $Configuration } else { Join-Path $solutionPlatform $Configuration }
$outputDirectory = Join-Path $repoRoot $platformDirectory
Write-Host "Build completed: $outputDirectory"
