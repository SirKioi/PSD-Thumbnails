# PSD Thumbnail Provider for Windows

Display PSD file thumbnails directly in Windows File Explorer with this lightweight thumbnail handler.

## Features

- 🖼️ Shows PSD thumbnails in Windows Explorer
- ⚡ Fast rendering using RLE decompression
- 🎨 Works with any default .psd handler (Photoshop, Clip Studio Paint, Krita, etc.)
- 🔧 Easy installation with GUI wizard
- 💯 Native Windows integration using COM

## Installation

1. Download `PSD_Thumbnail_Installer.exe` from the releases
2. Run the installer as Administrator
3. Follow the installation wizard
4. On the finish page, check "Restart Windows Explorer" (checked by default)
5. Open File Explorer and view .psd files in Large Icons or Extra Large Icons mode

## Building from Source

### Requirements

- Windows 10/11
- Visual Studio 2019 or later (or Build Tools) with C++ development tools
- CMake 3.15 or later
- Inno Setup 6 (for creating the installer)

### Build Steps

1. **Build the DLL:**
   ```powershell
   .\Build.ps1
   ```
   This creates `build\PSDThumbnailProvider.dll`

2. **Create the installer:**
   ```powershell
   cd installer
   .\BuildInstaller.ps1
   ```
   This creates `installer\PSD_Thumbnail_Installer.exe`

## Uninstallation

1. Open Windows Settings → Apps → Installed apps
2. Find "PSD Thumbnail Provider"
3. Click Uninstall
4. Follow the prompts

## How It Works

This thumbnail provider:
- Registers as a Windows COM server
- Implements `IThumbnailProvider` and `IInitializeWithStream` interfaces
- Parses PSD files to extract image data
- Supports RLE/PackBits compression (compression method 1)
- Renders thumbnails from composite image data
- Uses GDI+ for high-quality scaling

## Technical Details

### Supported PSD Features

- Color Mode: RGB (mode 3)
- Channels: 1-4 (R, G, B, and optional Alpha)
- Compression: Uncompressed (0) and RLE (1)
- Maximum size: 4096x4096 pixels

### Registry Keys

The installer creates these registry entries:

- **CLSID:** `HKLM\SOFTWARE\Classes\CLSID\{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}`
- **Shell Extension Approval:** `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved`
- **Thumbnail Handler:** `HKLM\SOFTWARE\Classes\.psd\ShellEx\{e357fccd-a995-4576-b01f-234630154e96}`

## License

MIT License - See LICENSE.txt for details

## Credits

**This project was 100% written with GitHub Copilot** - an AI pair programmer that assisted in generating all the code, from the C++ thumbnail provider implementation to the Windows installer scripts.

Developed using:
- C++17
- Windows COM/Shell Extensions API
- GDI+ for image processing
- Inno Setup for installer
- GitHub Copilot (AI-assisted development)

## Troubleshooting

**Thumbnails not showing?**
- Make sure you're using Large Icons or Extra Large Icons view
- Clear thumbnail cache: Delete `%localappdata%\Microsoft\Windows\Explorer\thumbcache_*.db`
- Restart Windows Explorer via Task Manager

**Installation failed?**
- Make sure you run the installer as Administrator
- Check if another thumbnail provider is conflicting

**Thumbnails appear corrupted?**
- Some PSD files may use unsupported features
- Try re-saving the PSD with "Maximize Compatibility" enabled
