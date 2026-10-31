#include "app.h"

#define SHAPE_MAXPT 32

static const wchar_t *preset_names[BRUSH_PRESET_COUNT] = {
    L"브러시", L"붓글씨 브러시", L"붓글씨 펜", L"에어브러시",
    L"유화 브러시", L"크레용", L"마커", L"일반 연필", L"수채화 브러시"
};

const wchar_t *brush_preset_name(int preset)
{
    if (preset < 0 || preset >= BRUSH_PRESET_COUNT) return L"브러시";
    return preset_names[preset];
}

static unsigned uhash(unsigned x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static float frand(int x, int y, int s)
{
    unsigned u = (unsigned)x * 73856093u ^ (unsigned)y * 19349663u ^ (unsigned)s * 83492791u;
    return (uhash(u) & 0xFFFF) / 65535.0f;
}

static void unit_dir(float *ux, float *uy)
{
    float len = sqrtf(g.stroke_dx * g.stroke_dx + g.stroke_dy * g.stroke_dy);
    if (len < 0.001f) {
        *ux = 1.0f;
        *uy = 0.0f;
        return;
    }
    *ux = g.stroke_dx / len;
    *uy = g.stroke_dy / len;
}

int brush_active_preset(void)
{
    if (g.tool == TOOL_PENCIL) return BRUSH_PENCIL;
    if (g.tool == TOOL_BRUSH) return g.brush_preset;
    return BRUSH_ROUND;
}

float brush_spacing(void)
{
    float rad = (float)g.brush_size * 0.5f;
    int p = brush_active_preset();
    float s;
    if (rad < 0.5f) rad = 0.5f;
    switch (p) {
    case BRUSH_AIR: s = rad * 0.42f; break;
    case BRUSH_OIL: s = rad * 0.22f; break;
    case BRUSH_CRAYON: s = rad * 0.20f; break;
    case BRUSH_PENCIL: s = rad * 0.22f; break;
    case BRUSH_WATER: s = rad * 0.28f; break;
    case BRUSH_MARKER: s = rad * 0.14f; break;
    case BRUSH_CALLI:
    case BRUSH_CALLI_PEN: s = rad * 0.12f; break;
    default: s = rad * 0.16f; break;
    }
    if (s < 0.8f) s = 0.8f;
    return s;
}

static float tip_cover(int preset, float ddx, float ddy, float rad, float ux, float uy, int px, int py)
{
    float along, across, d, t, k, n, rx, ry, press;
    along = ddx * ux + ddy * uy;
    across = -ddx * uy + ddy * ux;
    d = sqrtf(ddx * ddx + ddy * ddy);
    press = clampf(g.stroke_len / (rad * 3.0f + 1.0f), 0.35f, 1.0f);
    n = frand(px, py, preset + 3);

    switch (preset) {
    case BRUSH_CALLI:
        rx = rad * (1.15f + 0.55f * press);
        ry = rad * 0.34f;
        t = (along / rx) * (along / rx) + (across / ry) * (across / ry);
        if (t > 1.0f) return 0;
        k = t < 0.55f ? 1.0f : 1.0f - (t - 0.55f) / 0.45f;
        return k * (0.88f + 0.12f * n);

    case BRUSH_CALLI_PEN:
        rx = rad * (0.85f + 0.55f * press);
        ry = rad * 0.18f;
        t = (along / rx) * (along / rx) + (across / ry) * (across / ry);
        if (t > 1.0f) return 0;
        k = t < 0.72f ? 1.0f : 1.0f - (t - 0.72f) / 0.28f;
        return k;

    case BRUSH_AIR: {
        float spread = rad * 1.45f, acc = 0;
        int i;
        if (d > spread) return 0;
        for (i = 0; i < 7; i++) {
            float ox = (frand(px, py, 20 + i) - 0.5f) * spread * 1.1f;
            float oy = (frand(px, py, 40 + i) - 0.5f) * spread * 1.1f;
            float pd = sqrtf((ddx - ox) * (ddx - ox) + (ddy - oy) * (ddy - oy));
            float pr = rad * 0.22f;
            if (pd < pr) acc += (1.0f - pd / pr) * 0.22f;
        }
        t = 1.0f - d / spread;
        if (t < 0) t = 0;
        if (n > 0.62f + 0.25f * t) return 0;
        return clampf(acc + t * t * 0.12f, 0, 0.55f);
    }

    case BRUSH_OIL: {
        float half = rad * 0.55f, slot;
        if (fabsf(along) > half || fabsf(across) > rad) return 0;
        slot = fmodf(fabsf(across) + rad, 2.6f);
        if (slot > 1.55f) return n * 0.12f;
        k = 1.0f - fabsf(along) / half * 0.25f;
        k *= 0.72f + 0.28f * n;
        return k;
    }

    case BRUSH_CRAYON:
        if (d > rad) return 0;
        t = d / rad;
        if (n < 0.22f + 0.48f * t * t) return 0;
        return 0.70f + 0.30f * n;

    case BRUSH_MARKER: {
        float half = rad * 0.15f, thick = rad * 0.92f, dist;
        if (fabsf(along) <= half) dist = fabsf(across);
        else {
            float ax = fabsf(along) - half;
            dist = sqrtf(ax * ax + across * across);
        }
        if (dist > thick) return 0;
        k = dist < thick * 0.82f ? 0.92f : 0.92f * (1.0f - (dist - thick * 0.82f) / (thick * 0.18f));
        return k;
    }

    case BRUSH_PENCIL: {
        float pr = rad * 0.42f;
        if (d > pr) return 0;
        t = d / pr;
        if (n < 0.18f + 0.55f * t) return 0;
        return (0.50f + 0.40f * n) * (1.0f - t * 0.35f);
    }

    case BRUSH_WATER: {
        float wr = rad * 1.35f;
        if (d > wr) return 0;
        t = d / wr;
        k = (1.0f - t) * (1.0f - t) * 0.34f;
        if (t > 0.52f && t < 0.92f)
            k += 0.10f * (1.0f - fabsf(t - 0.72f) / 0.20f);
        k *= 0.85f + 0.15f * n;
        return k;
    }

    default: {
        float inner = rad * 0.62f;
        if (d > rad) return 0;
        if (d <= inner) return 1.0f;
        return 1.0f - (d - inner) / (rad - inner + 0.0001f);
    }
    }
}

static void stamp_buf(uint8_t *bgra, int w, int h, int ox, int oy,
                      float fx, float fy, int preset, int opacity,
                      uint8_t sb, uint8_t sg, uint8_t sr, int sel_on)
{
    float rad, ux, uy;
    int x0, y0, x1, y1, x, y;
    rad = (float)g.brush_size * 0.5f;
    if (preset == BRUSH_PENCIL) rad *= 0.85f;
    if (preset == BRUSH_AIR || preset == BRUSH_WATER) rad *= 1.15f;
    if (rad < 0.5f) rad = 0.5f;
    unit_dir(&ux, &uy);
    x0 = (int)floorf(fx - rad * 1.6f - 1.0f);
    y0 = (int)floorf(fy - rad * 1.6f - 1.0f);
    x1 = (int)ceilf(fx + rad * 1.6f + 1.0f);
    y1 = (int)ceilf(fy + rad * 1.6f + 1.0f);
    for (y = y0; y < y1; y++) {
        int ly = y - oy;
        if (ly < 0 || ly >= h) continue;
        for (x = x0; x < x1; x++) {
            int lx = x - ox, m, pa;
            float k;
            uint8_t src[4], *p;
            if (lx < 0 || lx >= w) continue;
            m = sel_on ? sel_mask(x, y) : 255;
            if (m <= 0) continue;
            k = tip_cover(preset, (x + 0.5f) - fx, (y + 0.5f) - fy, rad, ux, uy, x, y);
            if (k <= 0.001f) continue;
            pa = (int)(opacity * k * m / 255.0f);
            if (pa <= 0) continue;
            p = bgra + ((size_t)ly * (size_t)w + (size_t)lx) * 4;
            src[0] = sb;
            src[1] = sg;
            src[2] = sr;
            src[3] = (uint8_t)clampi(pa, 0, 255);
            blend_pixel(p, src, 255, BLEND_NORMAL);
        }
    }
}

void brush_paint(float fx, float fy, int mode)
{
    Layer *L = doc_active(&g.doc);
    int preset, opacity;
    float rad;
    if (!L || !L->bgra) return;
    preset = (mode == 2) ? BRUSH_PENCIL : brush_active_preset();
    opacity = g.brush_opacity;
    if (preset == BRUSH_AIR) opacity = opacity * 90 / 255;
    else if (preset == BRUSH_WATER) opacity = opacity * 140 / 255;
    else if (preset == BRUSH_MARKER) opacity = opacity * 210 / 255;
    else if (preset == BRUSH_PENCIL) opacity = opacity * 190 / 255;
    stamp_buf(L->bgra, L->w, L->h, L->x, L->y, fx, fy, preset, opacity,
              GetBValue(g.color), GetGValue(g.color), GetRValue(g.color), 1);
    rad = (float)g.brush_size * 0.8f + 4.0f;
    comp_invalidate_rect((int)floorf(fx - rad), (int)floorf(fy - rad),
                         (int)ceilf(rad * 2), (int)ceilf(rad * 2));
}

void brush_render_preview(uint8_t *bgra, int w, int h, int preset)
{
    int i, n, saved_size, saved_op, tool;
    float saved_dx, saved_dy, saved_len, prevx, prevy;
    n = w * 2;
    if (n < 40) n = 40;
    memset(bgra, 0, (size_t)w * (size_t)h * 4);
    for (i = 0; i < w * h; i++) {
        bgra[i * 4 + 0] = 8;
        bgra[i * 4 + 1] = 8;
        bgra[i * 4 + 2] = 10;
        bgra[i * 4 + 3] = 255;
    }
    saved_size = g.brush_size;
    saved_op = g.brush_opacity;
    saved_dx = g.stroke_dx;
    saved_dy = g.stroke_dy;
    saved_len = g.stroke_len;
    tool = g.tool;
    g.tool = TOOL_BRUSH;
    g.brush_size = clampi(h * 2 / 3, 8, 28);
    g.brush_opacity = 230;
    g.stroke_len = 0;
    prevx = 8;
    prevy = h * 0.55f;
    g.stroke_dx = 1;
    g.stroke_dy = 0;
    for (i = 0; i <= n; i++) {
        float t = (float)i / (float)n;
        float x = 8.0f + t * (w - 16.0f);
        float y = h * 0.55f - sinf(t * 3.14159265f) * (h * 0.30f);
        float dx = x - prevx, dy = y - prevy;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > 0.001f) {
            g.stroke_dx = dx;
            g.stroke_dy = dy;
        }
        stamp_buf(bgra, w, h, 0, 0, x, y, preset, g.brush_opacity, 245, 245, 248, 0);
        g.stroke_len += dist;
        prevx = x;
        prevy = y;
    }
    g.brush_size = saved_size;
    g.brush_opacity = saved_op;
    g.stroke_dx = saved_dx;
    g.stroke_dy = saved_dy;
    g.stroke_len = saved_len;
    g.tool = tool;
}

static int pip(float px, float py, const float *xs, const float *ys, int n)
{
    int i, c = 0;
    for (i = 0; i < n; i++) {
        float x0 = xs[i], y0 = ys[i], x1 = xs[(i + 1) % n], y1 = ys[(i + 1) % n];
        if (((y0 > py) != (y1 > py)) &&
            (px < (x1 - x0) * (py - y0) / ((y1 - y0) == 0 ? 1e-6f : (y1 - y0)) + x0))
            c = !c;
    }
    return c;
}

static float dist_seg(float px, float py, float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0, dy = y1 - y0, l2 = dx * dx + dy * dy, t;
    if (l2 < 1e-8f) return sqrtf((px - x0) * (px - x0) + (py - y0) * (py - y0));
    t = ((px - x0) * dx + (py - y0) * dy) / l2;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    dx = px - (x0 + t * dx);
    dy = py - (y0 + t * dy);
    return sqrtf(dx * dx + dy * dy);
}

void shape_norm_box(float *x0, float *y0, float *x1, float *y1)
{
    int k = g.shape_kind;
    if (k == SHAPE_CIRCLE || k == SHAPE_SQUARE || k == SHAPE_ROUNDRECT) {
        float w = fabsf(*x1 - *x0), h = fabsf(*y1 - *y0), s;
        s = w < h ? w : h;
        *x1 = *x0 + (*x1 >= *x0 ? s : -s);
        *y1 = *y0 + (*y1 >= *y0 ? s : -s);
    }
}

static int shape_poly(int kind, float x0, float y0, float x1, float y1, float *xs, float *ys)
{
    float xa = x0 < x1 ? x0 : x1, xb = x0 > x1 ? x0 : x1;
    float ya = y0 < y1 ? y0 : y1, yb = y0 > y1 ? y0 : y1;
    float cx = (xa + xb) * 0.5f, cy = (ya + yb) * 0.5f;
    float rx = (xb - xa) * 0.5f, ry = (yb - ya) * 0.5f;
    int n = 0, i;
    if (rx < 0.5f) rx = 0.5f;
    if (ry < 0.5f) ry = 0.5f;

    switch (kind) {
    case SHAPE_RECT:
    case SHAPE_SQUARE:
        xs[0] = xa; ys[0] = ya; xs[1] = xb; ys[1] = ya;
        xs[2] = xb; ys[2] = yb; xs[3] = xa; ys[3] = yb;
        return 4;
    case SHAPE_PARA:
        xs[0] = xa; ys[0] = yb;
        xs[1] = xa + rx * 0.55f; ys[1] = ya;
        xs[2] = xb; ys[2] = ya;
        xs[3] = xb - rx * 0.55f; ys[3] = yb;
        return 4;
    case SHAPE_TRI:
        xs[0] = cx; ys[0] = ya; xs[1] = xb; ys[1] = yb; xs[2] = xa; ys[2] = yb;
        return 3;
    case SHAPE_RTRI:
        xs[0] = xa; ys[0] = ya; xs[1] = xa; ys[1] = yb; xs[2] = xb; ys[2] = yb;
        return 3;
    case SHAPE_DIAMOND:
        xs[0] = cx; ys[0] = ya; xs[1] = xb; ys[1] = cy;
        xs[2] = cx; ys[2] = yb; xs[3] = xa; ys[3] = cy;
        return 4;
    case SHAPE_PENTA:
    case SHAPE_HEX: {
        int sides = kind == SHAPE_PENTA ? 5 : 6;
        for (i = 0; i < sides; i++) {
            float a = -3.14159265f / 2 + i * 6.2831853f / sides;
            xs[i] = cx + cosf(a) * rx;
            ys[i] = cy + sinf(a) * ry;
        }
        return sides;
    }
    case SHAPE_ARROW_R:
    case SHAPE_ARROW_L:
    case SHAPE_ARROW_U:
    case SHAPE_ARROW_D: {
        float pts[7][2] = {
            { xa, cy - ry * 0.32f }, { xa + rx * 1.05f, cy - ry * 0.32f },
            { xa + rx * 1.05f, ya }, { xb, cy },
            { xa + rx * 1.05f, yb }, { xa + rx * 1.05f, cy + ry * 0.32f },
            { xa, cy + ry * 0.32f }
        };
        for (i = 0; i < 7; i++) {
            float px = pts[i][0], py = pts[i][1], qx = px, qy = py;
            if (kind == SHAPE_ARROW_L) { qx = xa + xb - px; qy = py; }
            else if (kind == SHAPE_ARROW_U) { qx = cx + (py - cy) * rx / (ry + 0.001f); qy = cy - (px - cx) * ry / (rx + 0.001f); }
            else if (kind == SHAPE_ARROW_D) { qx = cx - (py - cy) * rx / (ry + 0.001f); qy = cy + (px - cx) * ry / (rx + 0.001f); }
            xs[i] = qx;
            ys[i] = qy;
        }
        return 7;
    }
    case SHAPE_STAR4:
    case SHAPE_STAR5:
    case SHAPE_STAR6: {
        int pts = kind == SHAPE_STAR4 ? 8 : (kind == SHAPE_STAR5 ? 10 : 12);
        float inner = kind == SHAPE_STAR6 ? 0.45f : 0.40f;
        for (i = 0; i < pts; i++) {
            float a = -3.14159265f / 2 + i * 3.14159265f / (pts / 2);
            float rr = (i & 1) ? inner : 1.0f;
            xs[i] = cx + cosf(a) * rx * rr;
            ys[i] = cy + sinf(a) * ry * rr;
        }
        return pts;
    }
    case SHAPE_BUBBLE:
        xs[0] = xa + rx * 0.25f; ys[0] = ya;
        xs[1] = xb - rx * 0.25f; ys[1] = ya;
        xs[2] = xb; ys[2] = ya + ry * 0.35f;
        xs[3] = xb; ys[3] = yb - ry * 0.45f;
        xs[4] = xb - rx * 0.25f; ys[4] = yb - ry * 0.18f;
        xs[5] = xa + rx * 0.42f; ys[5] = yb - ry * 0.18f;
        xs[6] = xa + rx * 0.12f; ys[6] = yb;
        xs[7] = xa + rx * 0.28f; ys[7] = yb - ry * 0.18f;
        xs[8] = xa + rx * 0.25f; ys[8] = yb - ry * 0.18f;
        xs[9] = xa; ys[9] = yb - ry * 0.45f;
        xs[10] = xa; ys[10] = ya + ry * 0.35f;
        return 11;
    default:
        break;
    }
    (void)n;
    return 0;
}

int shape_contains(int kind, float px, float py, float x0, float y0, float x1, float y1)
{
    float xa, ya, xb, yb, cx, cy, rx, ry, nx, ny, xs[SHAPE_MAXPT], ys[SHAPE_MAXPT];
    int n, i;
    float thick = (float)g.brush_size * 0.5f;
    if (thick < 1.0f) thick = 1.0f;
    xa = x0 < x1 ? x0 : x1;
    xb = x0 > x1 ? x0 : x1;
    ya = y0 < y1 ? y0 : y1;
    yb = y0 > y1 ? y0 : y1;
    cx = (xa + xb) * 0.5f;
    cy = (ya + yb) * 0.5f;
    rx = (xb - xa) * 0.5f;
    ry = (yb - ya) * 0.5f;
    if (rx < 0.5f) rx = 0.5f;
    if (ry < 0.5f) ry = 0.5f;

    if (kind == SHAPE_LINE) {
        return dist_seg(px, py, x0, y0, x1, y1) <= thick;
    }
    if (kind == SHAPE_CURVE) {
        float cpx = (x0 + x1) * 0.5f - (y1 - y0) * 0.28f;
        float cpy = (y0 + y1) * 0.5f + (x1 - x0) * 0.28f;
        float pax = x0, pay = y0;
        for (i = 1; i <= 16; i++) {
            float t = i / 16.0f, u = 1.0f - t;
            float bx = u * u * x0 + 2 * u * t * cpx + t * t * x1;
            float by = u * u * y0 + 2 * u * t * cpy + t * t * y1;
            if (dist_seg(px, py, pax, pay, bx, by) <= thick) return 1;
            pax = bx;
            pay = by;
        }
        return 0;
    }
    if (kind == SHAPE_ELLIPSE || kind == SHAPE_CIRCLE) {
        nx = (px - cx) / rx;
        ny = (py - cy) / ry;
        return nx * nx + ny * ny <= 1.0f;
    }
    if (kind == SHAPE_ROUNDRECT) {
        float rr = (rx < ry ? rx : ry) * 0.35f;
        float ix0 = xa + rr, iy0 = ya + rr, ix1 = xb - rr, iy1 = yb - rr;
        if (px >= ix0 && px <= ix1 && py >= ya && py <= yb) return 1;
        if (py >= iy0 && py <= iy1 && px >= xa && px <= xb) return 1;
        if (px < ix0 && py < iy0) { nx = (px - ix0) / rr; ny = (py - iy0) / rr; return nx * nx + ny * ny <= 1; }
        if (px > ix1 && py < iy0) { nx = (px - ix1) / rr; ny = (py - iy0) / rr; return nx * nx + ny * ny <= 1; }
        if (px < ix0 && py > iy1) { nx = (px - ix0) / rr; ny = (py - iy1) / rr; return nx * nx + ny * ny <= 1; }
        if (px > ix1 && py > iy1) { nx = (px - ix1) / rr; ny = (py - iy1) / rr; return nx * nx + ny * ny <= 1; }
        return 0;
    }
    if (kind == SHAPE_BUBBLE_ROUND) {
        nx = (px - cx) / rx;
        ny = (py - (cy - ry * 0.08f)) / (ry * 0.88f);
        if (nx * nx + ny * ny <= 1.0f) return 1;
        {
            float tx0 = cx - rx * 0.15f, ty0 = cy + ry * 0.55f;
            float tx1 = cx + rx * 0.05f, ty1 = yb;
            float tx2 = cx + rx * 0.35f, ty2 = cy + ry * 0.50f;
            float txs[3] = { tx0, tx1, tx2 }, tys[3] = { ty0, ty1, ty2 };
            return pip(px, py, txs, tys, 3);
        }
    }
    if (kind == SHAPE_CLOUD) {
        float c[5][4] = {
            { cx - rx * 0.35f, cy + ry * 0.05f, rx * 0.48f, ry * 0.48f },
            { cx + rx * 0.30f, cy + ry * 0.08f, rx * 0.50f, ry * 0.46f },
            { cx, cy - ry * 0.28f, rx * 0.42f, ry * 0.42f },
            { cx - rx * 0.55f, cy - ry * 0.05f, rx * 0.32f, ry * 0.34f },
            { cx + rx * 0.52f, cy - ry * 0.02f, rx * 0.30f, ry * 0.32f }
        };
        for (i = 0; i < 5; i++) {
            nx = (px - c[i][0]) / c[i][2];
            ny = (py - c[i][1]) / c[i][3];
            if (nx * nx + ny * ny <= 1.0f) return 1;
        }
        return 0;
    }
    n = shape_poly(kind, x0, y0, x1, y1, xs, ys);
    if (n < 3) return 0;
    return pip(px, py, xs, ys, n);
}

int shape_outline(int kind, float x0, float y0, float x1, float y1, POINT *out, int maxpt)
{
    float xs[SHAPE_MAXPT], ys[SHAPE_MAXPT];
    int n, i, vx, vy;
    if (kind == SHAPE_CURVE) {
        float cpx = (x0 + x1) * 0.5f - (y1 - y0) * 0.28f;
        float cpy = (y0 + y1) * 0.5f + (x1 - x0) * 0.28f;
        n = maxpt < 17 ? maxpt : 17;
        for (i = 0; i < n; i++) {
            float t = i / (n - 1.0f), u = 1.0f - t;
            float bx = u * u * x0 + 2 * u * t * cpx + t * t * x1;
            float by = u * u * y0 + 2 * u * t * cpy + t * t * y1;
            doc_to_view(bx, by, &vx, &vy);
            out[i].x = vx;
            out[i].y = vy;
        }
        return n;
    }
    if (kind == SHAPE_LINE) {
        doc_to_view(x0, y0, &vx, &vy);
        out[0].x = vx; out[0].y = vy;
        doc_to_view(x1, y1, &vx, &vy);
        out[1].x = vx; out[1].y = vy;
        return 2;
    }
    if (kind == SHAPE_ELLIPSE || kind == SHAPE_CIRCLE || kind == SHAPE_ROUNDRECT ||
        kind == SHAPE_CLOUD || kind == SHAPE_BUBBLE_ROUND) {
        return 0;
    }
    n = shape_poly(kind, x0, y0, x1, y1, xs, ys);
    if (n > maxpt) n = maxpt;
    for (i = 0; i < n; i++) {
        doc_to_view(xs[i], ys[i], &vx, &vy);
        out[i].x = vx;
        out[i].y = vy;
    }
    return n;
}

void tools_set_shape(int kind)
{
    if (kind < 0 || kind >= SHAPE_COUNT) return;
    g.shape_kind = kind;
    if (kind == SHAPE_LINE || kind == SHAPE_CURVE) tools_set_tool(TOOL_SHAPE_LINE);
    else if (kind == SHAPE_ELLIPSE || kind == SHAPE_CIRCLE) tools_set_tool(TOOL_SHAPE_ELLIPSE);
    else tools_set_tool(TOOL_SHAPE_RECT);
}

static int shape_interior_pt(int kind, float px, float py, float x0, float y0, float x1, float y1)
{
    if (kind == SHAPE_LINE || kind == SHAPE_CURVE) return 0;
    return shape_contains(kind, px, py, x0, y0, x1, y1);
}

static int shape_stroke_pt(int kind, float px, float py, float x0, float y0, float x1, float y1)
{
    float xa, ya, xb, yb, cx, cy, rx, ry, nx, ny, hw, xs[SHAPE_MAXPT], ys[SHAPE_MAXPT];
    int n, i, saved;
    if (!g.shape_outline) return 0;
    hw = (float)g.stroke_width * 0.5f;
    if (hw < 0.5f) hw = 0.5f;
    if (kind == SHAPE_LINE || kind == SHAPE_CURVE) {
        saved = g.brush_size;
        g.brush_size = g.stroke_width;
        n = shape_contains(kind, px, py, x0, y0, x1, y1);
        g.brush_size = saved;
        return n;
    }
    xa = x0 < x1 ? x0 : x1;
    xb = x0 > x1 ? x0 : x1;
    ya = y0 < y1 ? y0 : y1;
    yb = y0 > y1 ? y0 : y1;
    cx = (xa + xb) * 0.5f;
    cy = (ya + yb) * 0.5f;
    rx = (xb - xa) * 0.5f;
    ry = (yb - ya) * 0.5f;
    if (rx < 0.5f) rx = 0.5f;
    if (ry < 0.5f) ry = 0.5f;
    if (kind == SHAPE_ELLIPSE || kind == SHAPE_CIRCLE) {
        float d, rmin = rx < ry ? rx : ry, inner;
        nx = (px - cx) / rx;
        ny = (py - cy) / ry;
        d = nx * nx + ny * ny;
        if (d > 1.0f) return 0;
        inner = 1.0f - hw / rmin;
        if (inner < 0.0f) inner = 0.0f;
        return d >= inner * inner;
    }
    if (!shape_contains(kind, px, py, x0, y0, x1, y1)) return 0;
    if (!g.shape_fill) {
        n = shape_poly(kind, x0, y0, x1, y1, xs, ys);
        if (n < 2) return 0;
        for (i = 0; i < n; i++) {
            int j = (i + 1) % n;
            if (dist_seg(px, py, xs[i], ys[i], xs[j], ys[j]) <= hw) return 1;
        }
        return 0;
    }
    n = shape_poly(kind, x0, y0, x1, y1, xs, ys);
    if (n < 2) return 0;
    for (i = 0; i < n; i++) {
        int j = (i + 1) % n;
        if (dist_seg(px, py, xs[i], ys[i], xs[j], ys[j]) <= hw) return 1;
    }
    if (kind == SHAPE_ROUNDRECT) {
        float rr = (rx < ry ? rx : ry) * 0.35f;
        float ix0 = xa + rr, iy0 = ya + rr, ix1 = xb - rr, iy1 = yb - rr;
        if (px < ix0 && py < iy0) {
            nx = (px - ix0) / rr;
            ny = (py - iy0) / rr;
            return nx * nx + ny * ny >= (1.0f - hw / rr) * (1.0f - hw / rr);
        }
        if (px > ix1 && py < iy0) {
            nx = (px - ix1) / rr;
            ny = (py - iy0) / rr;
            return nx * nx + ny * ny >= (1.0f - hw / rr) * (1.0f - hw / rr);
        }
        if (px < ix0 && py > iy1) {
            nx = (px - ix0) / rr;
            ny = (py - iy1) / rr;
            return nx * nx + ny * ny >= (1.0f - hw / rr) * (1.0f - hw / rr);
        }
        if (px > ix1 && py > iy1) {
            nx = (px - ix1) / rr;
            ny = (py - iy1) / rr;
            return nx * nx + ny * ny >= (1.0f - hw / rr) * (1.0f - hw / rr);
        }
    }
    return 0;
}

static void stamp_preset(Layer *L, int x, int y, int preset, uint8_t b, uint8_t gv, uint8_t r, int m)
{
    int saved_p, saved_t, saved_w;
    COLORREF saved_c;
    int lx = x - L->x, ly = y - L->y;
    if (lx < 0 || ly < 0 || lx >= L->w || ly >= L->h) return;
    (void)m;
    saved_p = g.brush_preset;
    saved_t = g.tool;
    saved_w = g.brush_size;
    saved_c = g.color;
    g.color = RGB(r, gv, b);
    g.brush_preset = preset;
    g.tool = TOOL_BRUSH;
    g.brush_size = g.stroke_width;
    brush_paint((float)x + 0.5f, (float)y + 0.5f, 0);
    g.brush_preset = saved_p;
    g.tool = saved_t;
    g.brush_size = saved_w;
    g.color = saved_c;
}

void shape_raster_layer(Layer *L, float x0, float y0, float x1, float y1, int kind)
{
    int xa, ya, xb, yb, x, y, pad;
    uint8_t fb, fg, fr, bb, bg, br;
    if (!L || !L->bgra) return;
    fb = GetBValue(g.color);
    fg = GetGValue(g.color);
    fr = GetRValue(g.color);
    bb = GetBValue(g.color_bg);
    bg = GetGValue(g.color_bg);
    br = GetRValue(g.color_bg);
    xa = (int)floorf(x0 < x1 ? x0 : x1);
    ya = (int)floorf(y0 < y1 ? y0 : y1);
    xb = (int)ceilf(x0 > x1 ? x0 : x1);
    yb = (int)ceilf(y0 > y1 ? y0 : y1);
    pad = g.stroke_width + 4;
    xa -= pad;
    ya -= pad;
    xb += pad;
    yb += pad;
    for (y = ya; y <= yb; y++) {
        int ly = y - L->y;
        if (ly < 0 || ly >= L->h) continue;
        for (x = xa; x <= xb; x++) {
            int lx = x - L->x, m;
            uint8_t src[4], *p;
            float px = x + 0.5f, py = y + 0.5f;
            if (lx < 0 || lx >= L->w) continue;
            m = sel_mask(x, y);
            if (m <= 0) continue;
            if (g.shape_fill && shape_interior_pt(kind, px, py, x0, y0, x1, y1)) {
                src[0] = bb;
                src[1] = bg;
                src[2] = br;
                src[3] = (uint8_t)m;
                p = L->bgra + ((size_t)ly * (size_t)L->w + (size_t)lx) * 4;
                blend_pixel(p, src, 255, BLEND_NORMAL);
            }
            if (shape_stroke_pt(kind, px, py, x0, y0, x1, y1)) {
                stamp_preset(L, x, y, g.shape_stroke_preset, fb, fg, fr, m);
            }
        }
    }
    comp_invalidate_all();
}

static int popup_row(void)
{
    int s = ui_scale(42);
    return s < 36 ? 36 : s;
}

static void popup_metrics(int *pw, int *ph, int *grid_h, int *row_h)
{
    int pad = ui_scale(8);
    *row_h = popup_row();
    *grid_h = pad;
    *pw = ui_scale(310);
    *ph = pad + BRUSH_PRESET_COUNT * *row_h + pad;
}

static int popup_hit(int x, int y)
{
    int pad = ui_scale(8), row = popup_row(), rw;
    (void)x;
    if (y < pad) return -1;
    rw = (y - pad) / row;
    if (rw >= 0 && rw < BRUSH_PRESET_COUNT) return 100 + rw;
    return -1;
}

static void blit_preview(HDC hdc, RECT rc, int preset)
{
    BITMAPINFO bmi;
    void *bits = NULL;
    HDC mem;
    HBITMAP bm, old;
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    if (w < 8 || h < 8) return;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    mem = CreateCompatibleDC(hdc);
    bm = CreateDIBSection(mem, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    old = SelectObject(mem, bm);
    if (bits) brush_render_preview((uint8_t *)bits, w, h, preset);
    BitBlt(hdc, rc.left, rc.top, w, h, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bm);
    DeleteDC(mem);
}

LRESULT CALLBACK BrushPopupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc, mem;
        HBITMAP bm, old;
        HGDIOBJ of;
        RECT rc;
        int pad, row, i, grid_h, pw, ph;
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        mem = CreateCompatibleDC(hdc);
        bm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        old = SelectObject(mem, bm);
        FillRect(mem, &rc, g.br_panel);
        pad = ui_scale(8);
        popup_metrics(&pw, &ph, &grid_h, &row);
        of = SelectObject(mem, g.font_small);
        SetBkMode(mem, TRANSPARENT);
        for (i = 0; i < BRUSH_PRESET_COUNT; i++) {
            RECT rowr, name, prev, bar;
            int y = grid_h + i * row;
            int on = (g.tool == TOOL_BRUSH || g.tool == TOOL_PENCIL) && brush_active_preset() == i;
            rowr.left = pad; rowr.top = y; rowr.right = rc.right - pad; rowr.bottom = y + row - 2;
            FillRect(mem, &rowr, on ? g.br_sel : g.br_btn);
            if (on) {
                HBRUSH accent = CreateSolidBrush(RGB(80, 160, 255));
                bar = rowr;
                bar.right = bar.left + 3;
                FillRect(mem, &bar, accent);
                DeleteObject(accent);
            }
            name = rowr;
            name.left += ui_scale(10);
            name.right = name.left + ui_scale(118);
            SetTextColor(mem, RGB(235, 235, 240));
            DrawTextW(mem, preset_names[i], -1, &name, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            prev = rowr;
            prev.left = name.right + 4;
            prev.right -= 4;
            prev.top += 4;
            prev.bottom -= 4;
            blit_preview(mem, prev, i);
        }
        SelectObject(mem, of);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bm);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_LBUTTONDOWN: {
        int hit = popup_hit(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (hit >= 100 && hit < 100 + BRUSH_PRESET_COUNT) {
            g.brush_preset = hit - 100;
            g.shape_stroke_preset = g.brush_preset;
            if (g.tool != TOOL_BRUSH && g.tool != TOOL_SHAPE_RECT &&
                g.tool != TOOL_SHAPE_ELLIPSE && g.tool != TOOL_SHAPE_LINE)
                tools_set_tool(TOOL_BRUSH);
            brush_popup_hide();
            if (g.toolbar) InvalidateRect(g.toolbar, NULL, FALSE);
            ui_update_status();
            if (g.status) InvalidateRect(g.status, NULL, FALSE);
            SetFocus(g.canvas);
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int brush_popup_visible(void)
{
    return g.brush_popup && IsWindowVisible(g.brush_popup);
}

void brush_popup_hide(void)
{
    if (g.brush_popup) ShowWindow(g.brush_popup, SW_HIDE);
}

void brush_popup_layout(void)
{
    int pw, ph, grid_h, row, x = 0, y = 0;
    if (!g.brush_popup || !g.hwnd || !IsWindowVisible(g.brush_popup)) return;
    popup_metrics(&pw, &ph, &grid_h, &row);
    if (g.toolbar) {
        RECT r;
        POINT p;
        GetWindowRect(g.toolbar, &r);
        p.x = r.right;
        p.y = r.top;
        ScreenToClient(g.hwnd, &p);
        x = p.x;
        y = p.y;
    }
    SetWindowPos(g.brush_popup, HWND_TOP, x, y, pw, ph, SWP_NOACTIVATE);
}

void brush_popup_show(void)
{
    int pw, ph, grid_h, row;
    if (!g.hwnd) return;
    popup_metrics(&pw, &ph, &grid_h, &row);
    if (!g.brush_popup) {
        g.brush_popup = CreateWindowExW(WS_EX_NOACTIVATE, L"DaGrimBrushPopup", NULL,
            WS_CHILD | WS_CLIPSIBLINGS, 0, 0, pw, ph, g.hwnd, NULL, g.inst, NULL);
        if (!g.brush_popup) return;
    }
    ShowWindow(g.brush_popup, SW_SHOWNOACTIVATE);
    brush_popup_layout();
    InvalidateRect(g.brush_popup, NULL, FALSE);
    UpdateWindow(g.brush_popup);
}

void brush_popup_toggle(void)
{
    if (brush_popup_visible()) brush_popup_hide();
    else brush_popup_show();
}
