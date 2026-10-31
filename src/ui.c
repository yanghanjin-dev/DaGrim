#include "app.h"

static int tool_btn_h(void)
{
    int h = ui_scale(24);
    return h < 22 ? 22 : h;
}

static int tool_pad(void)
{
    int p = ui_scale(3);
    return p < 3 ? 3 : p;
}

static int toolbar_press = -1;

void ui_init_visuals(void)
{
    g.cur_arrow = LoadCursor(NULL, IDC_ARROW);
    g.cur_cross = LoadCursor(NULL, IDC_CROSS);
    g.cur_move = LoadCursor(NULL, IDC_SIZEALL);
    g.cur_hand = LoadCursor(NULL, IDC_HAND);
    g.cur_nwse = LoadCursor(NULL, IDC_SIZENWSE);
    g.cur_nesw = LoadCursor(NULL, IDC_SIZENESW);
    g.cur_we = LoadCursor(NULL, IDC_SIZEWE);
    g.cur_ns = LoadCursor(NULL, IDC_SIZENS);
    g.br_bg = CreateSolidBrush(RGB(28, 28, 32));
    g.br_panel = CreateSolidBrush(RGB(40, 40, 46));
    g.br_canvas = CreateSolidBrush(RGB(24, 24, 28));
    g.br_sel = CreateSolidBrush(RGB(0, 137, 145));
    g.br_btn = CreateSolidBrush(RGB(54, 54, 62));
    g.br_btn_hot = CreateSolidBrush(RGB(70, 70, 80));
    g.font = CreateFontW(-ui_scale(13), 0, 0, 0, FW_NORMAL, 0, 0, 0,
                         DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g.font_small = CreateFontW(-ui_scale(11), 0, 0, 0, FW_NORMAL, 0, 0, 0,
                               DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
}

void ui_free_visuals(void)
{
    if (g.font) DeleteObject(g.font);
    if (g.font_small) DeleteObject(g.font_small);
    if (g.br_bg) DeleteObject(g.br_bg);
    if (g.br_panel) DeleteObject(g.br_panel);
    if (g.br_canvas) DeleteObject(g.br_canvas);
    if (g.br_sel) DeleteObject(g.br_sel);
    if (g.br_btn) DeleteObject(g.br_btn);
    if (g.br_btn_hot) DeleteObject(g.br_btn_hot);
    if (g.icon) DestroyIcon(g.icon);
}

static void paint_begin(HWND hwnd, PAINTSTRUCT *ps, HDC *mem, HBITMAP *bm, HGDIOBJ *old, RECT *rc)
{
    HDC hdc = BeginPaint(hwnd, ps);
    GetClientRect(hwnd, rc);
    *mem = CreateCompatibleDC(hdc);
    *bm = CreateCompatibleBitmap(hdc, rc->right, rc->bottom);
    *old = SelectObject(*mem, *bm);
}

static void paint_end(HWND hwnd, PAINTSTRUCT *ps, HDC mem, HBITMAP bm, HGDIOBJ old, const RECT *rc)
{
    BitBlt(ps->hdc, 0, 0, rc->right, rc->bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bm);
    DeleteDC(mem);
    EndPaint(hwnd, ps);
}

static int toolbar_hit(int x, int y)
{
    RECT rc;
    int pad, bw, bh, i, top, yy;
    GetClientRect(g.toolbar, &rc);
    pad = tool_pad();
    bw = rc.right - pad * 2;
    if (bw < 8) bw = 8;
    bh = tool_btn_h();
    yy = y + g.tool_scroll;
    for (i = 0; i < TOOL_COUNT; i++) {
        top = pad + i * (bh + pad);
        if (x >= pad && x < pad + bw && yy >= top && yy < top + bh) return i;
    }
    top = pad + TOOL_COUNT * (bh + pad);
    if (x >= pad && x < pad + bw / 2 && yy >= top && yy < top + bh) return 100;
    if (x >= pad + bw / 2 && x < pad + bw && yy >= top && yy < top + bh) return 101;
    top += bh + pad;
    if (x >= pad && x < pad + bw && yy >= top && yy < top + bh) return 102;
    return -1;
}

void ui_scroll_tool_into_view(void)
{
    RECT rc;
    int pad, bh, top, bot, maxs;
    if (!g.toolbar) return;
    GetClientRect(g.toolbar, &rc);
    pad = tool_pad();
    bh = tool_btn_h();
    top = pad + g.tool * (bh + pad);
    bot = top + bh;
    if (top < g.tool_scroll) g.tool_scroll = top - pad;
    if (bot - g.tool_scroll > rc.bottom)
        g.tool_scroll = bot - rc.bottom + pad;
    maxs = pad + (TOOL_COUNT + 2) * (bh + pad) - rc.bottom;
    if (maxs < 0) maxs = 0;
    g.tool_scroll = clampi(g.tool_scroll, 0, maxs);
}

LRESULT CALLBACK ToolbarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC mem;
        HBITMAP bm;
        HGDIOBJ old, of;
        RECT rc;
        int pad, bw, bh, i;
        paint_begin(hwnd, &ps, &mem, &bm, &old, &rc);
        FillRect(mem, &rc, g.br_panel);
        pad = tool_pad();
        bw = rc.right - pad * 2;
        if (bw < 8) bw = 8;
        bh = tool_btn_h();
        of = SelectObject(mem, g.font_small);
        SetBkMode(mem, TRANSPARENT);
        for (i = 0; i < TOOL_COUNT; i++) {
            RECT b = { pad, pad + i * (bh + pad) - g.tool_scroll, pad + bw,
                       pad + i * (bh + pad) - g.tool_scroll + bh };
            FillRect(mem, &b, g.tool == i ? g.br_sel : g.br_panel);
            ui_draw_tool_icon(mem, i, &b, g.tool == i);
            if (i == TOOL_BRUSH) {
                SetTextColor(mem, RGB(200, 200, 210));
                DrawTextW(mem, L"▾", -1, &b, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
            }
        }
        {
            RECT fg, bg, sw;
            HBRUSH col;
            int top = pad + TOOL_COUNT * (bh + pad) - g.tool_scroll;
            fg.left = pad; fg.top = top; fg.right = pad + bw / 2 - 1; fg.bottom = top + bh;
            bg.left = pad + bw / 2 + 1; bg.top = top; bg.right = pad + bw; bg.bottom = top + bh;
            col = CreateSolidBrush(g.color);
            FillRect(mem, &fg, col);
            DeleteObject(col);
            col = CreateSolidBrush(g.color_bg);
            FillRect(mem, &bg, col);
            DeleteObject(col);
            FrameRect(mem, &fg, GetStockObject(WHITE_BRUSH));
            FrameRect(mem, &bg, GetStockObject(WHITE_BRUSH));
            sw.left = pad; sw.top = top + bh + pad; sw.right = pad + bw; sw.bottom = sw.top + bh;
            FillRect(mem, &sw, g.br_btn);
            SetTextColor(mem, RGB(235, 235, 240));
            DrawTextW(mem, L"전경↔배경", -1, &sw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        SelectObject(mem, of);
        paint_end(hwnd, &ps, mem, bm, old, &rc);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int maxs, pad = tool_pad(), bh = tool_btn_h();
        RECT rc;
        GetClientRect(hwnd, &rc);
        maxs = pad + (TOOL_COUNT + 2) * (bh + pad) - rc.bottom;
        if (maxs < 0) maxs = 0;
        g.tool_scroll = clampi(g.tool_scroll - delta / 4, 0, maxs);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int hit = toolbar_hit(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        toolbar_press = hit;
        if (hit == TOOL_BRUSH) {
            SetCapture(hwnd);
            return 0;
        }
        if (hit >= 0 && hit < TOOL_COUNT) {
            tools_set_tool(hit);
            InvalidateRect(hwnd, NULL, FALSE);
            SetFocus(g.canvas);
        } else if (hit == 100) {
            g.pick_bg = 0;
            ui_choose_color_slot(0);
        } else if (hit == 101) {
            g.pick_bg = 1;
            ui_choose_color_slot(1);
        } else if (hit == 102) {
            COLORREF t = g.color;
            g.color = g.color_bg;
            g.color_bg = t;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        if (GetCapture() == hwnd) ReleaseCapture();
        if (toolbar_press == TOOL_BRUSH) {
            if (brush_popup_visible())
                brush_popup_hide();
            else
                tools_set_tool(TOOL_BRUSH);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        toolbar_press = -1;
        return 0;
    }
    case WM_SETCURSOR:
        SetCursor(g.cur_arrow);
        return TRUE;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC mem;
        HBITMAP bm;
        HGDIOBJ old;
        RECT rc;
        int dx, dy, dw, dh;
        paint_begin(hwnd, &ps, &mem, &bm, &old, &rc);
        FillRect(mem, &rc, g.br_canvas);
        comp_ensure();
        if (g.disp && g.doc.w > 0 && g.doc.h > 0) {
            BITMAPINFO bmi;
            dx = (int)floorf(g.pan_x);
            dy = (int)floorf(g.pan_y);
            dw = (int)floorf(g.doc.w * g.zoom + 0.5f);
            dh = (int)floorf(g.doc.h * g.zoom + 0.5f);
            if (dw < 1) dw = 1;
            if (dh < 1) dh = 1;
            memset(&bmi, 0, sizeof(bmi));
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = g.doc.w;
            bmi.bmiHeader.biHeight = -g.doc.h;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            SetStretchBltMode(mem, COLORONCOLOR);
            StretchDIBits(mem, dx, dy, dw, dh, 0, 0, g.doc.w, g.doc.h,
                          g.disp, &bmi, DIB_RGB_COLORS, SRCCOPY);
            {
                HPEN pen = CreatePen(PS_SOLID, 1, RGB(70, 70, 80));
                HGDIOBJ op = SelectObject(mem, pen);
                HGDIOBJ ob = SelectObject(mem, GetStockObject(NULL_BRUSH));
                Rectangle(mem, dx - 1, dy - 1, dx + dw + 1, dy + dh + 1);
                SelectObject(mem, ob);
                SelectObject(mem, op);
                DeleteObject(pen);
            }
        }
        tools_draw_overlay(mem);
        paint_end(hwnd, &ps, mem, bm, old, &rc);
        return 0;
    }
    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        tools_on_down(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
        return 0;
    case WM_RBUTTONDOWN:
        SetFocus(hwnd);
        g.tool = TOOL_EYEDROPPER;
        InvalidateRect(g.toolbar, NULL, FALSE);
        tools_on_down(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
        return 0;
    case WM_MBUTTONDOWN:
        SetFocus(hwnd);
        tools_on_down(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);
        return 0;
    case WM_MOUSEMOVE:
        tools_on_move(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        tools_on_up(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MOUSEWHEEL: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        ScreenToClient(hwnd, &pt);
        view_zoom_at(pt.x, pt.y, delta > 0);
        ui_update_status();
        InvalidateRect(hwnd, NULL, FALSE);
        InvalidateRect(g.status, NULL, FALSE);
        return 0;
    }
    case WM_SETCURSOR: {
        POINT pt;
        if (g.panning || g.space_down || g.tool == TOOL_HAND) {
            SetCursor(g.cur_hand);
        } else if (g.tool == TOOL_MOVE) {
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            tools_set_cursor(pt.x, pt.y);
        } else {
            SetCursor(g.cur_cross);
        }
        return TRUE;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int layer_row_h(void)
{
    return ui_scale(40);
}

static int layer_list_top(void)
{
    return ui_scale(30);
}

static int layer_bottom_h(void)
{
    return ui_scale(156);
}

static int layer_index_at(int y)
{
    RECT rc;
    int top, bot, row, idx;
    GetClientRect(g.layers, &rc);
    top = layer_list_top();
    bot = rc.bottom - layer_bottom_h();
    if (y < top || y >= bot) return -1;
    row = (y - top + g.layer_scroll) / layer_row_h();
    if (row < 0 || row >= g.doc.count) return -1;
    idx = g.doc.count - 1 - row;
    return idx;
}

void ui_sync_layer_controls(void)
{
    Layer *L = doc_active(&g.doc);
    g.syncing = 1;
    if (g.opacity_track) {
        EnableWindow(g.opacity_track, L != NULL);
        SendMessageW(g.opacity_track, TBM_SETPOS, TRUE, L ? L->opacity : 255);
    }
    if (g.blend_combo) {
        EnableWindow(g.blend_combo, L != NULL);
        SendMessageW(g.blend_combo, CB_SETCURSEL, L ? L->blend : 0, 0);
    }
    g.syncing = 0;
}

LRESULT CALLBACK LayerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        g.opacity_track = CreateWindowExW(0, TRACKBAR_CLASSW, NULL,
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_OPACITY, g.inst, NULL);
        SendMessageW(g.opacity_track, TBM_SETRANGE, TRUE, MAKELPARAM(0, 255));
        SendMessageW(g.opacity_track, TBM_SETPOS, TRUE, 255);

        g.blend_combo = CreateWindowExW(0, L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BLEND, g.inst, NULL);
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"보통");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"곱하기");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"스크린");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"오버레이");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"어둡게");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"밝게");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"더하기");
        SendMessageW(g.blend_combo, CB_ADDSTRING, 0, (LPARAM)L"차이");
        SendMessageW(g.blend_combo, CB_SETCURSEL, 0, 0);

        g.btn_import = CreateWindowExW(0, L"BUTTON", L"사진 가져오기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_IMPORT, g.inst, NULL);
        g.btn_add = CreateWindowExW(0, L"BUTTON", L"추가", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_ADD, g.inst, NULL);
        g.btn_dup = CreateWindowExW(0, L"BUTTON", L"복제", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_DUP, g.inst, NULL);
        g.btn_del = CreateWindowExW(0, L"BUTTON", L"삭제", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_DEL, g.inst, NULL);
        g.btn_up = CreateWindowExW(0, L"BUTTON", L"위", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_UP, g.inst, NULL);
        g.btn_down = CreateWindowExW(0, L"BUTTON", L"아래", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)(intptr_t)IDC_BTN_DOWN, g.inst, NULL);

        SendMessageW(g.blend_combo, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_import, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_add, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_dup, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_del, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_up, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        SendMessageW(g.btn_down, WM_SETFONT, (WPARAM)g.font_small, TRUE);
        return 0;
    }
    case WM_SIZE: {
        RECT rc;
        int m, bh, bw, y, gap;
        GetClientRect(hwnd, &rc);
        m = ui_scale(8);
        bh = ui_scale(24);
        gap = ui_scale(4);
        y = rc.bottom - layer_bottom_h() + ui_scale(22);
        MoveWindow(g.opacity_track, m, y, rc.right - m * 2, ui_scale(22), TRUE);
        y += ui_scale(26);
        MoveWindow(g.blend_combo, m, y, rc.right - m * 2, ui_scale(120), TRUE);
        y = rc.bottom - ui_scale(86);
        MoveWindow(g.btn_import, m, y, rc.right - m * 2, bh, TRUE);
        y = rc.bottom - ui_scale(58);
        bw = (rc.right - m * 2 - gap * 2) / 3;
        MoveWindow(g.btn_add, m, y, bw, bh, TRUE);
        MoveWindow(g.btn_dup, m + bw + gap, y, bw, bh, TRUE);
        MoveWindow(g.btn_del, m + (bw + gap) * 2, y, bw, bh, TRUE);
        y += bh + gap;
        bw = (rc.right - m * 2 - gap) / 2;
        MoveWindow(g.btn_up, m, y, bw, bh, TRUE);
        MoveWindow(g.btn_down, m + bw + gap, y, bw, bh, TRUE);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC mem;
        HBITMAP bm;
        HGDIOBJ old, of;
        RECT rc, list, clip;
        int i, row, top, bot, rh;
        HRGN rgn;
        paint_begin(hwnd, &ps, &mem, &bm, &old, &rc);
        FillRect(mem, &rc, g.br_panel);
        of = SelectObject(mem, g.font);
        SetBkMode(mem, TRANSPARENT);
        SetTextColor(mem, RGB(230, 230, 235));
        {
            RECT t = { ui_scale(8), ui_scale(8), rc.right - ui_scale(8), ui_scale(28) };
            DrawTextW(mem, L"레이어", -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        top = layer_list_top();
        bot = rc.bottom - layer_bottom_h();
        SetRect(&list, 0, top, rc.right, bot > top ? bot : top);
        FillRect(mem, &list, g.br_bg);
        rgn = CreateRectRgn(list.left, list.top, list.right, list.bottom);
        SelectClipRgn(mem, rgn);

        rh = layer_row_h();
        SelectObject(mem, g.font_small);
        for (i = g.doc.count - 1, row = 0; i >= 0; i--, row++) {
            RECT item;
            Layer *L = &g.doc.layers[i];
            int y = top + row * rh - g.layer_scroll;
            wchar_t eye[4];
            if (y + rh < top || y > bot) continue;
            item.left = ui_scale(4);
            item.top = y + 2;
            item.right = rc.right - ui_scale(4);
            item.bottom = y + rh - 2;
            FillRect(mem, &item, i == g.doc.active ? g.br_sel : g.br_btn);
            wcscpy(eye, L->visible ? L"●" : L"○");
            {
                RECT er = { item.left, item.top, item.left + ui_scale(28), item.bottom };
                SetTextColor(mem, L->visible ? RGB(120, 200, 140) : RGB(120, 120, 130));
                DrawTextW(mem, eye, -1, &er, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            {
                RECT nr = { item.left + ui_scale(28), item.top, item.right - ui_scale(8), item.bottom };
                SetTextColor(mem, RGB(235, 235, 240));
                DrawTextW(mem, L->name, -1, &nr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            }
        }
        SelectClipRgn(mem, NULL);
        DeleteObject(rgn);

        SetRect(&clip, 0, bot, rc.right, rc.bottom);
        FillRect(mem, &clip, g.br_panel);
        SetTextColor(mem, RGB(200, 200, 210));
        SelectObject(mem, g.font_small);
        {
            Layer *L = doc_active(&g.doc);
            wchar_t buf[64];
            RECT t = { ui_scale(8), bot + ui_scale(4), rc.right - ui_scale(8), bot + ui_scale(22) };
            swprintf(buf, 64, L"불투명도  %d%%", L ? (L->opacity * 100 / 255) : 0);
            DrawTextW(mem, buf, -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        SelectObject(mem, of);
        paint_end(hwnd, &ps, mem, bm, old, &rc);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        int idx = layer_index_at(y);
        SetFocus(g.canvas);
        if (idx >= 0) {
            if (x < ui_scale(36)) {
                undo_push();
                g.doc.layers[idx].visible = !g.doc.layers[idx].visible;
                app_set_dirty(1);
                comp_invalidate_all();
            } else {
                g.doc.active = idx;
            }
            ui_sync_layer_controls();
            app_refresh();
        }
        return 0;
    }
    case WM_LBUTTONDBLCLK: {
        int idx = layer_index_at(GET_Y_LPARAM(lParam));
        if (idx >= 0) {
            g.doc.active = idx;
            app_command(IDM_LAYER_RENAME);
        }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int content, view, maxs;
        RECT rc;
        GetClientRect(hwnd, &rc);
        content = g.doc.count * layer_row_h();
        view = (rc.bottom - layer_bottom_h()) - layer_list_top();
        maxs = content > view ? content - view : 0;
        g.layer_scroll = clampi(g.layer_scroll - delta / 4, 0, maxs);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_HSCROLL:
        if ((HWND)lParam == g.opacity_track && !g.syncing) {
            Layer *L = doc_active(&g.doc);
            int code = LOWORD(wParam);
            if (L) {
                if (!g.opacity_undo && (code == TB_THUMBTRACK || code == TB_LINEUP ||
                    code == TB_LINEDOWN || code == TB_PAGEUP || code == TB_PAGEDOWN)) {
                    undo_push();
                    g.opacity_undo = 1;
                }
                if (code == TB_ENDTRACK || code == TB_THUMBPOSITION) g.opacity_undo = 0;
                L->opacity = (int)SendMessageW(g.opacity_track, TBM_GETPOS, 0, 0);
                app_set_dirty(1);
                comp_invalidate_all();
                InvalidateRect(g.canvas, NULL, FALSE);
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);
        if (id == IDC_BLEND && code == CBN_SELCHANGE && !g.syncing) {
            Layer *L = doc_active(&g.doc);
            int sel = (int)SendMessageW(g.blend_combo, CB_GETCURSEL, 0, 0);
            if (L && sel >= 0) {
                undo_push();
                L->blend = sel;
                app_set_dirty(1);
                comp_invalidate_all();
                app_refresh();
            }
        } else if (code == BN_CLICKED) {
            if (id == IDC_BTN_IMPORT) app_command(IDM_LAYER_FROM_FILE);
            else if (id == IDC_BTN_ADD) app_command(IDM_LAYER_NEW);
            else if (id == IDC_BTN_DUP) app_command(IDM_LAYER_DUP);
            else if (id == IDC_BTN_DEL) app_command(IDM_LAYER_DEL);
            else if (id == IDC_BTN_UP) app_command(IDM_LAYER_UP);
            else if (id == IDC_BTN_DOWN) app_command(IDM_LAYER_DOWN);
        }
        return 0;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
        SetBkColor((HDC)wParam, RGB(40, 40, 46));
        SetTextColor((HDC)wParam, RGB(230, 230, 235));
        return (LRESULT)g.br_panel;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ui_update_status(void)
{
    float dx = 0, dy = 0;
    int ix, iy;
    Layer *L = doc_active(&g.doc);
    view_to_doc(g.last_sx, g.last_sy, &dx, &dy);
    ix = (int)floorf(dx);
    iy = (int)floorf(dy);
    if (g.resize_on && L) {
        swprintf(g.status_text, 256,
                 L"크기 조절 %d × %d    모서리는 비율 유지, Shift는 자유    줌 %d%%",
                 L->w, L->h, (int)(g.zoom * 100.0f + 0.5f));
    } else if (g.tool == TOOL_BRUSH) {
        swprintf(g.status_text, 256,
                 L"%s    문서 %d × %d    줌 %d%%    좌표 %d, %d    크기 %d    %s    레이어 %d/%d",
                 brush_preset_name(g.brush_preset), g.doc.w, g.doc.h, (int)(g.zoom * 100.0f + 0.5f),
                 ix, iy, g.brush_size, g.have_sel ? L"선택됨" : L"",
                 g.doc.count ? g.doc.active + 1 : 0, g.doc.count);
    } else if (g.tool == TOOL_CLONE) {
        swprintf(g.status_text, 256,
                 L"도장    Alt+클릭으로 복제 원본    %s    브러시 %d    줌 %d%%",
                 g.clone_ok ? L"원본 지정됨" : L"원본 없음",
                 g.brush_size, (int)(g.zoom * 100.0f + 0.5f));
    } else {
        swprintf(g.status_text, 256,
                 L"%s    문서 %d × %d    줌 %d%%    좌표 %d, %d    브러시 %d    %s    레이어 %d/%d",
                 tools_name(g.tool), g.doc.w, g.doc.h, (int)(g.zoom * 100.0f + 0.5f),
                 ix, iy, g.brush_size, g.have_sel ? L"선택됨" : L"",
                 g.doc.count ? g.doc.active + 1 : 0, g.doc.count);
    }
}

LRESULT CALLBACK StatusProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC mem;
        HBITMAP bm;
        HGDIOBJ old, of;
        RECT rc;
        paint_begin(hwnd, &ps, &mem, &bm, &old, &rc);
        FillRect(mem, &rc, g.br_panel);
        of = SelectObject(mem, g.font_small);
        SetBkMode(mem, TRANSPARENT);
        SetTextColor(mem, RGB(200, 200, 210));
        {
            RECT t = rc;
            t.left += ui_scale(10);
            DrawTextW(mem, g.status_text, -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        SelectObject(mem, of);
        paint_end(hwnd, &ps, mem, bm, old, &rc);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ui_choose_color_slot(int bg)
{
    CHOOSECOLORW cc;
    memset(&cc, 0, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = g.hwnd;
    cc.rgbResult = bg ? g.color_bg : g.color;
    cc.lpCustColors = g.cust_colors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorW(&cc)) {
        if (bg) g.color_bg = cc.rgbResult;
        else g.color = cc.rgbResult;
        InvalidateRect(g.toolbar, NULL, FALSE);
        if (g.paintbar) InvalidateRect(g.paintbar, NULL, FALSE);
    }
}

void ui_choose_color(void)
{
    ui_choose_color_slot(g.pick_bg);
}

INT_PTR CALLBACK NewDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetWindowTextW(h, L"새 문서");
        SetDlgItemTextW(h, IDC_LBL_W, L"너비");
        SetDlgItemTextW(h, IDC_LBL_H, L"높이");
        SetDlgItemTextW(h, IDC_WHITEBG, L"흰 배경");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SetDlgItemInt(h, IDC_WIDTH, 1280, FALSE);
        SetDlgItemInt(h, IDC_HEIGHT, 720, FALSE);
        CheckDlgButton(h, IDC_WHITEBG, BST_CHECKED);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            BOOL okw = FALSE, okh = FALSE;
            int ww = (int)GetDlgItemInt(h, IDC_WIDTH, &okw, FALSE);
            int hh = (int)GetDlgItemInt(h, IDC_HEIGHT, &okh, FALSE);
            if (!okw || !okh || ww < 1 || hh < 1 || ww > DAGRIM_MAX_SIZE || hh > DAGRIM_MAX_SIZE) {
                MessageBoxW(h, L"크기는 1–8192 사이여야 합니다.", L"DaGrim", MB_ICONWARNING);
                return TRUE;
            }
            g.new_w = ww;
            g.new_h = hh;
            g.new_white = IsDlgButtonChecked(h, IDC_WHITEBG) == BST_CHECKED;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void bc_labels(HWND h)
{
    wchar_t b[64], c[64];
    int bv = (int)SendDlgItemMessageW(h, IDC_BRIGHT, TBM_GETPOS, 0, 0) - 100;
    int cv = (int)SendDlgItemMessageW(h, IDC_CONTRAST, TBM_GETPOS, 0, 0) - 100;
    swprintf(b, 64, L"밝기  %+d", bv);
    swprintf(c, 64, L"대비  %+d", cv);
    SetDlgItemTextW(h, IDC_LBL_B, b);
    SetDlgItemTextW(h, IDC_LBL_C, c);
}

INT_PTR CALLBACK BcDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetWindowTextW(h, L"밝기 / 대비");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SendDlgItemMessageW(h, IDC_BRIGHT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 200));
        SendDlgItemMessageW(h, IDC_CONTRAST, TBM_SETRANGE, TRUE, MAKELPARAM(0, 200));
        SendDlgItemMessageW(h, IDC_BRIGHT, TBM_SETPOS, TRUE, 100);
        SendDlgItemMessageW(h, IDC_CONTRAST, TBM_SETPOS, TRUE, 100);
        bc_labels(h);
        return TRUE;
    case WM_HSCROLL:
        bc_labels(h);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            g.adj_b = (int)SendDlgItemMessageW(h, IDC_BRIGHT, TBM_GETPOS, 0, 0) - 100;
            g.adj_c = (int)SendDlgItemMessageW(h, IDC_CONTRAST, TBM_GETPOS, 0, 0) - 100;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK SatDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG: {
        wchar_t s[64];
        SetWindowTextW(h, L"채도");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 200));
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETPOS, TRUE, 100);
        swprintf(s, 64, L"채도  %+d", 0);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_HSCROLL: {
        wchar_t s[64];
        int v = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0) - 100;
        swprintf(s, 64, L"채도  %+d", v);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            g.adj_s = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0) - 100;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK RenameDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetWindowTextW(h, L"레이어 이름");
        SetDlgItemTextW(h, IDC_LBL_N, L"이름");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SetDlgItemTextW(h, IDC_NAME, g.rename_buf);
        SendDlgItemMessageW(h, IDC_NAME, EM_SETSEL, 0, -1);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            GetDlgItemTextW(h, IDC_NAME, g.rename_buf, 64);
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK ScaleDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    static int orig_w, orig_h, busy;
    (void)l;
    switch (m) {
    case WM_INITDIALOG: {
        Layer *L = doc_active(&g.doc);
        orig_w = g.new_w > 0 ? g.new_w : (L ? L->w : 1);
        orig_h = g.new_h > 0 ? g.new_h : (L ? L->h : 1);
        busy = 1;
        SetWindowTextW(h, L"크기 조절");
        SetDlgItemTextW(h, IDC_LBL_W, L"너비");
        SetDlgItemTextW(h, IDC_LBL_H, L"높이");
        SetDlgItemTextW(h, IDC_KEEP_ASPECT, L"비율 유지");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SetDlgItemInt(h, IDC_WIDTH, orig_w, FALSE);
        SetDlgItemInt(h, IDC_HEIGHT, orig_h, FALSE);
        CheckDlgButton(h, IDC_KEEP_ASPECT, BST_CHECKED);
        busy = 0;
        return TRUE;
    }
    case WM_COMMAND: {
        int id = LOWORD(w);
        int code = HIWORD(w);
        if (id == IDOK) {
            BOOL okw = FALSE, okh = FALSE;
            int ww = (int)GetDlgItemInt(h, IDC_WIDTH, &okw, FALSE);
            int hh = (int)GetDlgItemInt(h, IDC_HEIGHT, &okh, FALSE);
            if (!okw || !okh || ww < 1 || hh < 1 || ww > DAGRIM_MAX_SIZE || hh > DAGRIM_MAX_SIZE) {
                MessageBoxW(h, L"크기는 1–8192 사이여야 합니다.", L"DaGrim", MB_ICONWARNING);
                return TRUE;
            }
            g.new_w = ww;
            g.new_h = hh;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (id == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        if (!busy && code == EN_CHANGE && IsDlgButtonChecked(h, IDC_KEEP_ASPECT) == BST_CHECKED &&
            orig_w > 0 && orig_h > 0) {
            BOOL ok = FALSE;
            int v = (int)GetDlgItemInt(h, id, &ok, FALSE);
            if (!ok || v < 1) break;
            busy = 1;
            if (id == IDC_WIDTH)
                SetDlgItemInt(h, IDC_HEIGHT, (int)((double)v * orig_h / orig_w + 0.5), FALSE);
            else if (id == IDC_HEIGHT)
                SetDlgItemInt(h, IDC_WIDTH, (int)((double)v * orig_w / orig_h + 0.5), FALSE);
            busy = 0;
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

INT_PTR CALLBACK TextDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetWindowTextW(h, L"문자");
        SetDlgItemTextW(h, IDC_LBL_T, L"내용");
        SetDlgItemTextW(h, IDC_LBL_FS, L"크기");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SetDlgItemTextW(h, IDC_TEXT, g.text_buf);
        SetDlgItemInt(h, IDC_FONTSIZE, g.text_size > 0 ? g.text_size : 48, FALSE);
        SendDlgItemMessageW(h, IDC_TEXT, EM_SETSEL, 0, -1);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            BOOL ok = FALSE;
            int sz;
            GetDlgItemTextW(h, IDC_TEXT, g.text_buf, 256);
            sz = (int)GetDlgItemInt(h, IDC_FONTSIZE, &ok, FALSE);
            if (!ok || sz < 8 || sz > 400) {
                MessageBoxW(h, L"글자 크기는 8–400 사이여야 합니다.", L"DaGrim", MB_ICONWARNING);
                return TRUE;
            }
            g.text_size = sz;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK HueDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG: {
        wchar_t s[64];
        SetWindowTextW(h, L"색조");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 360));
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETPOS, TRUE, 180);
        swprintf(s, 64, L"색조  %+d", 0);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_HSCROLL: {
        wchar_t s[64];
        int v = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0) - 180;
        swprintf(s, 64, L"색조  %+d", v);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            g.adj_s = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0) - 180;
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK BlurDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG: {
        wchar_t s[64];
        int r = g.filt_r > 0 ? g.filt_r : 3;
        SetWindowTextW(h, L"흐리게");
        SetDlgItemTextW(h, IDOK, L"확인");
        SetDlgItemTextW(h, IDCANCEL, L"취소");
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETRANGE, TRUE, MAKELPARAM(1, 20));
        SendDlgItemMessageW(h, IDC_SAT, TBM_SETPOS, TRUE, r);
        swprintf(s, 64, L"반지름  %d", r);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_HSCROLL: {
        wchar_t s[64];
        int v = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0);
        swprintf(s, 64, L"반지름  %d", v);
        SetDlgItemTextW(h, IDC_LBL_S, s);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            g.filt_r = (int)SendDlgItemMessageW(h, IDC_SAT, TBM_GETPOS, 0, 0);
            EndDialog(h, IDOK);
            return TRUE;
        }
        if (LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK AboutDlgProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    (void)l;
    switch (m) {
    case WM_INITDIALOG:
        SetWindowTextW(h, L"DaGrim 정보");
        SetDlgItemTextW(h, IDC_ABOUT_TEXT,
            L"DaGrim v" DAGRIM_VERSION_WSTRING L"\n레이어 사진 편집기  ·  선택/브러시/도장/문자/필터\nPNG / JPEG / BMP / .dagrim");
        SetDlgItemTextW(h, IDOK, L"확인");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK || LOWORD(w) == IDCANCEL) {
            EndDialog(h, IDOK);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void ui_register_classes(HINSTANCE inst)
{
    WNDCLASSW wc;
    memset(&wc, 0, sizeof(wc));
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wc.lpfnWndProc = ToolbarProc;
    wc.lpszClassName = L"DaGrimToolbar";
    RegisterClassW(&wc);

    wc.lpfnWndProc = CanvasProc;
    wc.lpszClassName = L"DaGrimCanvas";
    RegisterClassW(&wc);

    wc.lpfnWndProc = LayerProc;
    wc.lpszClassName = L"DaGrimLayers";
    RegisterClassW(&wc);

    wc.lpfnWndProc = StatusProc;
    wc.lpszClassName = L"DaGrimStatus";
    RegisterClassW(&wc);

    wc.lpfnWndProc = BrushPopupProc;
    wc.lpszClassName = L"DaGrimBrushPopup";
    wc.hbrBackground = g.br_panel ? g.br_panel : (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
}

typedef struct ValueDialog { const wchar_t *title,*label; int lo,hi,value; } ValueDialog;
static void value_label(HWND h,ValueDialog *v) { wchar_t t[128]; swprintf(t,128,L"%ls  %d",v->label,v->value); SetDlgItemTextW(h,IDC_LBL_S,t); }
static INT_PTR CALLBACK ValueDlgProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 ValueDialog *v=(ValueDialog*)GetWindowLongPtrW(h,DWLP_USER);
 if(m==WM_INITDIALOG) { v=(ValueDialog*)l; SetWindowLongPtrW(h,DWLP_USER,l); SetWindowTextW(h,v->title); SetDlgItemTextW(h,IDOK,L"확인"); SetDlgItemTextW(h,IDCANCEL,L"취소"); SendDlgItemMessageW(h,IDC_SAT,TBM_SETRANGE,TRUE,MAKELPARAM(v->lo,v->hi)); SendDlgItemMessageW(h,IDC_SAT,TBM_SETPOS,TRUE,v->value); value_label(h,v); return TRUE; }
 if(!v) return FALSE;
 if(m==WM_HSCROLL) { v->value=(int)SendDlgItemMessageW(h,IDC_SAT,TBM_GETPOS,0,0); value_label(h,v); return TRUE; }
 if(m==WM_COMMAND && (LOWORD(w)==IDOK || LOWORD(w)==IDCANCEL)) { EndDialog(h,LOWORD(w)); return TRUE; } return FALSE;
}
int ui_value_dialog(const wchar_t *title,const wchar_t *label,int lo,int hi,int *value) {
 ValueDialog v={title,label,lo,hi,clampi(*value,lo,hi)};
 if(DialogBoxParamW(g.inst,MAKEINTRESOURCEW(IDD_SAT),g.hwnd,ValueDlgProc,(LPARAM)&v)!=IDOK) return 0;
 *value=v.value; return 1;
}
