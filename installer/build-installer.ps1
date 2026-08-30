[CmdletBinding()]
param(
    [string]$AppVersion = '2.0.0.0',
    [switch]$SkipBuild,
    [string]$SignToolName = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$cacheDirectory = Join-Path $PSScriptRoot '.cache'
$redistPath = Join-Path $cacheDirectory 'vc_redist.x64.exe'
$redistUrl = 'https://aka.ms/vs/17/release/vc_redist.x64.exe'
$redistSha256 = 'CC0FF0EB1DC3F5188AE6300FAEF32BF5BEEBA4BDD6E8E445A9184072096B713B'

if (-not $SkipBuild) {
    & (Join-Path $repoRoot 'scripts\build.ps1') -Configuration Release -Platform x64
}

$buildOutput = Join-Path $repoRoot 'x64\Release'
$requiredFiles = @(
    (Join-Path $buildOutput 'Razeru.exe'),
    (Join-Path $buildOutput 'RzruUI.dll'),
    (Join-Path $repoRoot 'config\Razeru.json')
)
foreach ($file in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $file)) {
        throw "Required installer payload is missing: $file"
    }
}

New-Item -ItemType Directory -Force -Path $cacheDirectory | Out-Null
if (-not (Test-Path -LiteralPath $redistPath)) {
    Write-Host "Downloading Microsoft Visual C++ Redistributable..."
    Invoke-WebRequest -Uri $redistUrl -OutFile $redistPath
}

$actualRedistHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $redistPath).Hash
if ($actualRedistHash -ne $redistSha256) {
    throw "Unexpected VC++ Redistributable SHA-256: $actualRedistHash"
}

$redistSignature = Get-AuthenticodeSignature -LiteralPath $redistPath
if ($redistSignature.Status -ne 'Valid' -or $redistSignature.SignerCertificate.Subject -notlike '*Microsoft Corporation*') {
    throw 'The Microsoft VC++ Redistributable signature is not valid.'
}

$isccCandidates = @(
    (Join-Path $env:LOCALAPPDATA 'Programs\Inno\ISCC.exe'),
    (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe')
)
$iscc = $isccCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $iscc) {
    throw 'ISCC.exe was not found. Install Inno Setup 6.7 or newer.'
}

$scriptPath = Join-Path $PSScriptRoot 'Razeru.iss'
$isccArguments = @(
    '/Qp',
    "/DAppVersion=$AppVersion",
    "/DBuildOutputDir=$buildOutput",
    "/DVCRedistPath=$redistPath"
)
if ($SignToolName) {
    $isccArguments += "/DSignToolName=$SignToolName"
}
$isccArguments += $scriptPath

& $iscc @isccArguments
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup compilation failed with exit code $LASTEXITCODE."
}

$setupPath = Join-Path $repoRoot "dist\Razeru-Setup-$AppVersion-x64.exe"
if (-not (Test-Path -LiteralPath $setupPath)) {
    throw "Expected setup executable was not created: $setupPath"
}

$setupHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $setupPath).Hash
Write-Host "Installer: $setupPath"
Write-Host "SHA-256:  $setupHash"
