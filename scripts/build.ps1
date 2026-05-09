<#
.SYNOPSIS
    Configure and build pong-sdl3-cpp on Windows.

.DESCRIPTION
    Thin wrapper around CMake that performs an out-of-source build.
    On Windows the default generator is Visual Studio (latest installed
    version detected via vswhere). Pass -Generator to override, for
    example to use Ninja.

.PARAMETER Config
    Build configuration. Defaults to Debug. Common values: Debug, Release,
    RelWithDebInfo, MinSizeRel.

.PARAMETER BuildDir
    Build directory. Defaults to "build" at the repository root.

.PARAMETER Generator
    CMake generator to use. Defaults to the latest installed Visual Studio.

.PARAMETER Arch
    Visual Studio architecture (-A flag). Defaults to x64. Ignored for
    non Visual Studio generators.

.PARAMETER Clean
    Remove the build directory before configuring.

.PARAMETER ConfigureOnly
    Only run the CMake configure step, skip the build step.

.PARAMETER ExtraArgs
    Extra arguments forwarded to the CMake configure step.

.EXAMPLE
    scripts\build.ps1

.EXAMPLE
    scripts\build.ps1 -Config Release

.EXAMPLE
    scripts\build.ps1 -Generator Ninja -Config Release

.EXAMPLE
    scripts\build.ps1 -Clean
#>

[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Config = 'Debug',

    [string]$BuildDir = 'build',

    [string]$Generator,

    [ValidateSet('x64', 'Win32', 'ARM64')]
    [string]$Arch = 'x64',

    [switch]$Clean,

    [switch]$ConfigureOnly,

    [string[]]$ExtraArgs
)

$ErrorActionPreference = 'Stop'

function Get-DefaultVisualStudioGenerator {
    $programFilesX86 = ${env:ProgramFiles(x86)}
    if (-not $programFilesX86) {
        return $null
    }
    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        return $null
    }

    $version = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationVersion 2>$null
    if (-not $version) {
        return $null
    }

    $major = [int]($version -split '\.')[0]
    switch ($major) {
        17 { return 'Visual Studio 17 2022' }
        16 { return 'Visual Studio 16 2019' }
        15 { return 'Visual Studio 15 2017' }
        default { return $null }
    }
}

function Invoke-Native {
    param(
        [Parameter(Mandatory)]
        [string]$Command,

        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    Write-Host "> $Command $($Arguments -join ' ')" -ForegroundColor DarkGray
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command '$Command' exited with code $LASTEXITCODE."
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $repoRoot $BuildDir
}

if ($Clean -and (Test-Path -LiteralPath $buildPath)) {
    Write-Host "Removing build directory: $buildPath" -ForegroundColor Yellow
    Remove-Item -LiteralPath $buildPath -Recurse -Force
}

if (-not $Generator) {
    $Generator = Get-DefaultVisualStudioGenerator
    if (-not $Generator) {
        Write-Warning 'No Visual Studio installation detected. Falling back to the CMake default generator.'
    }
}

$configureArgs = @('-S', $repoRoot, '-B', $buildPath)

if ($Generator) {
    $configureArgs += @('-G', $Generator)
    if ($Generator -like 'Visual Studio*') {
        $configureArgs += @('-A', $Arch)
    } else {
        $configureArgs += "-DCMAKE_BUILD_TYPE=$Config"
    }
} else {
    $configureArgs += "-DCMAKE_BUILD_TYPE=$Config"
}

if ($ExtraArgs) {
    $configureArgs += $ExtraArgs
}

Write-Host "Repository root : $repoRoot"
Write-Host "Build directory : $buildPath"
Write-Host "Configuration   : $Config"
if ($Generator) { Write-Host "Generator       : $Generator" }
if ($Generator -like 'Visual Studio*') { Write-Host "Architecture    : $Arch" }
Write-Host ''

Invoke-Native cmake @configureArgs

if ($ConfigureOnly) {
    Write-Host 'Configure complete. Skipping build (-ConfigureOnly).' -ForegroundColor Green
    exit 0
}

$buildArgs = @('--build', $buildPath, '--config', $Config)
Invoke-Native cmake @buildArgs

Write-Host ''
Write-Host "Build succeeded ($Config)." -ForegroundColor Green
