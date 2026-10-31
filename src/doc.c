#include "app.h"

void doc_init(Document *d)
{
    if (!d) return;
    memset(d, 0, sizeof(*d));
    d->active = -1;
}

void doc_free(Document *d)
{
    int i;
    if (!d) return;
    if (d->layers) {
        for (i = 0; i < d->count; i++) layer_free(&d->layers[i]);
        free(d->layers);
    }
    memset(d, 0, sizeof(*d));
    d->active = -1;
}

int doc_clone(Document *dst, const Document *src)
{
    int i;
    if (!dst || !src) return 0;
    doc_free(dst);
    dst->w = src->w;
    dst->h = src->h;
    dst->active = src->active;
    dst->dirty = src->dirty;
    wcsncpy(dst->path, src->path, MAX_PATH - 1);
    dst->path[MAX_PATH - 1] = 0;
    dst->count = src->count;
    dst->cap = src->count;
    if (src->count > 0) {
        dst->layers = xcalloc((size_t)src->count, sizeof(Layer));
        for (i = 0; i < src->count; i++) {
            if (!layer_copy(&dst->layers[i], &src->layers[i])) return 0;
        }
    }
    return 1;
}

static int doc_grow(Document *d)
{
    int ncap;
    Layer *nl;
    if (d->count < d->cap) return 1;
    if (d->count >= DAGRIM_MAX_LAYERS) return 0;
    ncap = d->cap ? d->cap * 2 : 8;
    if (ncap > DAGRIM_MAX_LAYERS) ncap = DAGRIM_MAX_LAYERS;
    nl = (Layer *)realloc(d->layers, (size_t)ncap * sizeof(Layer));
    if (!nl) return 0;
    d->layers = nl;
    d->cap = ncap;
    return 1;
}

int doc_insert(Document *d, int at, Layer *L)
{
    if (!d || !L || !L->bgra) return 0;
    if (!doc_grow(d)) return 0;
    if (at < 0) at = 0;
    if (at > d->count) at = d->count;
    if (at < d->count) {
        memmove(&d->layers[at + 1], &d->layers[at], (size_t)(d->count - at) * sizeof(Layer));
    }
    d->layers[at] = *L;
    memset(L, 0, sizeof(*L));
    d->count++;
    d->active = at;
    return 1;
}

int doc_new(Document *d, int w, int h, int white)
{
    Layer L;
    wchar_t name[64];
    if (!d || w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE) return 0;
    doc_free(d);
    d->w = w;
    d->h = h;
    d->active = -1;
    wcscpy(name, white ? L"배경" : L"레이어 1");
    if (!layer_create(&L, w, h, name)) return 0;
    if (white) layer_fill_white(&L);
    if (!doc_insert(d, 0, &L)) {
        layer_free(&L);
        return 0;
    }
    d->dirty = 0;
    return 1;
}

Layer *doc_active(Document *d)
{
    if (!d || d->active < 0 || d->active >= d->count) return NULL;
    return &d->layers[d->active];
}

int doc_add_empty(Document *d)
{
    Layer L;
    wchar_t name[64];
    if (!d || d->w < 1 || d->h < 1) return 0;
    swprintf(name, 64, L"레이어 %d", d->count + 1);
    if (!layer_create(&L, d->w, d->h, name)) return 0;
    return doc_insert(d, d->count, &L);
}

int doc_duplicate_active(Document *d)
{
    Layer copy;
    Layer *src = doc_active(d);
    wchar_t name[64];
    if (!src) return 0;
    if (!layer_copy(&copy, src)) return 0;
    swprintf(name, 64, L"%s 복사", src->name);
    name[63] = 0;
    wcsncpy(copy.name, name, 63);
    copy.name[63] = 0;
    copy.x = src->x + 8;
    copy.y = src->y + 8;
    return doc_insert(d, d->active + 1, &copy);
}

int doc_delete_active(Document *d)
{
    int i;
    if (!d || d->active < 0 || d->active >= d->count) return 0;
    layer_free(&d->layers[d->active]);
    for (i = d->active; i < d->count - 1; i++) d->layers[i] = d->layers[i + 1];
    d->count--;
    if (d->count == 0) d->active = -1;
    else if (d->active >= d->count) d->active = d->count - 1;
    return 1;
}

int doc_move_active(Document *d, int dir)
{
    int to;
    Layer tmp;
    if (!d || d->active < 0) return 0;
    to = d->active + dir;
    if (to < 0 || to >= d->count) return 0;
    tmp = d->layers[d->active];
    d->layers[d->active] = d->layers[to];
    d->layers[to] = tmp;
    d->active = to;
    return 1;
}

void doc_ensure_canvas(Document *d, int w, int h)
{
    if (!d) return;
    if (w > d->w) d->w = w;
    if (h > d->h) d->h = h;
    if (d->w > DAGRIM_MAX_SIZE) d->w = DAGRIM_MAX_SIZE;
    if (d->h > DAGRIM_MAX_SIZE) d->h = DAGRIM_MAX_SIZE;
}

int doc_merge_down(Document *d)
{
    Layer *top, *bot;
    int x0, y0, x1, y1, nw, nh, x, y;
    uint8_t *nb;
    if (!d || d->active < 1) return 0;
    top = &d->layers[d->active];
    bot = &d->layers[d->active - 1];
    if (!top->bgra || !bot->bgra) return 0;
    x0 = top->x < bot->x ? top->x : bot->x;
    y0 = top->y < bot->y ? top->y : bot->y;
    x1 = (top->x + top->w) > (bot->x + bot->w) ? (top->x + top->w) : (bot->x + bot->w);
    y1 = (top->y + top->h) > (bot->y + bot->h) ? (top->y + top->h) : (bot->y + bot->h);
    nw = x1 - x0;
    nh = y1 - y0;
    if (nw < 1 || nh < 1) return 0;
    nb = xcalloc((size_t)nw * (size_t)nh, 4);
    for (y = 0; y < bot->h; y++) {
        for (x = 0; x < bot->w; x++) {
            int dx = bot->x + x - x0, dy = bot->y + y - y0;
            memcpy(nb + ((size_t)dy * (size_t)nw + (size_t)dx) * 4,
                   bot->bgra + ((size_t)y * (size_t)bot->w + (size_t)x) * 4, 4);
        }
    }
    for (y = 0; y < top->h; y++) {
        for (x = 0; x < top->w; x++) {
            int dx = top->x + x - x0, dy = top->y + y - y0;
            uint8_t *dst = nb + ((size_t)dy * (size_t)nw + (size_t)dx) * 4;
            uint8_t *src = top->bgra + ((size_t)y * (size_t)top->w + (size_t)x) * 4;
            blend_pixel(dst, src, top->opacity, top->blend);
        }
    }
    free(bot->bgra);
    bot->bgra = nb;
    bot->w = nw;
    bot->h = nh;
    bot->x = x0;
    bot->y = y0;
    layer_free(top);
    for (x = d->active; x < d->count - 1; x++) d->layers[x] = d->layers[x + 1];
    d->count--;
    d->active--;
    return 1;
}

int doc_flatten(Document *d)
{
    uint8_t *flat;
    Layer L;
    int i;
    if (!d || d->w < 1 || d->h < 1) return 0;
    flat = xmalloc((size_t)d->w * (size_t)d->h * 4);
    if (!flatten_document(d, flat)) {
        free(flat);
        return 0;
    }
    memset(&L, 0, sizeof(L));
    L.w = d->w;
    L.h = d->h;
    L.bgra = flat;
    L.opacity = 255;
    L.visible = 1;
    L.blend = BLEND_NORMAL;
    wcsncpy(L.name, L"배경", 63);
    for (i = 0; i < d->count; i++) layer_free(&d->layers[i]);
    free(d->layers);
    d->layers = NULL;
    d->count = d->cap = 0;
    d->active = -1;
    return doc_insert(d, 0, &L);
}

int doc_canvas_size(Document *d, int w, int h)
{
    if (!d || w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE) return 0;
    d->w = w;
    d->h = h;
    sel_reset();
    return 1;
}

int doc_image_size(Document *d, int w, int h)
{
    int i;
    float sx, sy;
    if (!d || w < 1 || h < 1 || w > DAGRIM_MAX_SIZE || h > DAGRIM_MAX_SIZE) return 0;
    if (d->w < 1 || d->h < 1) return 0;
    sx = (float)w / (float)d->w;
    sy = (float)h / (float)d->h;
    for (i = 0; i < d->count; i++) {
        Layer *L = &d->layers[i];
        int nw = clampi((int)floorf(L->w * sx + 0.5f), 1, DAGRIM_MAX_SIZE);
        int nh = clampi((int)floorf(L->h * sy + 0.5f), 1, DAGRIM_MAX_SIZE);
        if (!layer_scale_from(L, L->bgra, L->w, L->h, nw, nh)) return 0;
        L->x = (int)floorf(L->x * sx + 0.5f);
        L->y = (int)floorf(L->y * sy + 0.5f);
    }
    d->w = w;
    d->h = h;
    sel_reset();
    return 1;
}
