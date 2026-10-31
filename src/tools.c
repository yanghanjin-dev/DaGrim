#include "app.h"

int view_to_doc(int vx, int vy, float *dx, float *dy)
{
    if (g.zoom <= 0.0001f) return 0;
    *dx = ((float)vx - g.pan_x) / g.zoom;
    *dy = ((float)vy - g.pan_y) / g.zoom;
    return 1;
}

void doc_to_view(float dx, float dy, int *vx, int *vy)
{
    *vx = (int)floorf(dx * g.zoom + g.pan_x);
    *vy = (int)floorf(dy * g.zoom + g.pan_y);
}

void view_fit(void)
{
    RECT rc;
    int cw, ch;
    float zx, zy, dw, dh;
    if (!g.canvas || g.doc.w < 1 || g.doc.h < 1) return;
    GetClientRect(g.canvas, &rc);
    cw = rc.right;
    ch = rc.bottom;
    if (cw < 1 || ch < 1) return;
    zx = (float)cw / (float)g.doc.w;
    zy = (float)ch / (float)g.doc.h;
    g.zoom = (zx < zy ? zx : zy) * 0.92f;
    g.zoom = clampf(g.zoom, 0.02f, 64.0f);
    dw = g.doc.w * g.zoom;
    dh = g.doc.h * g.zoom;
    g.pan_x = (cw - dw) * 0.5f;
    g.pan_y = (ch - dh) * 0.5f;
    g.fitted = 1;
}

void view_actual(void)
{
    RECT rc;
    int cw, ch;
    if (!g.canvas) return;
    GetClientRect(g.canvas, &rc);
    cw = rc.right;
    ch = rc.bottom;
    g.zoom = 1.0f;
    g.pan_x = (cw - g.doc.w) * 0.5f;
    g.pan_y = (ch - g.doc.h) * 0.5f;
}

void view_zoom_at(int mx, int my, int zoom_in)
{
    float factor = zoom_in ? 1.12f : (1.0f / 1.12f);
    float nz = clampf(g.zoom * factor, 0.02f, 64.0f);
    if (g.zoom <= 0.0001f) g.zoom = 1.0f;
    g.pan_x = mx - (mx - g.pan_x) * (nz / g.zoom);
    g.pan_y = my - (my - g.pan_y) * (nz / g.zoom);
    g.zoom = nz;
}

static int stamp_mode(void)
{
    if (g.tool == TOOL_ERASER) return 1;
    if (g.tool == TOOL_PENCIL) return 2;
    if (g.tool == TOOL_CLONE) return 3;
    if (g.tool == TOOL_DODGE) return 4;
    if (g.tool == TOOL_BURN) return 5;
    if (g.tool == TOOL_SPONGE) return 6;
    return 0;
}

static void stamp_brush(float fx, float fy, int mode)
{
    Layer *L = doc_active(&g.doc);
    float rad, inner;
    int x0, y0, x1, y1, x, y;
    uint8_t sb, sg, sr;
    if (!L || !L->bgra) return;
    rad = (float)g.brush_size * 0.5f;
    if (rad < 0.5f) rad = 0.5f;
    inner = (mode == 2) ? rad : rad * 0.62f;
    x0 = (int)floorf(fx - rad - 1.0f);
    y0 = (int)floorf(fy - rad - 1.0f);
    x1 = (int)ceilf(fx + rad + 1.0f);
    y1 = (int)ceilf(fy + rad + 1.0f);
    if (mode == 0 || mode == 2) {
        brush_paint(fx, fy, mode);
        return;
    }

    sb = GetBValue(g.color);
    sg = GetGValue(g.color);
    sr = GetRValue(g.color);

    for (y = y0; y < y1; y++) {
        int ly = y - L->y;
        if (ly < 0 || ly >= L->h) continue;
        for (x = x0; x < x1; x++) {
            int lx = x - L->x, m, pa;
            float ddx, ddy, d, k;
            uint8_t *p;
            if (lx < 0 || lx >= L->w) continue;
            m = sel_mask(x, y);
            if (m <= 0) continue;
            ddx = (x + 0.5f) - fx;
            ddy = (y + 0.5f) - fy;
            d = sqrtf(ddx * ddx + ddy * ddy);
            if (d > rad) continue;
            if (d <= inner) k = 1.0f;
            else k = 1.0f - (d - inner) / (rad - inner + 0.0001f);
            pa = (int)(g.brush_opacity * k * m / 255.0f);
            if (pa <= 0) continue;
            p = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
            if (mode == 1) {
                p[3] = (uint8_t)(p[3] * (255 - pa) / 255);
            } else if (mode == 3) {
                int sx = (int)floorf(x - g.clone_ox);
                int sy = (int)floorf(y - g.clone_oy);
                int slx = sx - L->x, sly = sy - L->y;
                uint8_t src[4];
                if (slx < 0 || sly < 0 || slx >= L->w || sly >= L->h) continue;
                memcpy(src, L->bgra + ((size_t)sly * (size_t)L->w + (size_t)slx) * 4, 4);
                src[3] = (uint8_t)(src[3] * pa / 255);
                blend_pixel(p, src, 255, BLEND_NORMAL);
            } else if (mode == 4 || mode == 5) {
                int ch, amt = pa / 6;
                for (ch = 0; ch < 3; ch++) {
                    int v = p[ch];
                    if (mode == 4) v += amt + (255 - v) * amt / 255;
                    else v -= amt + v * amt / 255;
                    p[ch] = clampu8(v);
                }
            } else if (mode == 6) {
                float gray = 0.114f * p[0] + 0.587f * p[1] + 0.299f * p[2];
                float t = pa / 255.0f;
                int ch;
                for (ch = 0; ch < 3; ch++)
                    p[ch] = clampu8((int)(p[ch] + (gray - p[ch]) * t + 0.5f));
            } else {
                uint8_t src[4] = { sb, sg, sr, (uint8_t)pa };
                blend_pixel(p, src, 255, BLEND_NORMAL);
            }
        }
    }
    comp_invalidate_rect(x0, y0, x1 - x0, y1 - y0);
}

static void stroke_to(float x, float y, int mode)
{
    float dx, dy, dist;
    int steps, i;
    if (mode == 3 && !g.have_stroke) {
        g.clone_ox = x - g.clone_sx;
        g.clone_oy = y - g.clone_sy;
    }
    if (!g.have_stroke) {
        g.stroke_dx = 1.0f;
        g.stroke_dy = 0.0f;
        g.stroke_len = 0;
        stamp_brush(x, y, mode);
        g.stroke_x = x;
        g.stroke_y = y;
        g.have_stroke = 1;
        return;
    }
    dx = x - g.stroke_x;
    dy = y - g.stroke_y;
    dist = sqrtf(dx * dx + dy * dy);
    if (dist > 0.001f) {
        g.stroke_dx = dx;
        g.stroke_dy = dy;
    }
    {
        float sp = (mode == 0 || mode == 2) ? brush_spacing() : 1.0f;
        steps = (int)(dist / sp);
        if (steps < 1) steps = 1;
    }
    for (i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        stamp_brush(g.stroke_x + dx * t, g.stroke_y + dy * t, mode);
        g.stroke_len += dist / (float)steps;
    }
    g.stroke_x = x;
    g.stroke_y = y;
}

static int is_paint_tool(int t)
{
    return t == TOOL_BRUSH || t == TOOL_PENCIL || t == TOOL_ERASER ||
           t == TOOL_CLONE || t == TOOL_DODGE || t == TOOL_BURN || t == TOOL_SPONGE;
}

static int is_sel_tool(int t)
{
    return t == TOOL_SEL_RECT || t == TOOL_SEL_ELLIPSE || t == TOOL_LASSO || t == TOOL_WAND;
}

const wchar_t *tools_name(int tool)
{
    static const wchar_t *names[] = {
        L"이동", L"선택", L"타원", L"올가미", L"마법봉", L"자르기", L"스포이드",
        L"브러시", L"연필", L"지우개", L"도장", L"페인트통", L"그라데",
        L"닷지", L"번", L"스폰지", L"문자", L"사각형", L"원", L"선", L"손"
    };
    if (tool < 0 || tool >= TOOL_COUNT) return L"";
    return names[tool];
}

void tools_set_tool(int tool)
{
    if (tool < 0 || tool >= TOOL_COUNT) return;
    g.tool = tool;
    tools_cancel_crop();
    g.poly_n = 0;
    if (tool != TOOL_BRUSH) brush_popup_hide();
    ui_scroll_tool_into_view();
    app_refresh();
    if (tool == TOOL_BRUSH) brush_popup_show();
}

static void sample_at(float dx, float dy)
{
    int x = (int)floorf(dx);
    int y = (int)floorf(dy);
    uint8_t *p;
    comp_ensure();
    if (!g.disp || x < 0 || y < 0 || x >= g.doc.w || y >= g.doc.h) return;
    p = g.disp + ((size_t)y * (size_t)g.doc.w + (size_t)x) * 4;
    g.color = RGB(p[2], p[1], p[0]);
    InvalidateRect(g.toolbar, NULL, FALSE);
}

enum {
    HANDLE_NW = 0, HANDLE_N, HANDLE_NE,
    HANDLE_W, HANDLE_E,
    HANDLE_SW, HANDLE_S, HANDLE_SE
};

static int handle_hit_px(void)
{
    int s = ui_scale(16);
    return s < 14 ? 14 : s;
}

static void layer_view_rect(const Layer *L, int *x0, int *y0, int *x1, int *y1)
{
    doc_to_view((float)L->x, (float)L->y, x0, y0);
    doc_to_view((float)(L->x + L->w), (float)(L->y + L->h), x1, y1);
}

static void handle_points(int x0, int y0, int x1, int y1, POINT *p)
{
    int mx = (x0 + x1) / 2;
    int my = (y0 + y1) / 2;
    p[HANDLE_NW].x = x0; p[HANDLE_NW].y = y0;
    p[HANDLE_N].x  = mx; p[HANDLE_N].y  = y0;
    p[HANDLE_NE].x = x1; p[HANDLE_NE].y = y0;
    p[HANDLE_W].x  = x0; p[HANDLE_W].y  = my;
    p[HANDLE_E].x  = x1; p[HANDLE_E].y  = my;
    p[HANDLE_SW].x = x0; p[HANDLE_SW].y = y1;
    p[HANDLE_S].x  = mx; p[HANDLE_S].y  = y1;
    p[HANDLE_SE].x = x1; p[HANDLE_SE].y = y1;
}

static int hit_resize_handle(int sx, int sy)
{
    Layer *L = doc_active(&g.doc);
    POINT p[8];
    int x0, y0, x1, y1, i, hs, edge;
    if (g.tool != TOOL_MOVE || !L || !L->bgra) return -1;
    layer_view_rect(L, &x0, &y0, &x1, &y1);
    handle_points(x0, y0, x1, y1, p);
    hs = handle_hit_px();
    for (i = 0; i < 8; i++) {
        if (abs(sx - p[i].x) <= hs && abs(sy - p[i].y) <= hs) return i;
    }
    edge = ui_scale(8);
    if (edge < 6) edge = 6;
    if (sx >= x0 - edge && sx <= x1 + edge && abs(sy - y0) <= edge) {
        if (abs(sx - x0) <= hs) return HANDLE_NW;
        if (abs(sx - x1) <= hs) return HANDLE_NE;
        return HANDLE_N;
    }
    if (sx >= x0 - edge && sx <= x1 + edge && abs(sy - y1) <= edge) {
        if (abs(sx - x0) <= hs) return HANDLE_SW;
        if (abs(sx - x1) <= hs) return HANDLE_SE;
        return HANDLE_S;
    }
    if (sy >= y0 - edge && sy <= y1 + edge && abs(sx - x0) <= edge) {
        if (abs(sy - y0) <= hs) return HANDLE_NW;
        if (abs(sy - y1) <= hs) return HANDLE_SW;
        return HANDLE_W;
    }
    if (sy >= y0 - edge && sy <= y1 + edge && abs(sx - x1) <= edge) {
        if (abs(sy - y0) <= hs) return HANDLE_NE;
        if (abs(sy - y1) <= hs) return HANDLE_SE;
        return HANDLE_E;
    }
    return -1;
}

void tools_end_resize(void)
{
    free(g.resize_src);
    g.resize_src = NULL;
    g.resize_on = 0;
    g.resize_handle = -1;
    g.resize_sw = g.resize_sh = 0;
}

static int begin_resize(Layer *L, int handle)
{
    size_t n;
    if (!L || !L->bgra || L->w < 1 || L->h < 1) return 0;
    tools_end_resize();
    n = (size_t)L->w * (size_t)L->h * 4;
    g.resize_src = xmalloc(n);
    memcpy(g.resize_src, L->bgra, n);
    g.resize_sw = L->w;
    g.resize_sh = L->h;
    g.resize_ox = L->x;
    g.resize_oy = L->y;
    g.resize_ow = L->w;
    g.resize_oh = L->h;
    g.resize_handle = handle;
    g.resize_on = 1;
    return 1;
}

static void apply_handle_resize(Layer *L, float dx, float dy)
{
    int ox = g.resize_ox, oy = g.resize_oy, ow = g.resize_ow, oh = g.resize_oh;
    int mx = (int)floorf(dx + 0.5f);
    int my = (int)floorf(dy + 0.5f);
    int nw = ow, nh = oh, nx = ox, ny = oy;
    int lock = (GetKeyState(VK_SHIFT) & 0x8000) ? 0 : 1;
    int hdl = g.resize_handle;
    int is_corner = (hdl == HANDLE_NW || hdl == HANDLE_NE || hdl == HANDLE_SW || hdl == HANDLE_SE);

    if (!L || !g.resize_src) return;

    switch (hdl) {
    case HANDLE_NW: nw = (ox + ow) - mx; nh = (oy + oh) - my; break;
    case HANDLE_N:  nh = (oy + oh) - my; lock = 0; break;
    case HANDLE_NE: nw = mx - ox; nh = (oy + oh) - my; break;
    case HANDLE_W:  nw = (ox + ow) - mx; lock = 0; break;
    case HANDLE_E:  nw = mx - ox; lock = 0; break;
    case HANDLE_SW: nw = (ox + ow) - mx; nh = my - oy; break;
    case HANDLE_S:  nh = my - oy; lock = 0; break;
    case HANDLE_SE: nw = mx - ox; nh = my - oy; break;
    default: return;
    }

    if (nw < 1) nw = 1;
    if (nh < 1) nh = 1;
    if (nw > DAGRIM_MAX_SIZE) nw = DAGRIM_MAX_SIZE;
    if (nh > DAGRIM_MAX_SIZE) nh = DAGRIM_MAX_SIZE;

    if (lock && is_corner && ow > 0 && oh > 0) {
        double ar = (double)ow / (double)oh;
        double dw = fabs((double)nw / (double)ow - 1.0);
        double dh = fabs((double)nh / (double)oh - 1.0);
        if (dw >= dh) nh = (int)floor((double)nw / ar + 0.5);
        else nw = (int)floor((double)nh * ar + 0.5);
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        if (nw > DAGRIM_MAX_SIZE) {
            nw = DAGRIM_MAX_SIZE;
            nh = (int)floor((double)nw / ar + 0.5);
        }
        if (nh > DAGRIM_MAX_SIZE) {
            nh = DAGRIM_MAX_SIZE;
            nw = (int)floor((double)nh * ar + 0.5);
        }
        if (nw < 1) nw = 1;
        if (nh < 1) nh = 1;
        if (nw > DAGRIM_MAX_SIZE) nw = DAGRIM_MAX_SIZE;
        if (nh > DAGRIM_MAX_SIZE) nh = DAGRIM_MAX_SIZE;
    }

    switch (hdl) {
    case HANDLE_NW: nx = ox + ow - nw; ny = oy + oh - nh; break;
    case HANDLE_N:  nx = ox;           ny = oy + oh - nh; break;
    case HANDLE_NE: nx = ox;           ny = oy + oh - nh; break;
    case HANDLE_W:  nx = ox + ow - nw; ny = oy;           break;
    case HANDLE_E:  nx = ox;           ny = oy;           break;
    case HANDLE_SW: nx = ox + ow - nw; ny = oy;           break;
    case HANDLE_S:  nx = ox;           ny = oy;           break;
    case HANDLE_SE: nx = ox;           ny = oy;           break;
    }

    if (nw != L->w || nh != L->h)
        layer_scale_from(L, g.resize_src, g.resize_sw, g.resize_sh, nw, nh);
    L->x = nx;
    L->y = ny;
}

void tools_set_cursor(int sx, int sy)
{
    int h = g.resize_on ? g.resize_handle : hit_resize_handle(sx, sy);
    switch (h) {
    case HANDLE_NW:
    case HANDLE_SE:
        SetCursor(g.cur_nwse);
        break;
    case HANDLE_NE:
    case HANDLE_SW:
        SetCursor(g.cur_nesw);
        break;
    case HANDLE_N:
    case HANDLE_S:
        SetCursor(g.cur_ns);
        break;
    case HANDLE_W:
    case HANDLE_E:
        SetCursor(g.cur_we);
        break;
    default:
        SetCursor(g.cur_move);
        break;
    }
}

static void bucket_fill(float fx, float fy)
{
    Layer *L = doc_active(&g.doc);
    int w, h, sx, sy, n, head, tail, k, tol;
    int *qx, *qy;
    uint8_t *seen, seed[4], fill[4];
    static const int ox[4] = { 1, -1, 0, 0 };
    static const int oy[4] = { 0, 0, 1, -1 };
    if (!L || !L->bgra) return;
    sx = (int)floorf(fx) - L->x;
    sy = (int)floorf(fy) - L->y;
    if (sx < 0 || sy < 0 || sx >= L->w || sy >= L->h) return;
    w = L->w;
    h = L->h;
    n = w * h;
    memcpy(seed, L->bgra + ((size_t)sy * (size_t)w + (size_t)sx) * 4, 4);
    fill[0] = GetBValue(g.color);
    fill[1] = GetGValue(g.color);
    fill[2] = GetRValue(g.color);
    fill[3] = 255;
    tol = g.wand_tol > 0 ? g.wand_tol : 32;
    seen = xcalloc((size_t)n, 1);
    qx = (int *)xmalloc((size_t)n * sizeof(int));
    qy = (int *)xmalloc((size_t)n * sizeof(int));
    head = tail = 0;
    qx[tail] = sx;
    qy[tail] = sy;
    tail++;
    seen[sy * w + sx] = 1;
    while (head < tail) {
        int x = qx[head], y = qy[head];
        uint8_t *p = L->bgra + ((size_t)y * (size_t)w + (size_t)x) * 4;
        head++;
        if (abs(p[0] - seed[0]) > tol || abs(p[1] - seed[1]) > tol || abs(p[2] - seed[2]) > tol)
            continue;
        if (sel_mask(L->x + x, L->y + y) <= 0) continue;
        memcpy(p, fill, 4);
        for (k = 0; k < 4; k++) {
            int nx = x + ox[k], ny = y + oy[k];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
            if (seen[ny * w + nx]) continue;
            seen[ny * w + nx] = 1;
            qx[tail] = nx;
            qy[tail] = ny;
            tail++;
        }
    }
    free(seen);
    free(qx);
    free(qy);
    comp_invalidate_all();
}

static void apply_gradient(float x0, float y0, float x1, float y1)
{
    Layer *L = doc_active(&g.doc);
    int x, y;
    float dx, dy, len2;
    uint8_t b0, g0, r0, b1, g1, r1;
    if (!L || !L->bgra) return;
    dx = x1 - x0;
    dy = y1 - y0;
    len2 = dx * dx + dy * dy;
    if (len2 < 1.0f) len2 = 1.0f;
    b0 = GetBValue(g.color);
    g0 = GetGValue(g.color);
    r0 = GetRValue(g.color);
    b1 = GetBValue(g.color_bg);
    g1 = GetGValue(g.color_bg);
    r1 = GetRValue(g.color_bg);
    for (y = 0; y < L->h; y++) {
        for (x = 0; x < L->w; x++) {
            float px = (float)(L->x + x) + 0.5f;
            float py = (float)(L->y + y) + 0.5f;
            float t, m;
            uint8_t src[4], *p;
            m = (float)sel_mask(L->x + x, L->y + y);
            if (m <= 0) continue;
            t = ((px - x0) * dx + (py - y0) * dy) / len2;
            if (t < 0) t = 0;
            if (t > 1) t = 1;
            src[0] = (uint8_t)(b0 + ((int)b1 - (int)b0) * t);
            src[1] = (uint8_t)(g0 + ((int)g1 - (int)g0) * t);
            src[2] = (uint8_t)(r0 + ((int)r1 - (int)r0) * t);
            src[3] = (uint8_t)m;
            p = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            blend_pixel(p, src, 255, BLEND_NORMAL);
        }
    }
    comp_invalidate_all();
}

static void raster_shape(float x0, float y0, float x1, float y1, int kind)
{
    Layer *L = doc_active(&g.doc);
    if (!L || !L->bgra) return;
    if (!g.shape_fill && !g.shape_outline) g.shape_fill = 1;
    shape_raster_layer(L, x0, y0, x1, y1, kind);
}

static void place_text(float fx, float fy)
{
    Layer *L;
    HDC hdc, mem;
    HBITMAP bm, oldb;
    HFONT font, oldf;
    BITMAPINFO bmi;
    void *bits = NULL;
    SIZE sz;
    int w, h, x, y, px;
    uint8_t *pbits;
    if (DialogBoxW(g.inst, MAKEINTRESOURCEW(IDD_TEXT), g.hwnd, TextDlgProc) != IDOK) return;
    if (!g.text_buf[0]) return;
    L = doc_active(&g.doc);
    if (!L || !L->bgra) return;
    px = g.text_size > 0 ? g.text_size : 48;
    hdc = GetDC(NULL);
    mem = CreateCompatibleDC(hdc);
    font = CreateFontW(-px, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                       DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    oldf = SelectObject(mem, font);
    GetTextExtentPoint32W(mem, g.text_buf, (int)wcslen(g.text_buf), &sz);
    w = sz.cx + 8;
    h = sz.cy + 8;
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bm = CreateDIBSection(mem, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    oldb = SelectObject(mem, bm);
    memset(bits, 0, (size_t)w * (size_t)h * 4);
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(255, 255, 255));
    TextOutW(mem, 4, 4, g.text_buf, (int)wcslen(g.text_buf));
    pbits = (uint8_t *)bits;
    undo_push();
    for (y = 0; y < h; y++) {
        int ly = (int)floorf(fy) + y - L->y;
        if (ly < 0 || ly >= L->h) continue;
        for (x = 0; x < w; x++) {
            int lx = (int)floorf(fx) + x - L->x;
            uint8_t *s, src[4], *d, a;
            if (lx < 0 || lx >= L->w) continue;
            s = pbits + ((size_t)y * (size_t)w + (size_t)x) * 4;
            a = s[1];
            if (a < 8) continue;
            src[0] = GetBValue(g.color);
            src[1] = GetGValue(g.color);
            src[2] = GetRValue(g.color);
            src[3] = a;
            d = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
            blend_pixel(d, src, 255, BLEND_NORMAL);
        }
    }
    SelectObject(mem, oldb);
    SelectObject(mem, oldf);
    DeleteObject(bm);
    DeleteObject(font);
    DeleteDC(mem);
    ReleaseDC(NULL, hdc);
    app_set_dirty(1);
    comp_invalidate_all();
    app_refresh();
}

void tools_on_down(int sx, int sy, int button)
{
    float dx, dy;
    Layer *L;
    int handle;

    g.last_sx = sx;
    g.last_sy = sy;
    view_to_doc(sx, sy, &dx, &dy);

    if (button == 2 || g.space_down || g.tool == TOOL_HAND) {
        g.panning = 1;
        SetCapture(g.canvas);
        return;
    }
    if (button != 1) return;

    if (g.tool == TOOL_MOVE) {
        L = doc_active(&g.doc);
        if (!L) return;
        undo_push();
        handle = hit_resize_handle(sx, sy);
        if (handle >= 0 && begin_resize(L, handle)) {
            g.dragging = 1;
            SetCapture(g.canvas);
            tools_set_cursor(sx, sy);
            return;
        }
        g.move_lx = L->x;
        g.move_ly = L->y;
        g.move_dx = dx;
        g.move_dy = dy;
        g.dragging = 1;
        SetCapture(g.canvas);
        return;
    }
    if (g.tool == TOOL_EYEDROPPER) {
        sample_at(dx, dy);
        g.dragging = 1;
        SetCapture(g.canvas);
        return;
    }
    if (g.tool == TOOL_CROP) {
        g.crop_on = 1;
        g.crop_x0 = g.crop_x1 = dx;
        g.crop_y0 = g.crop_y1 = dy;
        g.dragging = 1;
        SetCapture(g.canvas);
        InvalidateRect(g.canvas, NULL, FALSE);
        return;
    }
    if (g.tool == TOOL_SEL_RECT || g.tool == TOOL_SEL_ELLIPSE) {
        g.crop_x0 = g.crop_x1 = dx;
        g.crop_y0 = g.crop_y1 = dy;
        g.dragging = 1;
        SetCapture(g.canvas);
        InvalidateRect(g.canvas, NULL, FALSE);
        return;
    }
    if (g.tool == TOOL_LASSO) {
        g.poly_n = 1;
        g.poly_x[0] = dx;
        g.poly_y[0] = dy;
        g.dragging = 1;
        SetCapture(g.canvas);
        return;
    }
    if (g.tool == TOOL_WAND) {
        sel_from_wand((int)floorf(dx), (int)floorf(dy));
        app_refresh();
        return;
    }
    if (g.tool == TOOL_CLONE && (GetKeyState(VK_MENU) & 0x8000)) {
        g.clone_sx = dx;
        g.clone_sy = dy;
        g.clone_ok = 1;
        ui_update_status();
        InvalidateRect(g.status, NULL, FALSE);
        return;
    }
    if (g.tool == TOOL_BUCKET) {
        if (!doc_active(&g.doc)) return;
        undo_push();
        bucket_fill(dx, dy);
        app_set_dirty(1);
        app_refresh();
        return;
    }
    if (g.tool == TOOL_GRADIENT || g.tool == TOOL_SHAPE_RECT ||
        g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE) {
        if (!doc_active(&g.doc) && g.tool != TOOL_GRADIENT) return;
        g.crop_x0 = g.crop_x1 = dx;
        g.crop_y0 = g.crop_y1 = dy;
        g.dragging = 1;
        SetCapture(g.canvas);
        return;
    }
    if (g.tool == TOOL_TEXT) {
        place_text(dx, dy);
        return;
    }
    if (is_paint_tool(g.tool)) {
        if (!doc_active(&g.doc)) return;
        if (g.tool == TOOL_CLONE && !g.clone_ok) return;
        undo_push();
        g.have_stroke = 0;
        g.dragging = 1;
        SetCapture(g.canvas);
        stroke_to(dx, dy, stamp_mode());
        app_set_dirty(1);
        InvalidateRect(g.canvas, NULL, FALSE);
    }
}

void tools_on_move(int sx, int sy)
{
    float dx, dy;
    Layer *L;
    view_to_doc(sx, sy, &dx, &dy);

    if (g.panning) {
        g.pan_x += (float)(sx - g.last_sx);
        g.pan_y += (float)(sy - g.last_sy);
        g.last_sx = sx;
        g.last_sy = sy;
        InvalidateRect(g.canvas, NULL, FALSE);
        return;
    }
    if (!g.dragging) {
        g.last_sx = sx;
        g.last_sy = sy;
        ui_update_status();
        InvalidateRect(g.status, NULL, FALSE);
        return;
    }

    if (g.tool == TOOL_MOVE) {
        L = doc_active(&g.doc);
        if (L) {
            if (g.resize_on) {
                apply_handle_resize(L, dx, dy);
            } else {
                L->x = g.move_lx + (int)floorf(dx - g.move_dx + 0.5f);
                L->y = g.move_ly + (int)floorf(dy - g.move_dy + 0.5f);
            }
            app_set_dirty(1);
            comp_invalidate_all();
            InvalidateRect(g.canvas, NULL, FALSE);
        }
    } else if (g.tool == TOOL_EYEDROPPER) {
        sample_at(dx, dy);
    } else if (g.tool == TOOL_CROP || g.tool == TOOL_SEL_RECT || g.tool == TOOL_SEL_ELLIPSE ||
               g.tool == TOOL_GRADIENT || g.tool == TOOL_SHAPE_RECT ||
               g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE) {
        g.crop_x1 = dx;
        g.crop_y1 = dy;
        InvalidateRect(g.canvas, NULL, FALSE);
    } else if (g.tool == TOOL_LASSO) {
        if (g.poly_n > 0 && g.poly_n < DAGRIM_MAX_POLY) {
            float px = g.poly_x[g.poly_n - 1], py = g.poly_y[g.poly_n - 1];
            if (sqrtf((dx - px) * (dx - px) + (dy - py) * (dy - py)) >= 1.5f) {
                g.poly_x[g.poly_n] = dx;
                g.poly_y[g.poly_n] = dy;
                g.poly_n++;
            }
        }
        InvalidateRect(g.canvas, NULL, FALSE);
    } else if (is_paint_tool(g.tool)) {
        stroke_to(dx, dy, stamp_mode());
        app_set_dirty(1);
        InvalidateRect(g.canvas, NULL, FALSE);
    }
    g.last_sx = sx;
    g.last_sy = sy;
    ui_update_status();
    InvalidateRect(g.status, NULL, FALSE);
}

void tools_on_up(int sx, int sy)
{
    Layer *L;
    (void)sx;
    (void)sy;
    if (g.resize_on) {
        L = doc_active(&g.doc);
        if (L) doc_ensure_canvas(&g.doc, L->x + L->w, L->y + L->h);
        tools_end_resize();
        app_refresh();
    }
    if (g.dragging && (g.tool == TOOL_SEL_RECT || g.tool == TOOL_SEL_ELLIPSE)) {
        sel_from_rect(g.crop_x0, g.crop_y0, g.crop_x1, g.crop_y1, g.tool == TOOL_SEL_ELLIPSE);
        app_refresh();
    }
    if (g.dragging && g.tool == TOOL_LASSO) {
        sel_from_poly();
        g.poly_n = 0;
        app_refresh();
    }
    if (g.dragging && g.tool == TOOL_GRADIENT) {
        undo_push();
        apply_gradient(g.crop_x0, g.crop_y0, g.crop_x1, g.crop_y1);
        app_set_dirty(1);
        app_refresh();
    }
    if (g.dragging && (g.tool == TOOL_SHAPE_RECT || g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE)) {
        float x0 = g.crop_x0, y0 = g.crop_y0, x1 = g.crop_x1, y1 = g.crop_y1;
        shape_norm_box(&x0, &y0, &x1, &y1);
        undo_push();
        raster_shape(x0, y0, x1, y1, g.shape_kind);
        app_set_dirty(1);
        app_refresh();
    }
    if (g.panning || g.dragging) ReleaseCapture();
    g.panning = 0;
    g.dragging = 0;
    g.have_stroke = 0;
    if (g.tool == TOOL_MOVE || is_paint_tool(g.tool)) {
        InvalidateRect(g.layers, NULL, FALSE);
    }
}

void tools_cancel_crop(void)
{
    if (!g.crop_on) return;
    g.crop_on = 0;
    InvalidateRect(g.canvas, NULL, FALSE);
}

void tools_apply_crop(void)
{
    int x0, y0, x1, y1, w, h;
    if (!g.crop_on) return;
    x0 = (int)floorf(g.crop_x0 < g.crop_x1 ? g.crop_x0 : g.crop_x1);
    y0 = (int)floorf(g.crop_y0 < g.crop_y1 ? g.crop_y0 : g.crop_y1);
    x1 = (int)ceilf(g.crop_x0 > g.crop_x1 ? g.crop_x0 : g.crop_x1);
    y1 = (int)ceilf(g.crop_y0 > g.crop_y1 ? g.crop_y0 : g.crop_y1);
    x0 = clampi(x0, 0, g.doc.w);
    y0 = clampi(y0, 0, g.doc.h);
    x1 = clampi(x1, 0, g.doc.w);
    y1 = clampi(y1, 0, g.doc.h);
    w = x1 - x0;
    h = y1 - y0;
    g.crop_on = 0;
    if (w < 1 || h < 1) {
        InvalidateRect(g.canvas, NULL, FALSE);
        return;
    }
    undo_push();
    if (adj_crop_doc(&g.doc, x0, y0, w, h)) {
        app_set_dirty(1);
        comp_invalidate_all();
        view_fit();
        app_refresh();
    }
}

void tools_draw_overlay(HDC hdc)
{
    int x0, y0, x1, y1, i, hs;
    HPEN pen, old;
    HBRUSH oldb, fill, frame;
    POINT p[8];
    Layer *L;

    if (g.crop_on || ((g.tool == TOOL_SEL_RECT || g.tool == TOOL_SEL_ELLIPSE ||
                       g.tool == TOOL_GRADIENT || g.tool == TOOL_SHAPE_RECT ||
                       g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE) && g.dragging)) {
        doc_to_view(g.crop_x0, g.crop_y0, &x0, &y0);
        doc_to_view(g.crop_x1, g.crop_y1, &x1, &y1);
        if (g.tool == TOOL_GRADIENT) {
            pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            old = SelectObject(hdc, pen);
            MoveToEx(hdc, x0, y0, NULL);
            LineTo(hdc, x1, y1);
            SelectObject(hdc, old);
            DeleteObject(pen);
        } else if (g.tool == TOOL_SHAPE_RECT || g.tool == TOOL_SHAPE_ELLIPSE || g.tool == TOOL_SHAPE_LINE) {
            float fx0 = g.crop_x0, fy0 = g.crop_y0, fx1 = g.crop_x1, fy1 = g.crop_y1;
            POINT sp[32];
            int n;
            shape_norm_box(&fx0, &fy0, &fx1, &fy1);
            n = shape_outline(g.shape_kind, fx0, fy0, fx1, fy1, sp, 32);
            pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            old = SelectObject(hdc, pen);
            oldb = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            if (n >= 2) {
                Polyline(hdc, sp, n);
                if (n > 2) {
                    MoveToEx(hdc, sp[n - 1].x, sp[n - 1].y, NULL);
                    LineTo(hdc, sp[0].x, sp[0].y);
                }
            } else {
                int vx0, vy0, vx1, vy1;
                doc_to_view(fx0, fy0, &vx0, &vy0);
                doc_to_view(fx1, fy1, &vx1, &vy1);
                if (vx0 > vx1) { int t = vx0; vx0 = vx1; vx1 = t; }
                if (vy0 > vy1) { int t = vy0; vy0 = vy1; vy1 = t; }
                if (g.shape_kind == SHAPE_ELLIPSE || g.shape_kind == SHAPE_CIRCLE ||
                    g.shape_kind == SHAPE_BUBBLE_ROUND)
                    Ellipse(hdc, vx0, vy0, vx1, vy1);
                else if (g.shape_kind == SHAPE_ROUNDRECT)
                    RoundRect(hdc, vx0, vy0, vx1, vy1, (vx1 - vx0) / 4, (vy1 - vy0) / 4);
                else
                    Rectangle(hdc, vx0, vy0, vx1, vy1);
            }
            SelectObject(hdc, oldb);
            SelectObject(hdc, old);
            DeleteObject(pen);
        } else {
            if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
            if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
            pen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
            old = SelectObject(hdc, pen);
            oldb = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, x0, y0, x1, y1);
            SelectObject(hdc, oldb);
            SelectObject(hdc, old);
            DeleteObject(pen);
        }
        return;
    }
    if (g.tool == TOOL_LASSO && g.poly_n > 1) {
        int i, vx, vy, px, py;
        pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        old = SelectObject(hdc, pen);
        doc_to_view(g.poly_x[0], g.poly_y[0], &px, &py);
        MoveToEx(hdc, px, py, NULL);
        for (i = 1; i < g.poly_n; i++) {
            doc_to_view(g.poly_x[i], g.poly_y[i], &vx, &vy);
            LineTo(hdc, vx, vy);
        }
        SelectObject(hdc, old);
        DeleteObject(pen);
        return;
    }

    sel_draw_overlay(hdc);

    if (g.tool != TOOL_MOVE) return;
    L = doc_active(&g.doc);
    if (!L || !L->bgra) return;

    layer_view_rect(L, &x0, &y0, &x1, &y1);
    pen = CreatePen(PS_SOLID, 1, RGB(30, 30, 36));
    old = SelectObject(hdc, pen);
    oldb = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    SelectObject(hdc, old);
    DeleteObject(pen);
    pen = CreatePen(PS_SOLID, 1, RGB(90, 170, 255));
    SelectObject(hdc, pen);
    Rectangle(hdc, x0, y0, x1, y1);
    SelectObject(hdc, oldb);
    SelectObject(hdc, old);
    DeleteObject(pen);

    handle_points(x0, y0, x1, y1, p);
    hs = ui_scale(6);
    if (hs < 6) hs = 6;
    fill = CreateSolidBrush(RGB(90, 170, 255));
    frame = GetStockObject(WHITE_BRUSH);
    for (i = 0; i < 8; i++) {
        RECT r = { p[i].x - hs, p[i].y - hs, p[i].x + hs + 1, p[i].y + hs + 1 };
        FillRect(hdc, &r, fill);
        FrameRect(hdc, &r, frame);
    }
    DeleteObject(fill);
}
