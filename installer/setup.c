#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include "version.h"

static BOOL extract(WORD id, const wchar_t *path)
{
    HRSRC r = FindResourceW(NULL, MAKEINTRESOURCEW(id), RT_RCDATA);
    HGLOBAL data;
    HANDLE file;
    DWORD size, written;
    BOOL ok;
    if (!r || !(data = LoadResource(NULL, r))) return FALSE;
    size = SizeofResource(NULL, r);
    file = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return FALSE;
    ok = WriteFile(file, LockResource(data), size, &written, NULL) && written == size;
    CloseHandle(file);
    return ok;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    wchar_t self[MAX_PATH], temp[MAX_PATH], dir[MAX_PATH], app[MAX_PATH], script[MAX_PATH];
    wchar_t powershell[MAX_PATH], line[4096];
    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    DWORD code = 1;
    BOOL uninstall = !lstrcmpiW(cmd, L"/uninstall");
    (void)inst; (void)prev; (void)show;
    if (*cmd && !uninstall) return 2;
    if (!GetModuleFileNameW(NULL, self, MAX_PATH) ||
        !GetSystemDirectoryW(powershell, MAX_PATH)) return 1;
    wcscat(powershell, L"\\WindowsPowerShell\\v1.0\\powershell.exe");
    if (uninstall) {
        wcscpy(script, self);
        wchar_t *slash = wcsrchr(script, L'\\');
        if (!slash) return 1;
        wcscpy(slash + 1, L"maintenance.ps1");
        swprintf(line, 4096, L"\"%ls\" -NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -File \"%ls\" -Uninstall -ParentPid %lu", powershell, script, GetCurrentProcessId());
        if (!CreateProcessW(powershell, line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) goto error;
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        return 0;
    }
    if (!GetTempPathW(MAX_PATH, temp) || !GetTempFileNameW(temp, L"dgr", 0, dir)) goto error;
    DeleteFileW(dir);
    if (!CreateDirectoryW(dir, NULL)) goto error;
    swprintf(app, MAX_PATH, L"%ls\\DaGrim.exe", dir);
    swprintf(script, MAX_PATH, L"%ls\\maintenance.ps1", dir);
    if (!extract(101, app) || !extract(102, script)) goto cleanup;
    swprintf(line, 4096, L"\"%ls\" -NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -File \"%ls\" -SetupPath \"%ls\" -Version \"%ls\"", powershell, script, self, DAGRIM_VERSION_WSTRING);
    if (!CreateProcessW(powershell, line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) goto cleanup;
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
cleanup:
    DeleteFileW(app); DeleteFileW(script); RemoveDirectoryW(dir);
    if (!code) return 0;
error:
    MessageBoxW(NULL, L"설치를 완료하지 못했습니다. 실행 중인 DaGrim을 종료한 뒤 다시 시도하세요.", L"DaGrim 설치", MB_OK | MB_ICONERROR);
    return 1;
}
