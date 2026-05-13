<#
.SYNOPSIS
    Run the pong-sdl3-cpp test suite via CTest on Windows.

.DESCRIPTION
    Thin wrapper around CTest. Assumes the project has already been
    configured and built (for example via scripts\build.ps1). Fails fast
    with a clear message if the build directory does not exist, instead
    of silently triggering a build.

    Extra arguments are forwarded verbatim to CTest, so options like
    -R <regex>, -j <N>, --rerun-failed, etc. work without script changes.

.PARAMETER Config
    Build configuration to run tests for. Defaults to Debug. Required for
    multi-config generators (Visual Studio, Xcode); harmless for single-
    config generators (Ninja, Makefiles).

.PARAMETER BuildDir
    Build directory. Defaults to "build" at the repository root.

.PARAMETER ExtraArgs
    Extra arguments forwarded verbatim to ctest.

.EXAMPLE
    scripts\test.ps1

.EXAMPLE
    scripts\test.ps1 -Config Release

.EXAMPLE
    scripts\test.ps1 -ExtraArgs '-R','smoke','-j','4'
#>

[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Config = 'Debug',

    [string]$BuildDir = 'build',

    [string[]]$ExtraArgs
)

$ErrorActionPreference = 'Stop'

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

if (-not (Test-Path -LiteralPath $buildPath)) {
    Write-Error "Build directory not found: $buildPath. Build the project first (for example: scripts\build.ps1)."
    exit 1
}

# --no-tests=error turns CTest's silent "No tests were found!!!" exit
# code 0 into a real failure. Without it, a build directory configured
# with BUILD_TESTING=OFF or left in a partial state would report
# "Tests passed" while running zero tests. Listed before $ExtraArgs so
# a user can still opt out via -ExtraArgs '--no-tests=ignore'.
$ctestArgs = @('--test-dir', $buildPath, '--output-on-failure', '--no-tests=error', '-C', $Config)

if ($ExtraArgs) {
    $ctestArgs += $ExtraArgs
}

Write-Host "Repository root : $repoRoot"
Write-Host "Build directory : $buildPath"
Write-Host "Configuration   : $Config"
Write-Host ''

Invoke-Native ctest @ctestArgs

Write-Host ''
Write-Host "Tests passed ($Config)." -ForegroundColor Green
