# UME Development Environment Bootstrap (Windows)
# Run: .\scripts\bootstrap.ps1
#
# This script sets up a complete UME development environment from scratch.
# Prerequisites: Git, CMake 3.28+, Visual Studio 2022 (or Build Tools), Python 3.10+

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  UME Development Environment Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ── Check Prerequisites ──
function Test-Command($name) {
    return [bool](Get-Command $name -ErrorAction SilentlyContinue)
}

Write-Host "[1/6] Checking prerequisites..." -ForegroundColor Yellow

if (-not (Test-Command "cmake")) {
    Write-Host "  ERROR: cmake not found. Install CMake 3.28+ from https://cmake.org/download/" -ForegroundColor Red
    exit 1
}
$cmakeVersion = (cmake --version | Select-String -Pattern "\d+\.\d+\.\d+").Matches[0].Value
Write-Host "  cmake: $cmakeVersion" -ForegroundColor Green

if (-not (Test-Command "git")) {
    Write-Host "  ERROR: git not found. Install Git from https://git-scm.com/downloads" -ForegroundColor Red
    exit 1
}
Write-Host "  git: $(git --version)" -ForegroundColor Green

if (-not (Test-Command "python")) {
    Write-Host "  WARNING: python not found. Python bindings will not be available." -ForegroundColor Yellow
} else {
    Write-Host "  python: $(python --version)" -ForegroundColor Green
}

# Check for MSVC
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    Write-Host "  Visual Studio: $vsPath" -ForegroundColor Green
} else {
    Write-Host "  WARNING: Visual Studio not found. MSVC builds may not work." -ForegroundColor Yellow
}

# ── Install vcpkg ──
Write-Host ""
Write-Host "[2/6] Setting up vcpkg..." -ForegroundColor Yellow

$vcpkgRoot = Join-Path $PSScriptRoot "..\vcpkg"
if (-not (Test-Path $vcpkgRoot)) {
    git clone https://github.com/microsoft/vcpkg.git $vcpkgRoot
    & "$vcpkgRoot\bootstrap-vcpkg.bat" -disableMetrics
} else {
    Write-Host "  vcpkg already present at $vcpkgRoot" -ForegroundColor Green
}

$env:VCPKG_ROOT = (Resolve-Path $vcpkgRoot).Path
Write-Host "  VCPKG_ROOT=$env:VCPKG_ROOT" -ForegroundColor Green

# ── Install Ninja ──
Write-Host ""
Write-Host "[3/6] Checking Ninja build system..." -ForegroundColor Yellow

if (-not (Test-Command "ninja")) {
    Write-Host "  Installing Ninja via winget..." -ForegroundColor Yellow
    winget install Ninja-build.Ninja --accept-source-agreements --accept-package-agreements 2>$null
    if (-not (Test-Command "ninja")) {
        Write-Host "  WARNING: Ninja install failed. CMake will use default generator." -ForegroundColor Yellow
    }
} else {
    Write-Host "  ninja: $(ninja --version)" -ForegroundColor Green
}

# ── Configure Build ──
Write-Host ""
Write-Host "[4/6] Configuring CMake build..." -ForegroundColor Yellow

$buildDir = Join-Path $PSScriptRoot "..\build"

cmake -B $buildDir `
    -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DUME_BUILD_TESTS=ON `
    -DUME_BUILD_BENCHMARKS=OFF `
    -DUME_BUILD_EXAMPLES=OFF `
    -S (Join-Path $PSScriptRoot "..")

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ERROR: CMake configure failed." -ForegroundColor Red
    exit 1
}
Write-Host "  CMake configure: OK" -ForegroundColor Green

# ── Build ──
Write-Host ""
Write-Host "[5/6] Building UME..." -ForegroundColor Yellow

cmake --build $buildDir --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "  ERROR: Build failed." -ForegroundColor Red
    exit 1
}
Write-Host "  Build: OK" -ForegroundColor Green

# ── Run Tests ──
Write-Host ""
Write-Host "[6/6] Running tests..." -ForegroundColor Yellow

Push-Location $buildDir
ctest --output-on-failure --timeout 30
$testResult = $LASTEXITCODE
Pop-Location

if ($testResult -ne 0) {
    Write-Host "  WARNING: Some tests failed." -ForegroundColor Yellow
} else {
    Write-Host "  Tests: ALL PASSED" -ForegroundColor Green
}

# ── Done ──
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  UME development environment is ready!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Build directory: $buildDir" -ForegroundColor White
Write-Host "  To rebuild:      cmake --build $buildDir" -ForegroundColor White
Write-Host "  To test:         cd $buildDir && ctest" -ForegroundColor White
Write-Host ""
