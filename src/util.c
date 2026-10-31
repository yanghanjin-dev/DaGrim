#include "app.h"

void *xmalloc(size_t n)
{
    if (n == 0) n = 1;
    void *p = malloc(n);
    if (!p) {
        MessageBoxW(NULL, L"메모리가 부족합니다.", L"DaGrim", MB_ICONERROR);
        exit(1);
    }
    return p;
}

void *xcalloc(size_t n, size_t sz)
{
    size_t bytes;
    if (n == 0 || sz == 0) return xmalloc(1);
    if (sz != 0 && n > (size_t)-1 / sz) {
        MessageBoxW(NULL, L"메모리가 부족합니다.", L"DaGrim", MB_ICONERROR);
        exit(1);
    }
    bytes = n * sz;
    void *p = calloc(n, sz);
    if (!p) {
        MessageBoxW(NULL, L"메모리가 부족합니다.", L"DaGrim", MB_ICONERROR);
        exit(1);
    }
    (void)bytes;
    return p;
}

int clampi(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

uint8_t clampu8(int v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

int wide_to_utf8(const wchar_t *s, char *o, int on)
{
    return WideCharToMultiByte(CP_UTF8, 0, s, -1, o, on, NULL, NULL);
}

int utf8_to_wide(const char *s, wchar_t *o, int on)
{
    return MultiByteToWideChar(CP_UTF8, 0, s, -1, o, on);
}

int read_file(const wchar_t *path, uint8_t **data, size_t *len)
{
    FILE *f = _wfopen(path, L"rb");
    long sz;
    uint8_t *buf;
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return 0; }
    rewind(f);
    buf = xmalloc((size_t)sz + 1);
    if (sz > 0 && fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return 0;
    }
    buf[sz] = 0;
    fclose(f);
    *data = buf;
    *len = (size_t)sz;
    return 1;
}

int write_file(const wchar_t *path, const uint8_t *data, size_t len)
{
    FILE *f = _wfopen(path, L"wb");
    size_t n;
    if (!f) return 0;
    n = fwrite(data, 1, len, f);
    fclose(f);
    return n == len;
}

const wchar_t *path_ext(const wchar_t *path)
{
    const wchar_t *dot = NULL;
    if (!path) return L"";
    for (; *path; path++) {
        if (*path == L'\\' || *path == L'/') dot = NULL;
        else if (*path == L'.') dot = path;
    }
    return dot ? dot : L"";
}

const wchar_t *path_filename(const wchar_t *path)
{
    const wchar_t *base = path;
    if (!path || !path[0]) return L"제목 없음";
    for (; *path; path++) {
        if (*path == L'\\' || *path == L'/') base = path + 1;
    }
    return base[0] ? base : L"제목 없음";
}

int ext_eq(const wchar_t *path, const wchar_t *ext)
{
    return _wcsicmp(path_ext(path), ext) == 0;
}

UINT dpi_for(HWND h)
{
    typedef UINT (WINAPI *GetDpiForWindowFn)(HWND);
    static GetDpiForWindowFn fn;
    static int once;
    if (!once) {
        HMODULE user = GetModuleHandleW(L"user32.dll");
        if (user) fn = (GetDpiForWindowFn)GetProcAddress(user, "GetDpiForWindow");
        once = 1;
    }
    if (fn && h) return fn(h);
    return 96;
}

int ui_scale(int px)
{
    int dpi = g.dpi > 0 ? g.dpi : 96;
    return MulDiv(px, dpi, 96);
}

int edit_focused(void)
{
    HWND f = GetFocus();
    wchar_t cls[32];
    if (!f) return 0;
    GetClassNameW(f, cls, 32);
    return _wcsicmp(cls, L"Edit") == 0;
}
