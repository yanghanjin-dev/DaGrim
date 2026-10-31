#include "app.h"

App g;

static HMENU make_menu(void)
{
    HMENU bar = CreateMenu();
    HMENU file = CreatePopupMenu();
    HMENU edit = CreatePopupMenu();
    HMENU layer = CreatePopupMenu();
    HMENU adj = CreatePopupMenu();
    HMENU filt = CreatePopupMenu();
    HMENU tool = CreatePopupMenu();
    HMENU view = CreatePopupMenu();
    HMENU help = CreatePopupMenu();

    AppendMenuW(file, MF_STRING, IDM_FILE_NEW, L"새 문서\tCtrl+N");
    AppendMenuW(file, MF_STRING, IDM_FILE_OPEN, L"열기\tCtrl+O");
    AppendMenuW(file, MF_STRING, IDM_LAYER_FROM_FILE, L"사진 가져오기\tCtrl+I");
    AppendMenuW(file, MF_SEPARATOR, 0, NULL);
    AppendMenuW(file, MF_STRING, IDM_FILE_SAVE, L"저장\tCtrl+S");
    AppendMenuW(file, MF_STRING, IDM_FILE_SAVEAS, L"다른 이름으로 저장\tCtrl+Shift+S");
    AppendMenuW(file, MF_STRING, IDM_FILE_EXPORT, L"내보내기\tCtrl+E");
    AppendMenuW(file, MF_SEPARATOR, 0, NULL);
    AppendMenuW(file, MF_STRING, IDM_FILE_EXIT, L"종료");

    AppendMenuW(edit, MF_STRING, IDM_EDIT_UNDO, L"실행 취소\tCtrl+Z");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_REDO, L"다시 실행\tCtrl+Y");
    AppendMenuW(edit, MF_SEPARATOR, 0, NULL);
    AppendMenuW(edit, MF_STRING, IDM_EDIT_CUT, L"잘라내기\tCtrl+X");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_COPY, L"복사\tCtrl+C");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_PASTE, L"붙여넣기\tCtrl+V");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_CLEAR, L"지우기\tDelete");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_FILL, L"전경색으로 채우기");
    AppendMenuW(edit, MF_SEPARATOR, 0, NULL);
    AppendMenuW(edit, MF_STRING, IDM_EDIT_SELALL, L"모두 선택\tCtrl+A");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_DESEL, L"선택 해제\tCtrl+D");
    AppendMenuW(edit, MF_STRING, IDM_EDIT_INVSEL, L"선택 반전\tCtrl+Shift+I");

    AppendMenuW(edit, MF_STRING, IDM_SEL_EXPAND, L"선택 영역 확대...");
    AppendMenuW(edit, MF_STRING, IDM_SEL_CONTRACT, L"선택 영역 축소...");
    AppendMenuW(edit, MF_STRING, IDM_SEL_FEATHER, L"선택 영역 페더...");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_NEW, L"새 레이어");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_FROM_FILE, L"사진 가져오기\tCtrl+I");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_DUP, L"레이어 복제");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_DEL, L"레이어 삭제");
    AppendMenuW(layer, MF_SEPARATOR, 0, NULL);
    AppendMenuW(layer, MF_STRING, IDM_LAYER_UP, L"한 단계 위");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_DOWN, L"한 단계 아래");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_RENAME, L"이름 바꾸기");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_SCALE, L"크기 조절...\tCtrl+T");
    AppendMenuW(layer, MF_SEPARATOR, 0, NULL);
    AppendMenuW(layer, MF_STRING, IDM_LAYER_MERGE, L"아래 레이어와 병합");
    AppendMenuW(layer, MF_STRING, IDM_LAYER_FLAT, L"이미지 병합");

    AppendMenuW(adj, MF_STRING, IDM_ADJ_BC, L"밝기 / 대비...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_SAT, L"채도...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_HUE, L"색조...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_INVERT, L"반전");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_GRAY, L"흑백");
    AppendMenuW(adj, MF_SEPARATOR, 0, NULL);
    AppendMenuW(adj, MF_STRING, IDM_ADJ_ROT90, L"90도 회전");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_FLIP_H, L"좌우 반전");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_FLIP_V, L"상하 반전");
    AppendMenuW(adj, MF_SEPARATOR, 0, NULL);
    AppendMenuW(adj, MF_STRING, IDM_ADJ_CANVAS, L"캔버스 크기...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_IMGSIZE, L"이미지 크기...");

    AppendMenuW(adj, MF_STRING, IDM_ADJ_AUTO, L"자동 대비");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_THRESHOLD, L"한계값...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_POSTERIZE, L"포스터화...");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_ROTCCW, L"레이어 반시계 90도 회전");
    AppendMenuW(adj, MF_STRING, IDM_ADJ_ROT180, L"레이어 180도 회전");
    AppendMenuW(filt, MF_STRING, IDM_FILT_BLUR, L"흐리게...");
    AppendMenuW(filt, MF_STRING, IDM_FILT_SHARP, L"선명하게");

    AppendMenuW(tool, MF_STRING, IDM_TOOL_MOVE, L"이동\tV");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SELRECT, L"사각 선택\tM");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SELELLIPSE, L"타원 선택");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_LASSO, L"올가미\tL");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_WAND, L"마법봉\tW");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_CROP, L"자르기\tC");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_EYE, L"스포이드\tI");
    AppendMenuW(tool, MF_SEPARATOR, 0, NULL);
    {
        HMENU kinds = CreatePopupMenu();
        int i;
        AppendMenuW(tool, MF_STRING, IDM_TOOL_BRUSH, L"브러시\tB");
        AppendMenuW(tool, MF_STRING, IDM_BRUSH_PANEL, L"브러시 패널...");
        for (i = 0; i < BRUSH_PRESET_COUNT; i++)
            AppendMenuW(kinds, MF_STRING, IDM_BRUSH_KIND + i, brush_preset_name(i));
        AppendMenuW(tool, MF_POPUP, (UINT_PTR)kinds, L"브러시 종류");
    }
    AppendMenuW(tool, MF_STRING, IDM_TOOL_PENCIL, L"연필\tN");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_ERASER, L"지우개\tE");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_CLONE, L"복제 도장\tS");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_BUCKET, L"페인트통\tK");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_GRADIENT, L"그레이디언트\tG");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_DODGE, L"닷지\tO");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_BURN, L"번");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SPONGE, L"스폰지");
    AppendMenuW(tool, MF_SEPARATOR, 0, NULL);
    AppendMenuW(tool, MF_STRING, IDM_TOOL_TEXT, L"문자\tT");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SHAPERECT, L"사각형\tU");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SHAPEELLIPSE, L"원");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_SHAPELINE, L"선");
    AppendMenuW(tool, MF_STRING, IDM_TOOL_HAND, L"손\tH");
    AppendMenuW(tool, MF_SEPARATOR, 0, NULL);
    AppendMenuW(tool, MF_STRING, IDM_SWAP_COLORS, L"전경/배경 바꾸기\tX");
    AppendMenuW(tool, MF_STRING, IDM_DEFAULT_COLORS, L"기본 색\tD");
    AppendMenuW(tool, MF_STRING, IDM_COLOR, L"색 선택...");

    AppendMenuW(view, MF_STRING, IDM_VIEW_ZOOMIN, L"확대\tCtrl++");
    AppendMenuW(view, MF_STRING, IDM_VIEW_ZOOMOUT, L"축소\tCtrl+-");
    AppendMenuW(view, MF_STRING, IDM_VIEW_FIT, L"화면에 맞춤\tCtrl+0");
    AppendMenuW(view, MF_STRING, IDM_VIEW_100, L"실제 크기\tCtrl+1");

    AppendMenuW(help, MF_STRING, IDM_HELP_ABOUT, L"정보");

    AppendMenuW(bar, MF_POPUP, (UINT_PTR)file, L"파일");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)edit, L"편집");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)layer, L"레이어");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)adj, L"조정");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)filt, L"필터");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)tool, L"도구");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)view, L"보기");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)help, L"도움말");
    return bar;
}

static HICON make_icon(void)
{
    const int S = 32;
    BITMAPINFO bmi;
    HDC hdc, mem;
    HBITMAP color, mask;
    void *bits = NULL;
    uint8_t *px;
    int x, y;
    ICONINFO ii;
    HICON icon;
    HGDIOBJ old;
    RECT r;

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = S;
    bmi.bmiHeader.biHeight = -S;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdc = GetDC(NULL);
    color = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    px = (uint8_t *)bits;
    for (y = 0; y < S; y++) {
        for (x = 0; x < S; x++) {
            int dx = x - 16, dy = y - 16;
            int r2 = dx * dx + dy * dy;
            uint8_t a = 0, rch = 56, gch = 140, bch = 220;
            if (r2 <= 14 * 14) a = 255;
            else if (r2 <= 16 * 16) a = (uint8_t)clampi(255 - (r2 - 14 * 14) * 20, 0, 255);
            px[(y * S + x) * 4 + 0] = bch;
            px[(y * S + x) * 4 + 1] = gch;
            px[(y * S + x) * 4 + 2] = rch;
            px[(y * S + x) * 4 + 3] = a;
        }
    }
    mem = CreateCompatibleDC(hdc);
    old = SelectObject(mem, color);
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(255, 255, 255));
    SelectObject(mem, g.font);
    SetRect(&r, 0, 0, S, S);
    DrawTextW(mem, L"D", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, old);
    DeleteDC(mem);

    mask = CreateBitmap(S, S, 1, 1, NULL);
    memset(&ii, 0, sizeof(ii));
    ii.fIcon = TRUE;
    ii.hbmMask = mask;
    ii.hbmColor = color;
    icon = CreateIconIndirect(&ii);
    DeleteObject(color);
    DeleteObject(mask);
    ReleaseDC(NULL, hdc);
    return icon;
}

void update_title(void)
{
    wchar_t buf[MAX_PATH + 64];
    swprintf(buf, MAX_PATH + 64, L"DaGrim v" DAGRIM_VERSION_WSTRING L" - %s%s",
             path_filename(g.doc.path), g.doc.dirty ? L" *" : L"");
    SetWindowTextW(g.hwnd, buf);
}

void app_set_dirty(int dirty)
{
    g.doc.dirty = dirty ? 1 : 0;
    update_title();
}

void app_refresh(void)
{
    comp_ensure();
    update_title();
    ui_sync_layer_controls();
    ui_update_status();
    if (g.canvas) InvalidateRect(g.canvas, NULL, FALSE);
    if (g.layers) InvalidateRect(g.layers, NULL, FALSE);
    if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
    if (g.status) InvalidateRect(g.status, NULL, FALSE);
    if (brush_popup_visible()) {
        SetWindowPos(g.brush_popup, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        InvalidateRect(g.brush_popup, NULL, FALSE);
    }
}

static void app_layout(HWND hwnd)
{
    RECT r;
    int tb, lp, st, y0, y1, x1, x2;
    GetClientRect(hwnd, &r);
    tb = ui_scale(52);

    lp = ui_scale(240);
    st = ui_scale(26);
    y1 = r.bottom - st;
    if (y1 < 40) y1 = 40;
    y0 = 0;
    x1 = tb;
    x2 = r.right - lp;
    if (x2 < x1 + 80) x2 = x1 + 80;

    MoveWindow(g.toolbar, 0, y0, tb, y1 - y0, TRUE);
    MoveWindow(g.canvas, tb, y0, x2 - tb, y1 - y0, TRUE);
    MoveWindow(g.layers, x2, y0, r.right - x2, y1 - y0, TRUE);
    MoveWindow(g.status, 0, y1, r.right, st, TRUE);
    brush_popup_layout();
}

int confirm_discard(void)
{
    int r;
    if (!g.doc.dirty) return 1;
    r = MessageBoxW(g.hwnd,
                    L"저장하지 않은 변경이 있습니다.\n저장하시겠습니까?",
                    L"DaGrim", MB_YESNOCANCEL | MB_ICONWARNING);
    if (r == IDCANCEL) return 0;
    if (r == IDYES) {
        app_command(IDM_FILE_SAVE);
        return !g.doc.dirty;
    }
    return 1;
}

static int pick_open(wchar_t *path, int layered)
{
    OPENFILENAMEW ofn;
    memset(&ofn, 0, sizeof(ofn));
    path[0] = 0;
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g.hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    if (layered) {
        ofn.lpstrFilter = L"이미지 / DaGrim\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif;*.dagrim\0"
                          L"이미지\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0"
                          L"DaGrim 문서\0*.dagrim\0모든 파일\0*.*\0";
    } else {
        ofn.lpstrFilter = L"이미지\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0모든 파일\0*.*\0";
    }
    return GetOpenFileNameW(&ofn);
}

static int pick_save(wchar_t *path, int export_only)
{
    OPENFILENAMEW ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g.hwnd;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    if (export_only) {
        ofn.lpstrFilter = L"PNG\0*.png\0JPEG\0*.jpg\0";
        ofn.lpstrDefExt = L"png";
    } else {
        ofn.lpstrFilter = L"DaGrim 문서\0*.dagrim\0PNG\0*.png\0JPEG\0*.jpg\0";
        ofn.lpstrDefExt = L"dagrim";
    }
    return GetSaveFileNameW(&ofn);
}

static int save_flat(const wchar_t *path)
{
    uint8_t *flat;
    int ok;
    flat = xmalloc((size_t)g.doc.w * (size_t)g.doc.h * 4);
    if (!flatten_document(&g.doc, flat)) {
        free(flat);
        return 0;
    }
    if (ext_eq(path, L".jpg") || ext_eq(path, L".jpeg"))
        ok = io_save_jpg(path, flat, g.doc.w, g.doc.h, 92);
    else
        ok = io_save_png(path, flat, g.doc.w, g.doc.h);
    free(flat);
    return ok;
}

static int app_save(int save_as)
{
    wchar_t path[MAX_PATH];
    int ok;
    wcsncpy(path, g.doc.path, MAX_PATH - 1);
    path[MAX_PATH - 1] = 0;
    if (save_as || !path[0]) {
        path[0] = 0;
        if (!pick_save(path, 0)) return 0;
    }
    if (ext_eq(path, L".dagrim")) ok = io_save_dagrim(path, &g.doc);
    else ok = save_flat(path);
    if (!ok) {
        MessageBoxW(g.hwnd, L"저장에 실패했습니다.", L"DaGrim", MB_ICONERROR);
        return 0;
    }
    wcsncpy(g.doc.path, path, MAX_PATH - 1);
    g.doc.path[MAX_PATH - 1] = 0;
    app_set_dirty(0);
    return 1;
}

int app_add_image_layer(uint8_t *bgra, int w, int h, const wchar_t *name, int push_undo)
{
    Layer L;
    if (!bgra || w < 1 || h < 1) {
        free(bgra);
        return 0;
    }
    memset(&L, 0, sizeof(L));
    L.w = w;
    L.h = h;
    L.bgra = bgra;
    L.opacity = 255;
    L.visible = 1;
    L.blend = BLEND_NORMAL;
    if (name && name[0]) wcsncpy(L.name, name, 63);
    else wcsncpy(L.name, L"사진", 63);
    L.name[63] = 0;

    if (g.doc.w < 1 || g.doc.h < 1) {
        g.doc.w = w;
        g.doc.h = h;
        L.x = 0;
        L.y = 0;
    } else {
        doc_ensure_canvas(&g.doc, w, h);
        L.x = (g.doc.w - w) / 2;
        L.y = (g.doc.h - h) / 2;
        if (L.x < 0) L.x = 0;
        if (L.y < 0) L.y = 0;
    }

    if (push_undo) undo_push();
    if (!doc_insert(&g.doc, g.doc.count, &L)) {
        layer_free(&L);
        return 0;
    }
    if (push_undo) {
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
    }
    return 1;
}

static int add_layer_from_file_core(const wchar_t *path)
{
    uint8_t *bgra = NULL;
    int w = 0, h = 0;
    if (!io_load_image(path, &bgra, &w, &h)) return 0;
    return app_add_image_layer(bgra, w, h, path_filename(path), 0);
}

static UINT clipboard_png_format(void)
{
    static UINT fmt;
    if (!fmt) fmt = RegisterClipboardFormatW(L"PNG");
    return fmt;
}

int clipboard_has_image(void)
{
    UINT png = clipboard_png_format();
    return IsClipboardFormatAvailable(CF_DIB) ||
           IsClipboardFormatAvailable(CF_DIBV5) ||
           IsClipboardFormatAvailable(CF_BITMAP) ||
           IsClipboardFormatAvailable(CF_HDROP) ||
           (png && IsClipboardFormatAvailable(png));
}

static int clipboard_lock_bytes(UINT fmt, const uint8_t **ptr, size_t *len, HANDLE *hold)
{
    HANDLE h = GetClipboardData(fmt);
    void *p;
    if (!h) return 0;
    p = GlobalLock(h);
    if (!p) return 0;
    *hold = h;
    *ptr = (const uint8_t *)p;
    *len = GlobalSize(h);
    return 1;
}

static int clipboard_paste(void)
{
    uint8_t *bgra = NULL;
    int w = 0, h = 0;
    int ok = 0;
    UINT png = clipboard_png_format();
    wchar_t name[64];
    static int paste_n;

    if (!OpenClipboard(g.hwnd)) {
        MessageBoxW(g.hwnd, L"클립보드를 열 수 없습니다.", L"DaGrim", MB_ICONERROR);
        return 0;
    }

    if (png && IsClipboardFormatAvailable(png)) {
        const uint8_t *p = NULL;
        size_t len = 0;
        HANDLE clip = NULL;
        if (clipboard_lock_bytes(png, &p, &len, &clip)) {
            ok = io_load_image_mem(p, len, &bgra, &w, &h);
            GlobalUnlock(clip);
        }
    }
    if (!ok && IsClipboardFormatAvailable(CF_DIBV5)) {
        const uint8_t *p = NULL;
        size_t len = 0;
        HANDLE clip = NULL;
        if (clipboard_lock_bytes(CF_DIBV5, &p, &len, &clip)) {
            ok = io_load_dib(p, len, &bgra, &w, &h);
            GlobalUnlock(clip);
        }
    }
    if (!ok && IsClipboardFormatAvailable(CF_DIB)) {
        const uint8_t *p = NULL;
        size_t len = 0;
        HANDLE clip = NULL;
        if (clipboard_lock_bytes(CF_DIB, &p, &len, &clip)) {
            ok = io_load_dib(p, len, &bgra, &w, &h);
            GlobalUnlock(clip);
        }
    }
    if (!ok && IsClipboardFormatAvailable(CF_BITMAP)) {
        HBITMAP hbmp = (HBITMAP)GetClipboardData(CF_BITMAP);
        if (hbmp) ok = io_load_hbitmap(hbmp, &bgra, &w, &h);
    }
    if (!ok && IsClipboardFormatAvailable(CF_HDROP)) {
        HDROP drop = (HDROP)GetClipboardData(CF_HDROP);
        UINT n, i;
        int added = 0;
        if (drop) {
            n = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
            undo_push();
            for (i = 0; i < n; i++) {
                wchar_t path[MAX_PATH];
                if (DragQueryFileW(drop, i, path, MAX_PATH) &&
                    !ext_eq(path, L".dagrim") &&
                    add_layer_from_file_core(path))
                    added++;
            }
            CloseClipboard();
            if (added) {
                app_set_dirty(1);
                comp_invalidate_all();
                app_refresh();
                return 1;
            }
            MessageBoxW(g.hwnd, L"클립보드의 파일을 이미지로 가져올 수 없습니다.", L"DaGrim", MB_ICONWARNING);
            return 0;
        }
    }
    CloseClipboard();

    if (!ok || !bgra) {
        MessageBoxW(g.hwnd, L"클립보드에 붙여넣을 사진이 없습니다.\n다른 앱에서 사진을 복사한 뒤 Ctrl+V를 누르세요.",
                    L"DaGrim", MB_ICONINFORMATION);
        return 0;
    }

    paste_n++;
    swprintf(name, 64, L"붙여넣기 %d", paste_n);
    return app_add_image_layer(bgra, w, h, name, 1);
}

static int clipboard_copy(void)
{
    Layer *L = doc_active(&g.doc);
    const uint8_t *src = NULL;
    uint8_t *owned = NULL;
    int w, h, y, x;
    int bx0 = 0, by0 = 0, bx1 = 0, by1 = 0;
    size_t header, pix, stride;
    HGLOBAL mem;
    BITMAPINFOHEADER *bih;
    uint8_t *dst;

    if (g.have_sel && sel_bounds(&bx0, &by0, &bx1, &by1) && bx1 > bx0 && by1 > by0) {
        w = bx1 - bx0;
        h = by1 - by0;
        owned = xcalloc((size_t)w * (size_t)h, 4);
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                int dx = bx0 + x, dy = by0 + y, m = sel_mask(dx, dy);
                uint8_t *d = owned + ((size_t)y * (size_t)w + (size_t)x) * 4;
                if (m <= 0 || !L || !L->bgra) continue;
                {
                    int lx = dx - L->x, ly = dy - L->y;
                    if (lx >= 0 && ly >= 0 && lx < L->w && ly < L->h) {
                        uint8_t *s = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
                        d[0] = s[0];
                        d[1] = s[1];
                        d[2] = s[2];
                        d[3] = (uint8_t)(s[3] * m / 255);
                    }
                }
            }
        }
        src = owned;
    } else if (L && L->bgra) {
        src = L->bgra;
        w = L->w;
        h = L->h;
    } else {
        if (g.doc.w < 1 || g.doc.h < 1) return 0;
        w = g.doc.w;
        h = g.doc.h;
    }

    stride = (size_t)w * 4;
    pix = stride * (size_t)h;
    header = sizeof(BITMAPINFOHEADER);
    mem = GlobalAlloc(GMEM_MOVEABLE, header + pix);
    if (!mem) {
        free(owned);
        return 0;
    }
    bih = (BITMAPINFOHEADER *)GlobalLock(mem);
    if (!bih) {
        GlobalFree(mem);
        free(owned);
        return 0;
    }
    memset(bih, 0, header);
    bih->biSize = (DWORD)header;
    bih->biWidth = w;
    bih->biHeight = h;
    bih->biPlanes = 1;
    bih->biBitCount = 32;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = (DWORD)pix;
    dst = (uint8_t *)bih + header;

    if (src) {
        for (y = 0; y < h; y++)
            memcpy(dst + (size_t)(h - 1 - y) * stride, src + (size_t)y * stride, stride);
    } else {
        uint8_t *flat = xmalloc(pix);
        flatten_document(&g.doc, flat);
        for (y = 0; y < h; y++)
            memcpy(dst + (size_t)(h - 1 - y) * stride, flat + (size_t)y * stride, stride);
        free(flat);
    }
    free(owned);
    GlobalUnlock(mem);

    if (!OpenClipboard(g.hwnd)) {
        GlobalFree(mem);
        return 0;
    }
    EmptyClipboard();
    if (!SetClipboardData(CF_DIB, mem)) {
        GlobalFree(mem);
        CloseClipboard();
        return 0;
    }
    CloseClipboard();
    return 1;
}

static int import_photos(void)
{
    OPENFILENAMEW ofn;
    wchar_t *buf;
    int added = 0;

    buf = (wchar_t *)xcalloc(32768, sizeof(wchar_t));
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g.hwnd;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = 32768;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    ofn.lpstrFilter = L"이미지\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0모든 파일\0*.*\0";
    ofn.lpstrTitle = L"사진 가져오기";
    if (!GetOpenFileNameW(&ofn)) {
        free(buf);
        return 0;
    }

    undo_push();
    if (ofn.nFileOffset > 0 && buf[ofn.nFileOffset - 1] == 0) {
        wchar_t *p = buf + ofn.nFileOffset;
        while (*p) {
            wchar_t full[MAX_PATH];
            if (swprintf(full, MAX_PATH, L"%s\\%s", buf, p) > 0) {
                if (add_layer_from_file_core(full)) added++;
            }
            p += wcslen(p) + 1;
        }
    } else if (add_layer_from_file_core(buf)) {
        added++;
    }
    free(buf);

    if (!added) {
        MessageBoxW(g.hwnd, L"선택한 사진을 가져올 수 없습니다.", L"DaGrim", MB_ICONERROR);
        return 0;
    }
    app_set_dirty(1);
    comp_invalidate_all();
    app_refresh();
    return 1;
}

int app_open_path(const wchar_t *path)
{
    Document nd;
    doc_init(&nd);
    if (ext_eq(path, L".dagrim")) {
        if (!io_load_dagrim(path, &nd)) {
            MessageBoxW(g.hwnd, L"문서를 열 수 없습니다.", L"DaGrim", MB_ICONERROR);
            return 0;
        }
    } else {
        uint8_t *bgra = NULL;
        int w = 0, h = 0;
        Layer L;
        if (!io_load_image(path, &bgra, &w, &h)) {
            MessageBoxW(g.hwnd, L"이미지를 열 수 없습니다.", L"DaGrim", MB_ICONERROR);
            return 0;
        }
        nd.w = w;
        nd.h = h;
        memset(&L, 0, sizeof(L));
        L.w = w;
        L.h = h;
        L.bgra = bgra;
        L.opacity = 255;
        L.visible = 1;
        L.blend = BLEND_NORMAL;
        wcsncpy(L.name, path_filename(path), 63);
        L.name[63] = 0;
        if (!doc_insert(&nd, 0, &L)) {
            layer_free(&L);
            return 0;
        }
        wcsncpy(nd.path, path, MAX_PATH - 1);
        nd.path[MAX_PATH - 1] = 0;
        nd.dirty = 0;
    }
    tools_end_resize();
    undo_clear();
    doc_free(&g.doc);
    g.doc = nd;
    g.crop_on = 0;
    sel_reset();
    comp_invalidate_all();
    view_fit();
    app_refresh();
    return 1;
}

void app_command(int id)
{
    Layer *L;
    wchar_t path[MAX_PATH];

    switch (id) {
    case IDM_FILE_NEW:
        if (!confirm_discard()) return;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_NEW), g.hwnd, NewDlgProc) != IDOK) return;
        tools_end_resize();
        undo_clear();
        if (!doc_new(&g.doc, g.new_w, g.new_h, g.new_white)) return;
        g.crop_on = 0;
        sel_reset();
        comp_invalidate_all();
        view_fit();
        app_refresh();
        break;

    case IDM_FILE_OPEN:
        if (!confirm_discard()) return;
        if (!pick_open(path, 1)) return;
        app_open_path(path);
        break;

    case IDM_FILE_SAVE:
        app_save(0);
        break;

    case IDM_FILE_SAVEAS:
        app_save(1);
        break;

    case IDM_FILE_EXPORT:
        path[0] = 0;
        if (!pick_save(path, 1)) return;
        if (!save_flat(path))
            MessageBoxW(g.hwnd, L"내보내기에 실패했습니다.", L"DaGrim", MB_ICONERROR);
        break;

    case IDM_FILE_EXIT:
        PostMessageW(g.hwnd, WM_CLOSE, 0, 0);
        break;

    case IDM_EDIT_UNDO:
        if (undo_can()) {
            undo_perform();
            app_refresh();
        }
        break;

    case IDM_EDIT_REDO:
        if (redo_can()) {
            redo_perform();
            app_refresh();
        }
        break;

    case IDM_EDIT_COPY:
        if (!clipboard_copy())
            MessageBoxW(g.hwnd, L"복사에 실패했습니다.", L"DaGrim", MB_ICONERROR);
        break;

    case IDM_EDIT_PASTE:
        clipboard_paste();
        break;

    case IDM_EDIT_CUT:
        if (!clipboard_copy()) {
            MessageBoxW(g.hwnd, L"잘라내기에 실패했습니다.", L"DaGrim", MB_ICONERROR);
            break;
        }
        if (!doc_active(&g.doc)) break;
        undo_push();
        sel_clear_pixels();
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_EDIT_FILL:
        if (!doc_active(&g.doc)) return;
        undo_push();
        sel_fill_fg();
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_EDIT_SELALL:
        sel_all();
        app_refresh();
        break;

    case IDM_EDIT_DESEL:
        sel_deselect();
        app_refresh();
        break;

    case IDM_EDIT_INVSEL:
        sel_invert();
        app_refresh();
        break;

    case IDM_EDIT_CLEAR:
        if (g.have_sel) {
            if (!doc_active(&g.doc)) return;
            undo_push();
            sel_clear_pixels();
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        } else {
            app_command(IDM_LAYER_DEL);
        }
        break;

    case IDM_LAYER_NEW:
        undo_push();
        if (doc_add_empty(&g.doc)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_FROM_FILE:
        import_photos();
        break;

    case IDM_LAYER_DUP:
        undo_push();
        if (doc_duplicate_active(&g.doc)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_DEL:
        if (!doc_active(&g.doc)) return;
        undo_push();
        if (doc_delete_active(&g.doc)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_UP:
        undo_push();
        if (doc_move_active(&g.doc, 1)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_DOWN:
        undo_push();
        if (doc_move_active(&g.doc, -1)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_RENAME:
        L = doc_active(&g.doc);
        if (!L) return;
        wcsncpy(g.rename_buf, L->name, 63);
        g.rename_buf[63] = 0;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_RENAME), g.hwnd, RenameDlgProc) == IDOK &&
            g.rename_buf[0]) {
            undo_push();
            wcsncpy(L->name, g.rename_buf, 63);
            L->name[63] = 0;
            app_set_dirty(1);
            InvalidateRect(g.layers, NULL, FALSE);
            update_title();
        }
        break;

    case IDM_LAYER_SCALE:
        L = doc_active(&g.doc);
        if (!L || !L->bgra) return;
        g.new_w = L->w;
        g.new_h = L->h;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SCALE), g.hwnd, ScaleDlgProc) != IDOK) return;
        if (g.new_w == L->w && g.new_h == L->h) return;
        undo_push();
        if (layer_scale_from(L, L->bgra, L->w, L->h, g.new_w, g.new_h)) {
            doc_ensure_canvas(&g.doc, L->x + L->w, L->y + L->h);
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_MERGE:
        if (g.doc.active < 1) return;
        undo_push();
        if (doc_merge_down(&g.doc)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_LAYER_FLAT:
        if (g.doc.count < 1) return;
        undo_push();
        if (doc_flatten(&g.doc)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_SEL_EXPAND:
    case IDM_SEL_CONTRACT:
    case IDM_SEL_FEATHER: {
        int radius=3;
        const wchar_t *title=id==IDM_SEL_EXPAND?L"선택 영역 확대":id==IDM_SEL_CONTRACT?L"선택 영역 축소":L"선택 영역 페더";
        if(!g.have_sel)return;
        if(!ui_value_dialog(title,L"반지름 (픽셀)",1,100,&radius))return;
        sel_modify(id==IDM_SEL_EXPAND?0:id==IDM_SEL_CONTRACT?1:2,radius);
        app_refresh(); break;
    }
    case IDM_ADJ_AUTO:
    case IDM_ADJ_THRESHOLD:
    case IDM_ADJ_POSTERIZE:
    case IDM_ADJ_ROTCCW:
    case IDM_ADJ_ROT180: {
        int value=id==IDM_ADJ_THRESHOLD?128:4;
        L=doc_active(&g.doc); if(!L)return;
        if(id==IDM_ADJ_THRESHOLD && !ui_value_dialog(L"한계값",L"밝기 기준",0,255,&value))return;
        if(id==IDM_ADJ_POSTERIZE && !ui_value_dialog(L"포스터화",L"채널별 단계",2,256,&value))return;
        undo_push();
        if(id==IDM_ADJ_AUTO)adj_auto_contrast(L);
        else if(id==IDM_ADJ_THRESHOLD)adj_threshold(L,value);
        else if(id==IDM_ADJ_POSTERIZE)adj_posterize(L,value);
        else { adj_rotate_quarters(L,id==IDM_ADJ_ROTCCW?3:2); doc_ensure_canvas(&g.doc,L->x+L->w,L->y+L->h); }
        app_set_dirty(1); comp_invalidate_all(); app_refresh(); break;
    }
    case IDM_ADJ_BC:
        L = doc_active(&g.doc);
        if (!L) return;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_BC), g.hwnd, BcDlgProc) != IDOK) return;
        if (g.adj_b == 0 && g.adj_c == 0) return;
        undo_push();
        adj_brightness_contrast(L, g.adj_b, g.adj_c);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_SAT:
        L = doc_active(&g.doc);
        if (!L) return;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SAT), g.hwnd, SatDlgProc) != IDOK) return;
        if (g.adj_s == 0) return;
        undo_push();
        adj_saturation(L, g.adj_s);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_ROT90:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        adj_rotate90(L);
        doc_ensure_canvas(&g.doc, L->x + L->w, L->y + L->h);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_FLIP_H:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        adj_flip_h(L);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_FLIP_V:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        adj_flip_v(L);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_HUE:
        L = doc_active(&g.doc);
        if (!L) return;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SAT), g.hwnd, HueDlgProc) != IDOK) return;
        if (g.adj_s == 0) return;
        undo_push();
        adj_hue(L, g.adj_s);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_INVERT:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        adj_invert(L);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_GRAY:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        adj_grayscale(L);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_ADJ_CANVAS:
        if (g.doc.w < 1 || g.doc.h < 1) return;
        g.new_w = g.doc.w;
        g.new_h = g.doc.h;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SCALE), g.hwnd, ScaleDlgProc) != IDOK) return;
        if (g.new_w == g.doc.w && g.new_h == g.doc.h) return;
        undo_push();
        if (doc_canvas_size(&g.doc, g.new_w, g.new_h)) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        }
        break;

    case IDM_ADJ_IMGSIZE:
        if (g.doc.w < 1 || g.doc.h < 1) return;
        g.new_w = g.doc.w;
        g.new_h = g.doc.h;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SCALE), g.hwnd, ScaleDlgProc) != IDOK) return;
        if (g.new_w == g.doc.w && g.new_h == g.doc.h) return;
        undo_push();
        if (doc_image_size(&g.doc, g.new_w, g.new_h)) {
            app_set_dirty(1);
            comp_invalidate_all();
            view_fit();
            app_refresh();
        }
        break;

    case IDM_FILT_BLUR:
        L = doc_active(&g.doc);
        if (!L) return;
        if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_SAT), g.hwnd, BlurDlgProc) != IDOK) return;
        undo_push();
        filt_blur(L, g.filt_r);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_FILT_SHARP:
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        filt_sharpen(L);
        app_set_dirty(1);
        comp_invalidate_all();
        app_refresh();
        break;

    case IDM_VIEW_ZOOMIN:
        view_zoom_at(g.last_sx, g.last_sy, 1);
        app_refresh();
        break;
    case IDM_VIEW_ZOOMOUT:
        view_zoom_at(g.last_sx, g.last_sy, 0);
        app_refresh();
        break;
    case IDM_VIEW_FIT:
        view_fit();
        app_refresh();
        break;
    case IDM_VIEW_100:
        view_actual();
        app_refresh();
        break;

    case IDM_TOOL_MOVE: tools_set_tool(TOOL_MOVE); break;
    case IDM_TOOL_SELRECT: tools_set_tool(TOOL_SEL_RECT); break;
    case IDM_TOOL_SELELLIPSE: tools_set_tool(TOOL_SEL_ELLIPSE); break;
    case IDM_TOOL_LASSO: tools_set_tool(TOOL_LASSO); break;
    case IDM_TOOL_WAND: tools_set_tool(TOOL_WAND); break;
    case IDM_TOOL_CROP: tools_set_tool(TOOL_CROP); break;
    case IDM_TOOL_EYE: tools_set_tool(TOOL_EYEDROPPER); break;
    case IDM_TOOL_BRUSH: tools_set_tool(TOOL_BRUSH); break;
    case IDM_BRUSH_PANEL:
        tools_set_tool(TOOL_BRUSH);
        break;
    case IDM_TOOL_PENCIL: tools_set_tool(TOOL_PENCIL); break;
    case IDM_TOOL_ERASER: tools_set_tool(TOOL_ERASER); break;
    case IDM_TOOL_CLONE: tools_set_tool(TOOL_CLONE); break;
    case IDM_TOOL_BUCKET: tools_set_tool(TOOL_BUCKET); break;
    case IDM_TOOL_GRADIENT: tools_set_tool(TOOL_GRADIENT); break;
    case IDM_TOOL_DODGE: tools_set_tool(TOOL_DODGE); break;
    case IDM_TOOL_BURN: tools_set_tool(TOOL_BURN); break;
    case IDM_TOOL_SPONGE: tools_set_tool(TOOL_SPONGE); break;
    case IDM_TOOL_TEXT: tools_set_tool(TOOL_TEXT); break;
    case IDM_TOOL_SHAPERECT:
        g.shape_kind = SHAPE_RECT;
        tools_set_tool(TOOL_SHAPE_RECT);
        break;
    case IDM_TOOL_SHAPEELLIPSE:
        g.shape_kind = SHAPE_ELLIPSE;
        tools_set_tool(TOOL_SHAPE_ELLIPSE);
        break;
    case IDM_TOOL_SHAPELINE:
        g.shape_kind = SHAPE_LINE;
        tools_set_tool(TOOL_SHAPE_LINE);
        break;
    case IDM_TOOL_HAND: tools_set_tool(TOOL_HAND); break;
    case IDM_SWAP_COLORS: {
        COLORREF t = g.color;
        g.color = g.color_bg;
        g.color_bg = t;
        if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
        break;
    }
    case IDM_DEFAULT_COLORS:
        g.color = RGB(0, 0, 0);
        g.color_bg = RGB(255, 255, 255);
        if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
        break;

    case IDM_BRUSH_DEC:
        g.brush_size = clampi(g.brush_size - 1, 1, 200);
        ui_update_status();
        InvalidateRect(g.status, NULL, FALSE);
        break;
    case IDM_BRUSH_INC:
        g.brush_size = clampi(g.brush_size + 1, 1, 200);
        ui_update_status();
        InvalidateRect(g.status, NULL, FALSE);
        break;
    case IDM_APPLY_CROP:
        if (GetFocus() == g.blend_combo) break;
        tools_apply_crop();
        break;
    case IDM_CANCEL_CROP:
        if (brush_popup_visible()) {
            brush_popup_hide();
            break;
        }
        if (g.crop_on) tools_cancel_crop();
        else {
            sel_deselect();
            app_refresh();
        }
        break;
    case IDM_COLOR:
        ui_choose_color();
        break;
    case IDM_HELP_ABOUT:
        DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_ABOUT), g.hwnd, AboutDlgProc);
        break;
    default:
        if (id >= IDM_BRUSH_KIND && id < IDM_BRUSH_KIND + BRUSH_PRESET_COUNT) {
            g.brush_preset = id - IDM_BRUSH_KIND;
            tools_set_tool(TOOL_BRUSH);
            if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
            ui_update_status();
            if (g.status) InvalidateRect(g.status, NULL, FALSE);
        }
        break;
    }
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        g.hwnd = hwnd;
        g.dpi = (int)dpi_for(hwnd);
        ui_init_visuals();
        g.icon = make_icon();
        if (g.icon) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g.icon);
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g.icon);
        }
        g.toolbar = CreateWindowExW(0, L"DaGrimToolbar", NULL,
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, g.inst, NULL);
        g.canvas = CreateWindowExW(0, L"DaGrimCanvas", NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0, hwnd, NULL, g.inst, NULL);
        g.layers = CreateWindowExW(0, L"DaGrimLayers", NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, 0, hwnd, NULL, g.inst, NULL);
        g.status = CreateWindowExW(0, L"DaGrimStatus", NULL,
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, g.inst, NULL);
        DragAcceptFiles(hwnd, TRUE);
        doc_new(&g.doc, 1280, 720, 1);
        update_title();
        ui_update_status();
        return 0;

    case WM_SIZE:
        app_layout(hwnd);
        if (!g.fitted) view_fit();
        return 0;

    case WM_COMMAND:
        app_command(LOWORD(wParam));
        return 0;

    case WM_DROPFILES: {
        HDROP drop = (HDROP)wParam;
        UINT n = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
        UINT i;
        int added = 0;
        int opened = 0;
        for (i = 0; i < n; i++) {
            wchar_t path[MAX_PATH];
            if (!DragQueryFileW(drop, i, path, MAX_PATH)) continue;
            if (ext_eq(path, L".dagrim")) {
                if (!opened && confirm_discard()) {
                    if (app_open_path(path)) opened = 1;
                }
            } else {
                if (!added) undo_push();
                if (add_layer_from_file_core(path)) added++;
            }
        }
        DragFinish(drop);
        if (added) {
            app_set_dirty(1);
            comp_invalidate_all();
            app_refresh();
        } else if (n > 0 && !opened) {
            MessageBoxW(g.hwnd, L"가져온 사진이 없습니다.", L"DaGrim", MB_ICONWARNING);
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            g.space_down = 1;
            return 0;
        }
        break;

    case WM_KEYUP:
        if (wParam == VK_SPACE) {
            g.space_down = 0;
            return 0;
        }
        break;

    case WM_INITMENUPOPUP: {
        HMENU m = (HMENU)wParam;
        EnableMenuItem(m, IDM_EDIT_UNDO, MF_BYCOMMAND | (undo_can() ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_REDO, MF_BYCOMMAND | (redo_can() ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_COPY, MF_BYCOMMAND | (doc_active(&g.doc) || g.doc.w > 0 ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_CUT, MF_BYCOMMAND | (doc_active(&g.doc) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_PASTE, MF_BYCOMMAND | (clipboard_has_image() ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_FILL, MF_BYCOMMAND | (doc_active(&g.doc) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_EDIT_CLEAR, MF_BYCOMMAND | (doc_active(&g.doc) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_LAYER_SCALE, MF_BYCOMMAND | (doc_active(&g.doc) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_LAYER_MERGE, MF_BYCOMMAND | (g.doc.active >= 1 ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(m, IDM_LAYER_FLAT, MF_BYCOMMAND | (g.doc.count > 0 ? MF_ENABLED : MF_GRAYED));
        return 0;
    }

    case WM_CLOSE:
        if (!confirm_discard()) return 0;
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        brush_popup_hide();
        if (g.brush_popup) {
            DestroyWindow(g.brush_popup);
            g.brush_popup = NULL;
        }
        tools_end_resize();
        undo_clear();
        sel_reset();
        doc_free(&g.doc);
        free(g.disp);
        g.disp = NULL;
        ui_free_visuals();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    WNDCLASSW wc;
    HWND hwnd;
    MSG msg;
    HACCEL accel;
    INITCOMMONCONTROLSEX icc;
    ACCEL accels[] = {
        { FCONTROL | FVIRTKEY, 'N', IDM_FILE_NEW },
        { FCONTROL | FVIRTKEY, 'O', IDM_FILE_OPEN },
        { FCONTROL | FVIRTKEY, 'S', IDM_FILE_SAVE },
        { FCONTROL | FSHIFT | FVIRTKEY, 'S', IDM_FILE_SAVEAS },
        { FCONTROL | FVIRTKEY, 'E', IDM_FILE_EXPORT },
        { FCONTROL | FVIRTKEY, 'Z', IDM_EDIT_UNDO },
        { FCONTROL | FVIRTKEY, 'Y', IDM_EDIT_REDO },
        { FCONTROL | FVIRTKEY, 'C', IDM_EDIT_COPY },
        { FCONTROL | FVIRTKEY, 'V', IDM_EDIT_PASTE },
        { FCONTROL | FVIRTKEY, 'X', IDM_EDIT_CUT },
        { FCONTROL | FVIRTKEY, 'A', IDM_EDIT_SELALL },
        { FCONTROL | FVIRTKEY, 'D', IDM_EDIT_DESEL },
        { FCONTROL | FSHIFT | FVIRTKEY, 'I', IDM_EDIT_INVSEL },
        { FCONTROL | FVIRTKEY, 'I', IDM_LAYER_FROM_FILE },
        { FCONTROL | FVIRTKEY, 'T', IDM_LAYER_SCALE },
        { FCONTROL | FVIRTKEY, VK_OEM_PLUS, IDM_VIEW_ZOOMIN },
        { FCONTROL | FVIRTKEY, VK_ADD, IDM_VIEW_ZOOMIN },
        { FCONTROL | FVIRTKEY, VK_OEM_MINUS, IDM_VIEW_ZOOMOUT },
        { FCONTROL | FVIRTKEY, VK_SUBTRACT, IDM_VIEW_ZOOMOUT },
        { FCONTROL | FVIRTKEY, '0', IDM_VIEW_FIT },
        { FCONTROL | FVIRTKEY, '1', IDM_VIEW_100 },
        { FVIRTKEY, 'V', IDM_TOOL_MOVE },
        { FVIRTKEY, 'M', IDM_TOOL_SELRECT },
        { FSHIFT | FVIRTKEY, 'M', IDM_TOOL_SELELLIPSE },
        { FVIRTKEY, 'L', IDM_TOOL_LASSO },
        { FVIRTKEY, 'W', IDM_TOOL_WAND },
        { FVIRTKEY, 'C', IDM_TOOL_CROP },
        { FVIRTKEY, 'I', IDM_TOOL_EYE },
        { FVIRTKEY, 'B', IDM_TOOL_BRUSH },
        { FVIRTKEY, 'N', IDM_TOOL_PENCIL },
        { FVIRTKEY, 'E', IDM_TOOL_ERASER },
        { FVIRTKEY, 'S', IDM_TOOL_CLONE },
        { FVIRTKEY, 'K', IDM_TOOL_BUCKET },
        { FVIRTKEY, 'G', IDM_TOOL_GRADIENT },
        { FVIRTKEY, 'O', IDM_TOOL_DODGE },
        { FSHIFT | FVIRTKEY, 'O', IDM_TOOL_BURN },
        { FVIRTKEY, 'T', IDM_TOOL_TEXT },
        { FVIRTKEY, 'U', IDM_TOOL_SHAPERECT },
        { FVIRTKEY, 'H', IDM_TOOL_HAND },
        { FVIRTKEY, 'X', IDM_SWAP_COLORS },
        { FVIRTKEY, 'D', IDM_DEFAULT_COLORS },
        { FVIRTKEY, VK_OEM_4, IDM_BRUSH_DEC },
        { FVIRTKEY, VK_OEM_6, IDM_BRUSH_INC },
        { FVIRTKEY, VK_DELETE, IDM_EDIT_CLEAR },
        { FVIRTKEY, VK_BACK, IDM_EDIT_CLEAR },
        { FVIRTKEY, VK_RETURN, IDM_APPLY_CROP },
        { FVIRTKEY, VK_ESCAPE, IDM_CANCEL_CROP },
    };
    int argc = 0;
    wchar_t **argv;

    (void)prev;
    memset(&g, 0, sizeof(g));
    g.inst = inst;
    g.dpi = 96;
    g.tool = TOOL_BRUSH;
    g.color = RGB(20, 20, 20);
    g.color_bg = RGB(255, 255, 255);
    g.brush_size = 20;
    g.brush_opacity = 220;
    g.brush_preset = BRUSH_ROUND;
    g.shape_kind = SHAPE_RECT;
    g.shape_fill = 1;
    g.shape_outline = 1;
    g.shape_stroke_preset = BRUSH_ROUND;
    g.stroke_width = 3;
    g.stroke_dx = 1.0f;
    g.wand_tol = 32;
    g.text_size = 48;
    g.filt_r = 3;
    g.zoom = 1.0f;
    g.doc.active = -1;

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    ui_register_classes(inst);

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DaGrimMain";
    RegisterClassW(&wc);

    {
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        int ww = sw * 3 / 4;
        int wh = sh * 4 / 5;
        if (ww < 1100) ww = sw > 1100 ? 1100 : sw;
        if (wh < 750) wh = sh > 750 ? 750 : sh;
        hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, L"DaGrimMain", L"DaGrim",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               (sw - ww) / 2, (sh - wh) / 2, ww, wh,
                               NULL, make_menu(), inst, NULL);
    }
    if (!hwnd) return 1;

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    SetFocus(g.canvas);

    argv = CommandLineToArgvW(cmd, &argc);
    if (argv && argc >= 2) app_open_path(argv[1]);
    if (argv) LocalFree(argv);

    accel = CreateAcceleratorTableW(accels, (int)(sizeof(accels) / sizeof(accels[0])));
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        HWND focus = GetFocus();
        wchar_t cls[32] = {0};
        int block = edit_focused();
        if (focus) {
            GetClassNameW(focus, cls, 32);
            if (focus == g.blend_combo || _wcsicmp(cls, L"ComboBox") == 0 ||
                _wcsicmp(cls, L"ComboLBox") == 0)
                block = 1;
        }
        if (!block) {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_SPACE) g.space_down = 1;
            if (msg.message == WM_KEYUP && msg.wParam == VK_SPACE) g.space_down = 0;
            if (TranslateAcceleratorW(g.hwnd, accel, &msg)) continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (accel) DestroyAcceleratorTable(accel);
    return (int)msg.wParam;
}