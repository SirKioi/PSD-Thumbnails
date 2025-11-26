#include <windows.h>
#include <shlwapi.h>
#include "ClassFactory.h"
#include "PSDThumbnailProvider.h"

HINSTANCE g_hInst = nullptr;
long g_cDllRef = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_PSDThumbnailProvider, rclsid))
    {
        ClassFactory* pClassFactory = new (std::nothrow) ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;
    HKEY hKey = nullptr;
    HKEY hSubKey = nullptr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileNameW(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Create CLSID key
    LONG result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Classes\\CLSID\\{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}",
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    
    if (result == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, nullptr, 0, REG_SZ, 
            (LPBYTE)L"PSD Thumbnail Provider", 
            (DWORD)((wcslen(L"PSD Thumbnail Provider") + 1) * sizeof(wchar_t)));
        
        // Create InProcServer32 subkey
        result = RegCreateKeyExW(hKey, L"InProcServer32", 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);
        if (result == ERROR_SUCCESS)
        {
            RegSetValueExW(hSubKey, nullptr, 0, REG_SZ, 
                (LPBYTE)szModule, 
                (DWORD)((wcslen(szModule) + 1) * sizeof(wchar_t)));
            
            RegSetValueExW(hSubKey, L"ThreadingModel", 0, REG_SZ, 
                (LPBYTE)L"Apartment", 
                (DWORD)((wcslen(L"Apartment") + 1) * sizeof(wchar_t)));
            
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }
    
    if (result != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(result);
        return hr;
    }

    // Register in Approved Shell Extensions
    result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved",
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    
    if (result == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, L"{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}", 0, REG_SZ, 
            (LPBYTE)L"PSD Thumbnail Provider", 
            (DWORD)((wcslen(L"PSD Thumbnail Provider") + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
    }

    // Register for .psd files
    result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Classes\\.psd\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96}",
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    
    if (result == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, nullptr, 0, REG_SZ, 
            (LPBYTE)L"{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}", 
            (DWORD)((wcslen(L"{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}") + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

    // Unregister the component
    RegDeleteTreeW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\CLSID\\{C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}");

    return hr;
}
