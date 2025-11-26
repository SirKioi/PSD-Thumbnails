# Build Installer Script

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PSD Thumbnail Provider - Installer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if DLL exists
Write-Host "[1/3] Checking for DLL..." -ForegroundColor Yellow
$dllPath = Join-Path $PSScriptRoot "..\build\PSDThumbnailProvider.dll"
if (-not (Test-Path $dllPath)) {
    Write-Host "ERROR: DLL not found at $dllPath" -ForegroundColor Red
    Write-Host "Please build the project first using Build.ps1" -ForegroundColor Yellow
    exit 1
}
Write-Host "      OK" -ForegroundColor Green

# Check if Inno Setup is installed
Write-Host "[2/3] Checking for Inno Setup..." -ForegroundColor Yellow
$innoSetupPaths = @(
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
)

$isccPath = $innoSetupPaths | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $isccPath) {
    Write-Host "ERROR: Inno Setup not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Inno Setup from: https://jrsoftware.org/isdl.php" -ForegroundColor Yellow
    Write-Host "Download: Inno Setup 6 (innosetup-6.x.x.exe)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Opening download page..."
    Start-Process "https://jrsoftware.org/isdl.php"
    exit 1
}
Write-Host "      OK" -ForegroundColor Green

# Check for LICENSE.txt
$licensePath = Join-Path $PSScriptRoot "LICENSE.txt"
if (-not (Test-Path $licensePath)) {
    Write-Host "Creating LICENSE.txt..." -ForegroundColor Yellow
    $licenseContent = @"
MIT License

Copyright (c) 2025 PSD Thumbnail Provider

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"@
    Set-Content -Path $licensePath -Value $licenseContent
}

# Comment out icon line if icon doesn't exist
$iconPath = Join-Path $PSScriptRoot "..\icon.ico"
if (-not (Test-Path $iconPath)) {
    $issPath = Join-Path $PSScriptRoot "setup.iss"
    $issContent = Get-Content $issPath -Raw
    if ($issContent -notmatch ';+SetupIconFile') {
        $issContent = $issContent -replace [regex]::Escape('SetupIconFile=..\icon.ico'), ';SetupIconFile=..\icon.ico'
        Set-Content -Path $issPath -Value $issContent
    }
}

# Build installer
Write-Host "[3/3] Building installer..." -ForegroundColor Yellow

$issFile = Join-Path $PSScriptRoot "setup.iss"
$originalDir = Get-Location
Set-Location $PSScriptRoot

& $isccPath $issFile 2>&1 | Out-Null

Set-Location $originalDir

if ($LASTEXITCODE -eq 0) {
    $installerPath = Join-Path $PSScriptRoot "PSD_Thumbnail_Installer.exe"
    
    Write-Host "      OK" -ForegroundColor Green
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  Installer Built Successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Location: .\installer\PSD_Thumbnail_Installer.exe" -ForegroundColor Yellow
    
    if (Test-Path $installerPath) {
        $fileSize = (Get-Item $installerPath).Length / 1MB
        Write-Host "Size: $([math]::Round($fileSize, 2)) MB" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Write-Host "To install:" -ForegroundColor Cyan
    Write-Host "  1. Double-click PSD_Thumbnail_Installer.exe" -ForegroundColor Gray
    Write-Host "  2. Follow the installation wizard" -ForegroundColor Gray
    Write-Host "  3. Check 'Restart Windows Explorer' on finish page" -ForegroundColor Gray
    Write-Host ""
} else {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}
