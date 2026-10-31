#include "app.h"

void adj_brightness_contrast(Layer *L, int brightness, int contrast)
{
    size_t i, n;
    float c, f;
    int b;
    if (!L || !L->bgra) return;
    brightness = clampi(brightness, -100, 100);
    contrast = clampi(contrast, -100, 100);
    b = (int)(brightness * 2.55f);
    c = (float)contrast;
    f = (259.0f * (c + 255.0f)) / (255.0f * (259.0f - c));
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        uint8_t *p = L->bgra + i * 4;
        int k, x = (int)(i % (size_t)L->w), y = (int)(i / (size_t)L->w);
        if (sel_mask(L->x + x, L->y + y) <= 0) continue;
        for (k = 0; k < 3; k++) {
            float v = f * (p[k] - 128.0f) + 128.0f + (float)b;
            p[k] = clampu8((int)(v + 0.5f));
        }
    }
}

void adj_saturation(Layer *L, int amount)
{
    size_t i, n;
    float s;
    if (!L || !L->bgra) return;
    amount = clampi(amount, -100, 100);
    s = 1.0f + amount / 100.0f;
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        uint8_t *p = L->bgra + i * 4;
        int x = (int)(i % (size_t)L->w), y = (int)(i / (size_t)L->w);
        float b, gv, r, gray;
        if (sel_mask(L->x + x, L->y + y) <= 0) continue;
        b = p[0]; gv = p[1]; r = p[2];
        gray = 0.114f * b + 0.587f * gv + 0.299f * r;
        p[0] = clampu8((int)(gray + (b - gray) * s + 0.5f));
        p[1] = clampu8((int)(gray + (gv - gray) * s + 0.5f));
        p[2] = clampu8((int)(gray + (r - gray) * s + 0.5f));
    }
}

int adj_rotate90(Layer *L)
{
    int x, y, nw, nh;
    uint8_t *dst;
    if (!L || !L->bgra) return 0;
    nw = L->h;
    nh = L->w;
    dst = xcalloc((size_t)nw * (size_t)nh, 4);
    for (y = 0; y < L->h; y++) {
        for (x = 0; x < L->w; x++) {
            const uint8_t *s = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            int dx = nw - 1 - y;
            int dy = x;
            uint8_t *d = dst + ((size_t)dy * (size_t)nw + (size_t)dx) * 4;
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d[3] = s[3];
        }
    }
    free(L->bgra);
    L->bgra = dst;
    L->w = nw;
    L->h = nh;
    return 1;
}

void adj_flip_h(Layer *L)
{
    int x, y;
    if (!L || !L->bgra) return;
    for (y = 0; y < L->h; y++) {
        for (x = 0; x < L->w / 2; x++) {
            uint8_t *a = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            uint8_t *b = L->bgra + ((size_t)y * (size_t)L->w + (size_t)(L->w - 1 - x)) * 4;
            uint8_t t[4];
            memcpy(t, a, 4);
            memcpy(a, b, 4);
            memcpy(b, t, 4);
        }
    }
}

void adj_flip_v(Layer *L)
{
    int x, y;
    if (!L || !L->bgra) return;
    for (y = 0; y < L->h / 2; y++) {
        for (x = 0; x < L->w; x++) {
            uint8_t *a = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            uint8_t *b = L->bgra + ((size_t)(L->h - 1 - y) * (size_t)L->w + (size_t)x) * 4;
            uint8_t t[4];
            memcpy(t, a, 4);
            memcpy(a, b, 4);
            memcpy(b, t, 4);
        }
    }
}

static int adj_sel(int docx, int docy)
{
    return sel_mask(docx, docy) > 0;
}

void adj_hue(Layer *L, int amount)
{
    size_t i, n;
    float a;
    if (!L || !L->bgra) return;
    amount = clampi(amount, -180, 180);
    a = amount * 3.14159265f / 180.0f;
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        uint8_t *p = L->bgra + i * 4;
        int x = (int)(i % (size_t)L->w), y = (int)(i / (size_t)L->w);
        float b, gv, r, gray, cb, cr, nb, ng, nr, cos_a, sin_a;
        if (!adj_sel(L->x + x, L->y + y)) continue;
        b = p[0]; gv = p[1]; r = p[2];
        gray = 0.114f * b + 0.587f * gv + 0.299f * r;
        cb = b - gray;
        cr = r - gray;
        cos_a = cosf(a);
        sin_a = sinf(a);
        nb = gray + cb * cos_a + (0.492f * (gv - gray)) * sin_a;
        nr = gray + cr * cos_a - (0.877f * (gv - gray)) * sin_a;
        ng = (gray - 0.114f * nb - 0.299f * nr) / 0.587f;
        p[0] = clampu8((int)(nb + 0.5f));
        p[1] = clampu8((int)(ng + 0.5f));
        p[2] = clampu8((int)(nr + 0.5f));
    }
}

void adj_invert(Layer *L)
{
    size_t i, n;
    if (!L || !L->bgra) return;
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        uint8_t *p = L->bgra + i * 4;
        int x = (int)(i % (size_t)L->w), y = (int)(i / (size_t)L->w);
        if (!adj_sel(L->x + x, L->y + y)) continue;
        p[0] = (uint8_t)(255 - p[0]);
        p[1] = (uint8_t)(255 - p[1]);
        p[2] = (uint8_t)(255 - p[2]);
    }
}

void adj_grayscale(Layer *L)
{
    size_t i, n;
    if (!L || !L->bgra) return;
    n = (size_t)L->w * (size_t)L->h;
    for (i = 0; i < n; i++) {
        uint8_t *p = L->bgra + i * 4;
        int x = (int)(i % (size_t)L->w), y = (int)(i / (size_t)L->w);
        int g;
        if (!adj_sel(L->x + x, L->y + y)) continue;
        g = (int)(0.114f * p[0] + 0.587f * p[1] + 0.299f * p[2] + 0.5f);
        p[0] = p[1] = p[2] = clampu8(g);
    }
}

void filt_blur(Layer *L, int radius)
{
    int r, x, y, k, pass;
    uint8_t *tmp;
    if (!L || !L->bgra) return;
    r = clampi(radius, 1, 40);
    tmp = xmalloc((size_t)L->w * (size_t)L->h * 4);
    for (pass = 0; pass < 2; pass++) {
        memcpy(tmp, L->bgra, (size_t)L->w * (size_t)L->h * 4);
        for (y = 0; y < L->h; y++) {
            for (x = 0; x < L->w; x++) {
                int s[4] = { 0, 0, 0, 0 }, c = 0, t;
                uint8_t *d;
                if (!adj_sel(L->x + x, L->y + y)) continue;
                for (k = -r; k <= r; k++) {
                    int xx = x + (pass ? 0 : k);
                    int yy = y + (pass ? k : 0);
                    uint8_t *p;
                    if (xx < 0 || yy < 0 || xx >= L->w || yy >= L->h) continue;
                    p = tmp + ((size_t)yy * (size_t)L->w + (size_t)xx) * 4;
                    for (t = 0; t < 4; t++) s[t] += p[t];
                    c++;
                }
                if (c < 1) continue;
                d = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
                for (t = 0; t < 4; t++) d[t] = (uint8_t)(s[t] / c);
            }
        }
    }
    free(tmp);
}

void filt_sharpen(Layer *L)
{
    int x, y, k;
    uint8_t *tmp;
    const int ker[9] = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
    if (!L || !L->bgra) return;
    tmp = xmalloc((size_t)L->w * (size_t)L->h * 4);
    memcpy(tmp, L->bgra, (size_t)L->w * (size_t)L->h * 4);
    for (y = 1; y < L->h - 1; y++) {
        for (x = 1; x < L->w - 1; x++) {
            int ch, acc;
            uint8_t *d;
            if (!adj_sel(L->x + x, L->y + y)) continue;
            d = L->bgra + ((size_t)y * (size_t)L->w + (size_t)x) * 4;
            for (ch = 0; ch < 3; ch++) {
                acc = 0;
                for (k = 0; k < 9; k++) {
                    int xx = x + (k % 3) - 1;
                    int yy = y + (k / 3) - 1;
                    acc += tmp[((size_t)yy * (size_t)L->w + (size_t)xx) * 4 + ch] * ker[k];
                }
                d[ch] = clampu8(acc);
            }
        }
    }
    free(tmp);
}

int adj_crop_doc(Document *d, int x, int y, int w, int h)
{
    int i;
    if (!d || w < 1 || h < 1) return 0;
    x = clampi(x, 0, d->w - 1);
    y = clampi(y, 0, d->h - 1);
    if (x + w > d->w) w = d->w - x;
    if (y + h > d->h) h = d->h - y;
    if (w < 1 || h < 1) return 0;

    for (i = 0; i < d->count; i++) {
        Layer *L = &d->layers[i];
        int nx0 = L->x > x ? L->x : x;
        int ny0 = L->y > y ? L->y : y;
        int nx1 = (L->x + L->w) < (x + w) ? (L->x + L->w) : (x + w);
        int ny1 = (L->y + L->h) < (y + h) ? (L->y + L->h) : (y + h);
        if (nx1 <= nx0 || ny1 <= ny0) {
            free(L->bgra);
            L->w = 1;
            L->h = 1;
            L->x = 0;
            L->y = 0;
            L->bgra = xcalloc(1, 4);
            continue;
        } else {
            int nw = nx1 - nx0, nh = ny1 - ny0;
            uint8_t *nb = xcalloc((size_t)nw * (size_t)nh, 4);
            int ly, lx;
            for (ly = 0; ly < nh; ly++) {
                int sy = ny0 - L->y + ly;
                for (lx = 0; lx < nw; lx++) {
                    int sx = nx0 - L->x + lx;
                    memcpy(nb + ((size_t)ly * (size_t)nw + (size_t)lx) * 4,
                           L->bgra + ((size_t)sy * (size_t)L->w + (size_t)sx) * 4, 4);
                }
            }
            free(L->bgra);
            L->bgra = nb;
            L->w = nw;
            L->h = nh;
            L->x = nx0 - x;
            L->y = ny0 - y;
        }
    }
    d->w = w;
    d->h = h;
    sel_reset();
    return 1;
}

/* Blend through soft selection masks without changing alpha. */
static void adj_lut(Layer *L,const uint8_t lut[256]) {
 if(!L || !L->bgra) return;
 for(int y=0;y<L->h;y++) for(int x=0;x<L->w;x++) { int m=sel_mask(L->x+x,L->y+y); uint8_t *p=L->bgra+((size_t)y*L->w+x)*4; for(int k=0;k<3;k++) p[k]=(uint8_t)((p[k]*(255-m)+lut[p[k]]*m+127)/255); }
}
void adj_auto_contrast(Layer *L) {
 int lo=255,hi=0; uint8_t lut[256]; if(!L || !L->bgra) return;
 for(int y=0;y<L->h;y++) for(int x=0;x<L->w;x++) { uint8_t *p=L->bgra+((size_t)y*L->w+x)*4; if(!p[3] || !sel_mask(L->x+x,L->y+y)) continue; for(int k=0;k<3;k++) { if(p[k]<lo)lo=p[k]; if(p[k]>hi)hi=p[k]; } }
 if(hi<=lo)return;
 for(int i=0;i<256;i++) { lut[i]=clampu8(((i-lo)*255+(hi-lo)/2)/(hi-lo)); }
 adj_lut(L,lut);
}
void adj_posterize(Layer *L,int levels) {
 uint8_t lut[256]; levels=clampi(levels,2,256);
 for(int i=0;i<256;i++) { int step=(i*(levels-1)+127)/255; lut[i]=(uint8_t)((step*255+(levels-1)/2)/(levels-1)); } adj_lut(L,lut);
}
void adj_threshold(Layer *L,int value) {
 if(!L || !L->bgra) return;
 value=clampi(value,0,255);
 for(int y=0;y<L->h;y++) for(int x=0;x<L->w;x++) { uint8_t *p=L->bgra+((size_t)y*L->w+x)*4; int m=sel_mask(L->x+x,L->y+y),gray=(p[2]*299+p[1]*587+p[0]*114+500)/1000,v=gray>=value?255:0; for(int k=0;k<3;k++) p[k]=(uint8_t)((p[k]*(255-m)+v*m+127)/255); }
}
int adj_rotate_quarters(Layer *L,int turns) {
 int nw,nh; uint8_t *dst; if(!L || !L->bgra)return 0;
 turns=((turns%4)+4)%4; if(!turns)return 1; nw=turns%2?L->h:L->w; nh=turns%2?L->w:L->h; dst=xmalloc((size_t)nw*nh*4);
 for(int y=0;y<L->h;y++) for(int x=0;x<L->w;x++) { int dx=turns==1?L->h-1-y:turns==2?L->w-1-x:y,dy=turns==1?x:turns==2?L->h-1-y:L->w-1-x; memcpy(dst+((size_t)dy*nw+dx)*4,L->bgra+((size_t)y*L->w+x)*4,4); }
 free(L->bgra); L->bgra=dst; L->w=nw; L->h=nh; return 1;
}
