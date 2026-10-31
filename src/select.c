#include "app.h"

void sel_reset(void)
{
    free(g.sel);
    g.sel = NULL;
    g.sel_w = g.sel_h = 0;
    g.have_sel = 0;
    g.poly_n = 0;
}

void sel_ensure(void)
{
    size_t n;
    if (g.doc.w < 1 || g.doc.h < 1) {
        sel_reset();
        return;
    }
    if (g.sel && g.sel_w == g.doc.w && g.sel_h == g.doc.h) return;
    n = (size_t)g.doc.w * (size_t)g.doc.h;
    free(g.sel);
    g.sel = xcalloc(n, 1);
    g.sel_w = g.doc.w;
    g.sel_h = g.doc.h;
    g.have_sel = 0;
}

int sel_mask(int docx, int docy)
{
    if (!g.have_sel || !g.sel) return 255;
    if (docx < 0 || docy < 0 || docx >= g.sel_w || docy >= g.sel_h) return 0;
    return g.sel[(size_t)docy * (size_t)g.sel_w + (size_t)docx];
}

void sel_all(void)
{
    sel_ensure();
    if (!g.sel) return;
    memset(g.sel, 255, (size_t)g.sel_w * (size_t)g.sel_h);
    g.have_sel = 1;
}

void sel_deselect(void)
{
    if (g.sel && g.sel_w > 0 && g.sel_h > 0)
        memset(g.sel, 0, (size_t)g.sel_w * (size_t)g.sel_h);
    g.have_sel = 0;
    g.poly_n = 0;
}

void sel_invert(void)
{
    size_t i, n;
    sel_ensure();
    if (!g.sel) return;
    n = (size_t)g.sel_w * (size_t)g.sel_h;
    if (!g.have_sel) {
        sel_all();
        return;
    }
    for (i = 0; i < n; i++) g.sel[i] = (uint8_t)(255 - g.sel[i]);
}

static void sel_norm_rect(float a0, float b0, float a1, float b1, int *x0, int *y0, int *x1, int *y1)
{
    int xa = (int)floorf(a0 < a1 ? a0 : a1);
    int ya = (int)floorf(b0 < b1 ? b0 : b1);
    int xb = (int)ceilf(a0 > a1 ? a0 : a1);
    int yb = (int)ceilf(b0 > b1 ? b0 : b1);
    *x0 = clampi(xa, 0, g.doc.w);
    *y0 = clampi(ya, 0, g.doc.h);
    *x1 = clampi(xb, 0, g.doc.w);
    *y1 = clampi(yb, 0, g.doc.h);
}

void sel_from_rect(float x0, float y0, float x1, float y1, int ellipse)
{
    int xa, ya, xb, yb, x, y;
    float cx, cy, rx, ry;
    sel_ensure();
    if (!g.sel) return;
    memset(g.sel, 0, (size_t)g.sel_w * (size_t)g.sel_h);
    sel_norm_rect(x0, y0, x1, y1, &xa, &ya, &xb, &yb);
    if (xb - xa < 1 || yb - ya < 1) {
        g.have_sel = 0;
        return;
    }
    cx = (x0 + x1) * 0.5f;
    cy = (y0 + y1) * 0.5f;
    rx = fabsf(x1 - x0) * 0.5f;
    ry = fabsf(y1 - y0) * 0.5f;
    if (rx < 0.5f) rx = 0.5f;
    if (ry < 0.5f) ry = 0.5f;
    for (y = ya; y < yb; y++) {
        for (x = xa; x < xb; x++) {
            int on = 1;
            if (ellipse) {
                float nx = ((float)x + 0.5f - cx) / rx;
                float ny = ((float)y + 0.5f - cy) / ry;
                on = (nx * nx + ny * ny) <= 1.0f;
            }
            if (on) g.sel[(size_t)y * (size_t)g.sel_w + (size_t)x] = 255;
        }
    }
    g.have_sel = 1;
}

void sel_from_poly(void)
{
    int i, y, x, xa, ya, xb, yb;
    sel_ensure();
    if (!g.sel || g.poly_n < 3) {
        g.have_sel = 0;
        return;
    }
    memset(g.sel, 0, (size_t)g.sel_w * (size_t)g.sel_h);
    xa = g.doc.w;
    ya = g.doc.h;
    xb = 0;
    yb = 0;
    for (i = 0; i < g.poly_n; i++) {
        int px = (int)floorf(g.poly_x[i]);
        int py = (int)floorf(g.poly_y[i]);
        if (px < xa) xa = px;
        if (py < ya) ya = py;
        if (px > xb) xb = px;
        if (py > yb) yb = py;
    }
    xa = clampi(xa, 0, g.doc.w - 1);
    ya = clampi(ya, 0, g.doc.h - 1);
    xb = clampi(xb + 1, 0, g.doc.w);
    yb = clampi(yb + 1, 0, g.doc.h);
    for (y = ya; y < yb; y++) {
        for (x = xa; x < xb; x++) {
            int inside = 0;
            float px = x + 0.5f, py = y + 0.5f;
            for (i = 0; i < g.poly_n; i++) {
                float xA = g.poly_x[i], yA = g.poly_y[i];
                float xB = g.poly_x[(i + 1) % g.poly_n], yB = g.poly_y[(i + 1) % g.poly_n];
                if (((yA > py) != (yB > py)) &&
                    (px < (xB - xA) * (py - yA) / ((yB - yA) == 0 ? 1e-6f : (yB - yA)) + xA))
                    inside = !inside;
            }
            if (inside) g.sel[(size_t)y * (size_t)g.sel_w + (size_t)x] = 255;
        }
    }
    g.have_sel = 1;
}

void sel_from_wand(int sx, int sy)
{
    int w, h, i, n, tol;
    uint8_t seed[4];
    int *qx, *qy, head, tail;
    uint8_t *seen, *src;
    sel_ensure();
    if (!g.sel || sx < 0 || sy < 0 || sx >= g.doc.w || sy >= g.doc.h) return;
    memset(g.sel, 0, (size_t)g.sel_w * (size_t)g.sel_h);
    comp_ensure();
    src = g.disp;
    if (!src) return;
    w = g.doc.w;
    h = g.doc.h;
    n = w * h;
    memcpy(seed, src + ((size_t)sy * (size_t)w + (size_t)sx) * 4, 4);
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
        int x = qx[head], y = qy[head], k;
        static const int ox[4] = { 1, -1, 0, 0 };
        static const int oy[4] = { 0, 0, 1, -1 };
        uint8_t *p = src + ((size_t)y * (size_t)w + (size_t)x) * 4;
        int db = abs(p[0] - seed[0]), dg = abs(p[1] - seed[1]), dr = abs(p[2] - seed[2]);
        head++;
        if (db > tol || dg > tol || dr > tol) continue;
        g.sel[(size_t)y * (size_t)w + (size_t)x] = 255;
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
    g.have_sel = 1;
    (void)i;
}

int sel_bounds(int *x0, int *y0, int *x1, int *y1)
{
    int x, y, found = 0;
    if (!g.have_sel || !g.sel) return 0;
    *x0 = g.sel_w;
    *y0 = g.sel_h;
    *x1 = 0;
    *y1 = 0;
    for (y = 0; y < g.sel_h; y++) {
        for (x = 0; x < g.sel_w; x++) {
            if (!g.sel[(size_t)y * (size_t)g.sel_w + (size_t)x]) continue;
            if (x < *x0) *x0 = x;
            if (y < *y0) *y0 = y;
            if (x + 1 > *x1) *x1 = x + 1;
            if (y + 1 > *y1) *y1 = y + 1;
            found = 1;
        }
    }
    return found;
}

void sel_fill_fg(void)
{
    Layer *L = doc_active(&g.doc);
    int x, y;
    uint8_t b = GetBValue(g.color), gv = GetGValue(g.color), r = GetRValue(g.color);
    if (!L || !L->bgra) return;
    for (y = 0; y < L->h; y++) {
        int dy = L->y + y;
        for (x = 0; x < L->w; x++) {
            int dx = L->x + x, m;
            uint8_t *p, src[4];
            m = sel_mask(dx, dy);
            if (m <= 0) continue;
            p = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            src[0] = b;
            src[1] = gv;
            src[2] = r;
            src[3] = (uint8_t)m;
            blend_pixel(p, src, 255, BLEND_NORMAL);
        }
    }
}

void sel_clear_pixels(void)
{
    Layer *L = doc_active(&g.doc);
    int x, y;
    if (!L || !L->bgra) return;
    if (!g.have_sel) {
        memset(L->bgra, 0, (size_t)L->w * (size_t)L->h * 4);
        return;
    }
    for (y = 0; y < L->h; y++) {
        int dy = L->y + y;
        for (x = 0; x < L->w; x++) {
            int m = sel_mask(L->x + x, dy);
            uint8_t *p;
            if (m <= 0) continue;
            p = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            p[3] = (uint8_t)(p[3] * (255 - m) / 255);
        }
    }
}

void sel_draw_overlay(HDC hdc)
{
    int x, y, vx, vy;
    HPEN pen, old;
    if (!g.have_sel || !g.sel) return;
    pen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
    old = SelectObject(hdc, pen);
    for (y = 0; y < g.sel_h; y++) {
        for (x = 0; x < g.sel_w; x++) {
            int s = g.sel[(size_t)y * (size_t)g.sel_w + (size_t)x];
            int edge;
            if (s < 128) continue;
            edge = (x == 0 || y == 0 || x == g.sel_w - 1 || y == g.sel_h - 1);
            if (!edge) {
                if (g.sel[(size_t)y * g.sel_w + (x ? x - 1 : x)] < 128) edge = 1;
                else if (g.sel[(size_t)y * g.sel_w + (x + 1 < g.sel_w ? x + 1 : x)] < 128) edge = 1;
                else if (g.sel[(size_t)(y ? y - 1 : y) * g.sel_w + x] < 128) edge = 1;
                else if (g.sel[(size_t)(y + 1 < g.sel_h ? y + 1 : y) * g.sel_w + x] < 128) edge = 1;
            }
            if (!edge) continue;
            doc_to_view((float)x, (float)y, &vx, &vy);
            MoveToEx(hdc, vx, vy, NULL);
            LineTo(hdc, vx + 1, vy);
        }
    }
    SelectObject(hdc, old);
    DeleteObject(pen);
}

/* Separable square dilation/erosion and box feather; outside is unselected. */
void sel_modify(int mode,int radius) {
 uint8_t *tmp,*out; size_t n; if(!g.have_sel || !g.sel)return;
 radius=clampi(radius,1,100); n=(size_t)g.sel_w*g.sel_h; tmp=xmalloc(n); out=xmalloc(n);
 for(int pass=0;pass<2;pass++) {
  const uint8_t *src=pass?tmp:g.sel; uint8_t *dst=pass?out:tmp;
  int lines=pass?g.sel_w:g.sel_h,len=pass?g.sel_h:g.sel_w;
  for(int line=0;line<lines;line++) { int sum=0;
   for(int pos=0;pos<len;pos++) { int value=mode==1?255:0;
    if(mode==2 && pos>0) { int rem=pos-radius-1,add=pos+radius;
     if(rem>=0)sum-=src[pass?(size_t)rem*g.sel_w+line:(size_t)line*g.sel_w+rem];
     if(add<len)sum+=src[pass?(size_t)add*g.sel_w+line:(size_t)line*g.sel_w+add];
    } else { if(mode==2)sum=0;
     for(int k=-radius;k<=radius;k++) { int at=pos+k,v=0; if(at>=0 && at<len)v=src[pass?(size_t)at*g.sel_w+line:(size_t)line*g.sel_w+at];
      if(mode==2)sum+=v; else if(mode==0 && v>value)value=v; else if(mode==1 && v<value)value=v;
     }
    }
    if(mode==2)value=(sum+radius)/(2*radius+1);
    dst[pass?(size_t)pos*g.sel_w+line:(size_t)line*g.sel_w+pos]=(uint8_t)value;
   }
  }
 }
 memcpy(g.sel,out,n); free(tmp); free(out);
}
