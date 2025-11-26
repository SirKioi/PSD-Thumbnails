# Build script for PSD Thumbnail Provider

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PSD Thumbnail Provider - Build" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for CMake
Write-Host "[1/4] Checking for CMake..." -ForegroundColor Yellow
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: CMake not found!" -ForegroundColor Red
    Write-Host "Please install CMake and add it to your PATH." -ForegroundColor Yellow
    exit 1
}
Write-Host "      OK" -ForegroundColor Green

# Check for Visual Studio or Build Tools
Write-Host "[2/4] Checking for Visual Studio Build Tools..." -ForegroundColor Yellow
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        $vsVersion = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property catalog_productLineVersion
        Write-Host "      Found: Visual Studio/Build Tools $vsVersion" -ForegroundColor Green
    } else {
        Write-Host "ERROR: Visual Studio or Build Tools with C++ tools not found!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "ERROR: Visual Studio or Build Tools not found!" -ForegroundColor Red
    exit 1
}

# Create build directory
Write-Host "[3/4] Configuring project..." -ForegroundColor Yellow
$buildDir = Join-Path $PSScriptRoot "build"
if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
}
New-Item -Path $buildDir -ItemType Directory | Out-Null

Set-Location $buildDir

# Configure with CMake using Build Tools environment
$vcvarsPath = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vcvarsPath) {
    cmd /c "`"$vcvarsPath`" && cmake .. -G `"NMake Makefiles`" -DCMAKE_BUILD_TYPE=Release" 2>&1 | Out-Null
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host "      OK" -ForegroundColor Green
} else {
    Write-Host "ERROR: vcvars64.bat not found!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host "[4/4] Building DLL..." -ForegroundColor Yellow
try {
    $ErrorActionPreference = "SilentlyContinue"
    cmd /c "`"$vcvarsPath`" && nmake" 2>&1 | Out-Null
    $ErrorActionPreference = "Stop"
} catch {}

# Check if DLL was created
$dllPath = Join-Path $buildDir "PSDThumbnailProvider.dll"
if (-not (Test-Path $dllPath)) {
    Write-Host "ERROR: Build failed - DLL not created!" -ForegroundColor Red
    exit 1
}

Set-Location $PSScriptRoot

Write-Host "      OK" -ForegroundColor Green
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "DLL location: .\build\PSDThumbnailProvider.dll" -ForegroundColor Yellow
Write-Host ""
Write-Host "Next step: Run BuildInstaller.ps1 to create the installer" -ForegroundColor Cyan
