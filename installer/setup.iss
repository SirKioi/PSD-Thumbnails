; PSD Thumbnail Provider - Inno Setup Installer Script
; Creates a professional Windows installer with wizard interface

#define MyAppName "PSD Thumbnail Provider"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "PSD Thumbnail Provider"
#define MyAppURL "https://github.com/yourusername/psd-thumbnail-provider"
#define CLSID "{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}"

[Setup]
AppId={{A8B9C1D2-E3F4-5A6B-7C8D-9E0F1A2B3C4D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=LICENSE.txt
OutputDir=.
OutputBaseFilename=PSD_Thumbnail_Installer
;;;;;;;SetupIconFile=..\icon.ico
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\PSDThumbnailProvider.dll
CreateUninstallRegKey=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\build\PSDThumbnailProvider.dll"; DestDir: "{app}"; Flags: ignoreversion regserver 64bit

[Registry]
; Shell extension approval
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved"; ValueType: string; ValueName: "{{#CLSID}}"; ValueData: "PSD Thumbnail Provider"; Flags: uninsdeletevalue

; .psd file thumbnail handler
Root: HKLM; Subkey: "SOFTWARE\Classes\.psd\ShellEx\{{e357fccd-a995-4576-b01f-234630154e96}}"; ValueType: string; ValueData: "{{#CLSID}}"; Flags: uninsdeletevalue

[Code]
var
  CSPPathPage: TInputDirWizardPage;
  AssociateCSPCheckbox: TNewCheckBox;

procedure InitializeWizard;
begin
  // Create custom page for CSP association
  CSPPathPage := CreateInputDirPage(wpSelectDir,
    'Clip Studio Paint Association', 'Would you like to set Clip Studio Paint as the default program for .psd files?',
    'Select the folder where Clip Studio Paint is installed, then click Next. If you don''t want to associate CSP, leave unchecked and click Next.',
    False, '');
  
  CSPPathPage.Add('');
  CSPPathPage.Values[0] := 'C:\Program Files\CELSYS\CLIP STUDIO 1.5\CLIP STUDIO PAINT';
  
  // Add checkbox to enable/disable association
  AssociateCSPCheckbox := TNewCheckBox.Create(CSPPathPage);
  AssociateCSPCheckbox.Parent := CSPPathPage.Surface;
  AssociateCSPCheckbox.Caption := 'Associate .psd files with Clip Studio Paint';
  AssociateCSPCheckbox.Left := 0;
  AssociateCSPCheckbox.Top := 100;
  AssociateCSPCheckbox.Width := CSPPathPage.SurfaceWidth;
  AssociateCSPCheckbox.Height := ScaleY(17);
  AssociateCSPCheckbox.Checked := False;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;
end;

procedure RegisterThumbnailProvider;
var
  ResultCode: Integer;
  DllPath: String;
begin
  DllPath := ExpandConstant('{app}\PSDThumbnailProvider.dll');
  
  // Register the DLL
  Exec('regsvr32.exe', '/s "' + DllPath + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

procedure UnregisterThumbnailProvider;
var
  ResultCode: Integer;
  DllPath: String;
begin
  DllPath := ExpandConstant('{app}\PSDThumbnailProvider.dll');
  
  // Unregister the DLL
  Exec('regsvr32.exe', '/u /s "' + DllPath + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

procedure AssociateCSP;
var
  CSPPath: String;
  CSPExe: String;
begin
  if not AssociateCSPCheckbox.Checked then
    Exit;
    
  CSPPath := CSPPathPage.Values[0];
  CSPExe := CSPPath + '\CLIPStudioPaint.exe';
  
  if not FileExists(CSPExe) then
  begin
    MsgBox('Clip Studio Paint executable not found at:' + #13#10 + CSPExe + #13#10#13#10 + 'Skipping file association.', mbError, MB_OK);
    Exit;
  end;
  
  // Register file association
  RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Classes\.psd', '', 'Photoshop.Image');
  RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Classes\Photoshop.Image', '', 'Adobe Photoshop Image');
  RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Classes\Photoshop.Image\DefaultIcon', '', CSPExe + ',0');
  RegWriteStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Classes\Photoshop.Image\shell\open\command', '', '"' + CSPExe + '" "%1"');
end;

procedure ClearThumbnailCache;
var
  ResultCode: Integer;
  CacheFolder: String;
begin
  CacheFolder := ExpandConstant('{localappdata}\Microsoft\Windows\Explorer');
  
  // Delete thumbnail cache files
  Exec('cmd.exe', '/c del /f /q "' + CacheFolder + '\thumbcache_*.db"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

procedure RestartExplorer;
var
  ResultCode: Integer;
  ExplorerPath: String;
begin
  ExplorerPath := ExpandConstant('{win}\explorer.exe');
  
  // Kill Explorer
  Exec('taskkill.exe', '/F /IM explorer.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Sleep(2000);
  
  // Start Explorer using cmd to ensure it starts
  Exec('cmd.exe', '/c start "" "' + ExplorerPath + '"', '', SW_HIDE, ewNoWait, ResultCode);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // Associate CSP if checkbox is checked
    AssociateCSP;
    
    // Clear thumbnail cache (don't restart Explorer yet)
    ClearThumbnailCache;
  end;
end;

var
  RestartExplorerCheckbox: TNewCheckBox;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpFinished then
  begin
    // Create checkbox on finish page to restart Explorer
    RestartExplorerCheckbox := TNewCheckBox.Create(WizardForm);
    RestartExplorerCheckbox.Parent := WizardForm.FinishedPage;
    RestartExplorerCheckbox.Caption := 'Restart Windows Explorer to apply changes';
    RestartExplorerCheckbox.Left := WizardForm.FinishedLabel.Left;
    RestartExplorerCheckbox.Top := WizardForm.FinishedLabel.Top + WizardForm.FinishedLabel.Height + ScaleY(20);
    RestartExplorerCheckbox.Width := WizardForm.FinishedLabel.Width;
    RestartExplorerCheckbox.Height := ScaleY(17);
    RestartExplorerCheckbox.Checked := True;
  end;
end;

procedure DeinitializeSetup();
begin
  // If user checked the restart Explorer checkbox
  if Assigned(RestartExplorerCheckbox) and RestartExplorerCheckbox.Checked then
  begin
    RestartExplorer;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Response: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    // Unregister the DLL before files are deleted
    UnregisterThumbnailProvider;
  end;
  
  if CurUninstallStep = usPostUninstall then
  begin
    // Clear thumbnail cache after uninstallation
    ClearThumbnailCache;
    
    // Ask if user wants to restart Explorer
    Response := MsgBox('Restart Windows Explorer to complete uninstallation?' + #13#10#13#10 + 'This is recommended to clear thumbnail cache.', mbConfirmation, MB_YESNO);
    if Response = IDYES then
    begin
      RestartExplorer;
    end;
  end;
end;







