#include "app.h"

static const COLORREF g_palette[] = {
    RGB(0, 0, 0), RGB(128, 128, 128), RGB(255, 255, 255), RGB(255, 0, 0),
    RGB(255, 128, 0), RGB(255, 255, 0), RGB(0, 180, 0), RGB(0, 128, 255),
    RGB(0, 0, 255), RGB(128, 0, 255), RGB(255, 0, 128), RGB(128, 64, 0),
    RGB(0, 128, 128), RGB(64, 128, 255), RGB(200, 200, 200), RGB(255, 200, 200),
    RGB(255, 255, 200), RGB(200, 255, 200), RGB(200, 220, 255), RGB(230, 200, 255)
};
#define PALETTE_COUNT ((int)(sizeof(g_palette) / sizeof(g_palette[0])))

static const int g_widths[] = { 1, 2, 3, 5, 8, 12, 20, 40 };
#define WIDTH_COUNT 8

static const int g_quick_shapes[] = {
    SHAPE_LINE, SHAPE_CURVE, SHAPE_RECT, SHAPE_ROUNDRECT, SHAPE_ELLIPSE,
    SHAPE_CIRCLE, SHAPE_TRI, SHAPE_RTRI, SHAPE_DIAMOND, SHAPE_STAR5
};
#define QUICK_SHAPE_COUNT 10

static int bar_cell(void)
{
    int s = ui_scale(28);
    return s < 24 ? 24 : s;
}

static int bar_pad(void)
{
    int p = ui_scale(4);
    return p < 4 ? 4 : p;
}

/* Shared 24-unit silhouettes, supersampled for clean small toolbar icons. */
static void icon_line(HDC dc, const POINT *p, int n) { Polyline(dc,p,n); }
#define IL(...) do { const POINT p[]={__VA_ARGS__}; icon_line(dc,p,sizeof(p)/sizeof(p[0])); } while(0)
#define V(x,y) {x,y}
void ui_draw_tool_icon(HDC hdc, int tool, const RECT *rc, int selected)
{
    int side=rc->right-rc->left, height=rc->bottom-rc->top, x,y,n,state;
    HDC dc; HBITMAP bm; HGDIOBJ old; HPEN pen; HBRUSH ink;
    COLORREF fg=selected?RGB(255,255,255):RGB(204,208,214);
    if(height<side) side=height;
    side-=ui_scale(4); if(side<8) return;
    x=(rc->left+rc->right-side)/2; y=(rc->top+rc->bottom-side)/2; n=side*3;
    dc=CreateCompatibleDC(hdc); bm=CreateCompatibleBitmap(hdc,n,n); old=SelectObject(dc,bm);
    StretchBlt(dc,0,0,n,n,hdc,x,y,side,side,SRCCOPY);
    state=SaveDC(dc); SetMapMode(dc,MM_ANISOTROPIC);
    SetWindowExtEx(dc,24,24,NULL); SetViewportExtEx(dc,n,n,NULL);
    pen=CreatePen(PS_SOLID,1,fg); ink=CreateSolidBrush(fg);
    SelectObject(dc,pen); SelectObject(dc,GetStockObject(NULL_BRUSH));
    switch(tool) {
    case TOOL_MOVE:
        IL(V(3,12),V(21,12)); IL(V(12,3),V(12,21));
        IL(V(6,9),V(3,12),V(6,15)); IL(V(18,9),V(21,12),V(18,15));
        IL(V(9,6),V(12,3),V(15,6)); IL(V(9,18),V(12,21),V(15,18)); break;
    case TOOL_SEL_RECT:
        for(int i=4;i<20;i+=4) { IL(V(i,5),V(i+2,5)); IL(V(i,19),V(i+2,19)); IL(V(4,i+1),V(4,i+3)); IL(V(20,i+1),V(20,i+3)); } break;
    case TOOL_SEL_ELLIPSE:
        for(int i=0;i<12;i++) { float a=i*6.2831853f/12; POINT p[2]={{12+(int)(8*cosf(a)),12+(int)(7*sinf(a))},{12+(int)(8*cosf(a+.28f)),12+(int)(7*sinf(a+.28f))}}; Polyline(dc,p,2); } break;
    case TOOL_LASSO:
        Ellipse(dc,3,4,21,16); IL(V(7,14),V(7,19),V(12,21),V(14,18),V(10,15)); break;
    case TOOL_WAND:
        IL(V(5,21),V(17,9),V(15,7),V(3,19),V(5,21)); IL(V(12,10),V(14,12));
        IL(V(18,2),V(18,6)); IL(V(20,8),V(23,8)); IL(V(10,3),V(13,6)); break;
    case TOOL_CROP:
        IL(V(7,2),V(7,17),V(22,17)); IL(V(2,7),V(17,7),V(17,22)); IL(V(10,14),V(21,3)); break;
    case TOOL_EYEDROPPER:
        IL(V(4,20),V(6,15),V(15,6),V(18,9),V(9,18),V(4,20));
        SelectObject(dc,ink); Polygon(dc,(POINT[]){V(14,5),V(17,2),V(22,7),V(19,10)},4); IL(V(12,5),V(19,12)); break;
    case TOOL_BRUSH:
        IL(V(9,14),V(17,3),V(20,3),V(20,6),V(12,16),V(9,14));
        SelectObject(dc,ink); Polygon(dc,(POINT[]){V(8,14),V(12,17),V(10,21),V(3,21),V(6,18)},5); break;
    case TOOL_PENCIL:
        IL(V(4,20),V(6,14),V(17,3),V(21,7),V(10,18),V(4,20)); IL(V(6,14),V(10,18)); IL(V(15,5),V(19,9)); break;
    case TOOL_ERASER:
        IL(V(3,14),V(13,4),V(21,12),V(13,20),V(9,20),V(3,14)); IL(V(8,9),V(16,17)); IL(V(13,20),V(22,20)); break;
    case TOOL_CLONE:
        RoundRect(dc,9,3,16,11,4,4); IL(V(10,10),V(10,14),V(5,16),V(4,20),V(21,20),V(20,16),V(15,14),V(15,10)); IL(V(5,23),V(20,23)); break;
    case TOOL_BUCKET:
        IL(V(3,12),V(12,3),V(20,11),V(11,20),V(3,12)); IL(V(4,12),V(19,12)); IL(V(7,2),V(12,7));
        SelectObject(dc,ink); Polygon(dc,(POINT[]){V(20,14),V(17,20),V(20,22),V(23,20)},4); break;
    case TOOL_GRADIENT:
        Rectangle(dc,3,5,22,20); for(int i=4;i<13;i+=2) { IL(V(i,6),V(i,19)); } IL(V(15,6),V(15,19)); break;
    case TOOL_DODGE:
        Ellipse(dc,9,3,21,15); IL(V(11,13),V(4,21)); SelectObject(dc,ink); Ellipse(dc,12,6,18,12); break;
    case TOOL_BURN:
        IL(V(5,20),V(3,13),V(5,9),V(8,12),V(8,5),V(11,4),V(13,10),V(17,8),V(21,12),V(20,18),V(16,21),V(5,20)); break;
    case TOOL_SPONGE:
        IL(V(3,9),V(7,4),V(17,4),V(21,9),V(19,19),V(6,21),V(3,9));
        Ellipse(dc,7,8,10,11); Ellipse(dc,14,7,17,10); Ellipse(dc,10,14,13,17); break;
    case TOOL_TEXT:
        IL(V(4,7),V(4,4),V(20,4),V(20,7)); IL(V(12,4),V(12,21)); IL(V(8,21),V(16,21)); break;
    case TOOL_SHAPE_RECT: Rectangle(dc,4,5,21,20); break;
    case TOOL_SHAPE_ELLIPSE: Ellipse(dc,3,5,22,20); break;
    case TOOL_SHAPE_LINE: IL(V(4,20),V(20,4)); break;
    case TOOL_HAND:
        IL(V(7,21),V(3,13),V(4,11),V(8,14),V(7,6),V(9,5),V(11,11),V(11,3),V(14,3),V(14,11),V(16,5),V(18,6),V(17,13),V(20,9),V(22,11),V(20,19),V(17,22),V(7,21)); break;
    }
    RestoreDC(dc,state); DeleteObject(pen); DeleteObject(ink);
    state=SaveDC(hdc); SetStretchBltMode(hdc,HALFTONE); SetBrushOrgEx(hdc,0,0,NULL);
    StretchBlt(hdc,x,y,side,side,dc,0,0,n,n,SRCCOPY); RestoreDC(hdc,state);
    SelectObject(dc,old); DeleteObject(bm); DeleteDC(dc);
}
#undef IL
#undef V
static void draw_mini_shape(HDC hdc, RECT r, int kind, int on)
{
    HPEN pen;
    int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
    int rx = (r.right - r.left) / 2 - 3, ry = (r.bottom - r.top) / 2 - 3;
    pen = CreatePen(PS_SOLID, 1, on ? RGB(255, 255, 255) : RGB(200, 200, 210));
    SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    switch (kind) {
    case SHAPE_LINE:
        MoveToEx(hdc, r.left + 3, r.bottom - 4, NULL);
        LineTo(hdc, r.right - 3, r.top + 4);
        break;
    case SHAPE_CURVE:
        MoveToEx(hdc, r.left + 3, r.bottom - 4, NULL);
        LineTo(hdc, cx, r.top + 4);
        LineTo(hdc, r.right - 3, r.bottom - 4);
        break;
    case SHAPE_RECT:
        Rectangle(hdc, r.left + 3, r.top + 5, r.right - 3, r.bottom - 3);
        break;
    case SHAPE_ROUNDRECT:
        RoundRect(hdc, r.left + 3, r.top + 5, r.right - 3, r.bottom - 3, 6, 6);
        break;
    case SHAPE_ELLIPSE:
    case SHAPE_CIRCLE:
        Ellipse(hdc, cx - rx, cy - ry + 2, cx + rx, cy + ry);
        break;
    case SHAPE_TRI:
        Polygon(hdc, (POINT[]){ { cx, r.top + 4 }, { r.right - 3, r.bottom - 3 }, { r.left + 3, r.bottom - 3 } }, 3);
        break;
    case SHAPE_RTRI:
        Polygon(hdc, (POINT[]){ { r.left + 3, r.top + 4 }, { r.left + 3, r.bottom - 3 }, { r.right - 3, r.bottom - 3 } }, 3);
        break;
    case SHAPE_DIAMOND:
        Polygon(hdc, (POINT[]){ { cx, r.top + 4 }, { r.right - 3, cy }, { cx, r.bottom - 3 }, { r.left + 3, cy } }, 4);
        break;
    case SHAPE_STAR5:
        Ellipse(hdc, cx - rx + 2, cy - ry, cx + rx - 2, cy + ry);
        break;
    default:
        Rectangle(hdc, r.left + 3, r.top + 5, r.right - 3, r.bottom - 3);
        break;
    }
    DeleteObject(pen);
}

static int paintbar_hit(int x, int y, int *out)
{
    RECT rc;
    int pad = bar_pad(), cell = bar_cell(), i, x0, cols;
    GetClientRect(g.paintbar, &rc);
    x0 = pad;
    if (y >= pad && y < pad + cell && x >= x0 && x < x0 + QUICK_SHAPE_COUNT * (cell + 2)) {
        i = (x - x0) / (cell + 2);
        if (i >= 0 && i < QUICK_SHAPE_COUNT) {
            *out = 1000 + g_quick_shapes[i];
            return 1;
        }
    }
    x0 += QUICK_SHAPE_COUNT * (cell + 2) + pad;
    if (y >= pad && y < pad + cell && x >= x0 && x < x0 + WIDTH_COUNT * (cell + 2)) {
        i = (x - x0) / (cell + 2);
        if (i >= 0 && i < WIDTH_COUNT) {
            *out = 2000 + i;
            return 1;
        }
    }
    x0 += WIDTH_COUNT * (cell + 2) + pad;
    if (x >= x0 && x < x0 + cell * 3 + 4 && y >= pad && y < pad + cell) {
        if (x < x0 + cell) { *out = 3000; return 1; }
        if (x < x0 + cell * 2 + 2) { *out = 3001; return 1; }
        *out = 3002;
        return 1;
    }
    x0 += cell * 3 + pad + pad;
    if (x >= x0 && x < x0 + cell) {
        if (y >= pad && y < pad + cell) { *out = 4000; return 1; }
        if (y >= pad + cell + 2 && y < pad + cell * 2 + 2) { *out = 4001; return 1; }
    }
    x0 += cell + pad;
    cols = 10;
    for (i = 0; i < PALETTE_COUNT; i++) {
        int col = i % cols, row = i / cols;
        int left = x0 + col * (cell / 2 + 2);
        int top = pad + row * (cell / 2 + 2);
        if (x >= left && x < left + cell / 2 && y >= top && y < top + cell / 2) {
            *out = 5000 + i;
            return 1;
        }
    }
    return 0;
}

static void pick_shape(int kind)
{
    tools_set_shape(kind);
}

LRESULT CALLBACK PaintbarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc, mem;
        HBITMAP bm, old;
        RECT rc, r;
        int pad = bar_pad(), cell = bar_cell(), i, x0;
        wchar_t buf[16];
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        mem = CreateCompatibleDC(hdc);
        bm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        old = SelectObject(mem, bm);
        FillRect(mem, &rc, g.br_panel);
        x0 = pad;
        for (i = 0; i < QUICK_SHAPE_COUNT; i++) {
            int kind = g_quick_shapes[i];
            int on = (g.tool == TOOL_SHAPE_RECT || g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE) &&
                     g.shape_kind == kind;
            r.left = x0 + i * (cell + 2);
            r.top = pad;
            r.right = r.left + cell;
            r.bottom = r.top + cell;
            FillRect(mem, &r, on ? g.br_sel : g.br_btn);
            draw_mini_shape(mem, r, kind, on);
        }
        x0 += QUICK_SHAPE_COUNT * (cell + 2) + pad;
        for (i = 0; i < WIDTH_COUNT; i++) {
            r.left = x0 + i * (cell + 2);
            r.top = pad;
            r.right = r.left + cell;
            r.bottom = r.top + cell;
            FillRect(mem, &r, g.stroke_width == g_widths[i] ? g.br_sel : g.br_btn);
            SetBkMode(mem, TRANSPARENT);
            SetTextColor(mem, RGB(230, 230, 235));
            swprintf(buf, 16, L"%d", g_widths[i]);
            DrawTextW(mem, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        x0 += WIDTH_COUNT * (cell + 2) + pad;
        r.left = x0; r.top = pad; r.right = r.left + cell; r.bottom = r.top + cell;
        FillRect(mem, &r, g.shape_fill ? g.br_sel : g.br_btn);
        DrawTextW(mem, L"채움", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        r.left = x0 + cell + 2; r.right = r.left + cell;
        FillRect(mem, &r, g.shape_outline ? g.br_sel : g.br_btn);
        DrawTextW(mem, L"윤곽", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        r.left = x0 + (cell + 2) * 2; r.right = r.left + cell;
        FillRect(mem, &r, g.br_btn);
        DrawTextW(mem, L"선종", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        x0 += cell * 3 + pad + pad;
        {
            HBRUSH b;
            r.left = x0; r.top = pad; r.right = r.left + cell; r.bottom = r.top + cell;
            b = CreateSolidBrush(g.color);
            FillRect(mem, &r, b);
            DeleteObject(b);
            FrameRect(mem, &r, GetStockObject(WHITE_BRUSH));
            r.top += cell + 2; r.bottom = r.top + cell;
            b = CreateSolidBrush(g.color_bg);
            FillRect(mem, &r, b);
            DeleteObject(b);
            FrameRect(mem, &r, GetStockObject(WHITE_BRUSH));
        }
        x0 += cell + pad;
        for (i = 0; i < PALETTE_COUNT; i++) {
            int col = i % 10, row = i / 10;
            HBRUSH b;
            r.left = x0 + col * (cell / 2 + 2);
            r.top = pad + row * (cell / 2 + 2);
            r.right = r.left + cell / 2;
            r.bottom = r.top + cell / 2;
            b = CreateSolidBrush(g_palette[i]);
            FillRect(mem, &r, b);
            DeleteObject(b);
            FrameRect(mem, &r, GetStockObject(WHITE_BRUSH));
        }
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bm);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int code, x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
        if (!paintbar_hit(x, y, &code)) return 0;
        if (code >= 1000 && code < 1000 + QUICK_SHAPE_COUNT) {
            pick_shape(g_quick_shapes[code - 1000]);
        } else if (code >= 2000 && code < 2000 + WIDTH_COUNT) {
            g.stroke_width = g_widths[code - 2000];
            g.brush_size = g.stroke_width;
        } else if (code == 3000) {
            g.shape_fill = !g.shape_fill;
        } else if (code == 3001) {
            g.shape_outline = !g.shape_outline;
            if (!g.shape_outline && !g.shape_fill) g.shape_fill = 1;
        } else if (code == 3002) {
            brush_popup_show();
        } else if (code >= 5000 && code < 5000 + PALETTE_COUNT) {
            g.color = g_palette[code - 5000];
        } else if (code == 4000) {
            g.pick_bg = 0;
            ui_choose_color_slot(0);
            return 0;
        } else if (code == 4001) {
            g.pick_bg = 1;
            ui_choose_color_slot(1);
            return 0;
        }
        InvalidateRect(hwnd, NULL, FALSE);
        if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
        ui_update_status();
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void paintbar_register(HINSTANCE inst)
{
    WNDCLASSW wc;
    memset(&wc, 0, sizeof(wc));
    wc.hInstance = inst;
    wc.lpfnWndProc = PaintbarProc;
    wc.lpszClassName = L"DaGrimPaintbar";
    wc.hbrBackground = g.br_panel ? g.br_panel : (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);
}
