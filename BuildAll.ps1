# Build All - Complete build and installer creation

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PSD Thumbnail Provider" -ForegroundColor Cyan
Write-Host "  Complete Build Process" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Build DLL
Write-Host "Step 1: Building DLL..." -ForegroundColor Cyan
Write-Host ""
& .\Build.ps1

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed! Aborting." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 2: Creating Installer..." -ForegroundColor Cyan
Write-Host ""

# Step 2: Build Installer
Set-Location installer
& .\BuildInstaller.ps1

if ($LASTEXITCODE -ne 0) {
    Set-Location ..
    Write-Host ""
    Write-Host "Installer creation failed!" -ForegroundColor Red
    exit 1
}

Set-Location ..

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  ALL DONE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "The installer is ready at:" -ForegroundColor Yellow
Write-Host "  .\installer\PSD_Thumbnail_Installer.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "You can now use this installer!" -ForegroundColor Green
Write-Host ""
