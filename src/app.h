#ifndef DAGRIM_APP_H
#define DAGRIM_APP_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wchar.h>

#include "resource.h"
#include "version.h"

#define DAGRIM_MAX_LAYERS 64
#define DAGRIM_MAX_UNDO   8
#define DAGRIM_MAX_SIZE   8192

enum {
    BLEND_NORMAL = 0,
    BLEND_MULTIPLY,
    BLEND_SCREEN,
    BLEND_OVERLAY,
    BLEND_DARKEN,
    BLEND_LIGHTEN,
    BLEND_ADD,
    BLEND_DIFFERENCE
};

enum {
    TOOL_MOVE = 0,
    TOOL_SEL_RECT,
    TOOL_SEL_ELLIPSE,
    TOOL_LASSO,
    TOOL_WAND,
    TOOL_CROP,
    TOOL_EYEDROPPER,
    TOOL_BRUSH,
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_CLONE,
    TOOL_BUCKET,
    TOOL_GRADIENT,
    TOOL_DODGE,
    TOOL_BURN,
    TOOL_SPONGE,
    TOOL_TEXT,
    TOOL_SHAPE_RECT,
    TOOL_SHAPE_ELLIPSE,
    TOOL_SHAPE_LINE,
    TOOL_HAND,
    TOOL_COUNT
};

#define DAGRIM_MAX_POLY 4096

enum {
    BRUSH_ROUND = 0,
    BRUSH_CALLI,
    BRUSH_CALLI_PEN,
    BRUSH_AIR,
    BRUSH_OIL,
    BRUSH_CRAYON,
    BRUSH_MARKER,
    BRUSH_PENCIL,
    BRUSH_WATER,
    BRUSH_PRESET_COUNT
};

enum {
    SHAPE_LINE = 0,
    SHAPE_CURVE,
    SHAPE_CIRCLE,
    SHAPE_SQUARE,
    SHAPE_ROUNDRECT,
    SHAPE_PARA,
    SHAPE_TRI,
    SHAPE_RTRI,
    SHAPE_DIAMOND,
    SHAPE_PENTA,
    SHAPE_HEX,
    SHAPE_ARROW_R,
    SHAPE_ARROW_L,
    SHAPE_ARROW_U,
    SHAPE_ARROW_D,
    SHAPE_STAR4,
    SHAPE_STAR5,
    SHAPE_STAR6,
    SHAPE_BUBBLE,
    SHAPE_BUBBLE_ROUND,
    SHAPE_CLOUD,
    SHAPE_GRID,
    SHAPE_RECT = SHAPE_GRID,
    SHAPE_ELLIPSE,
    SHAPE_COUNT
};

typedef struct Layer {
    wchar_t name[64];
    int x, y, w, h;
    uint8_t *bgra;
    int opacity;
    int visible;
    int blend;
} Layer;

typedef struct Document {
    int w, h;
    Layer *layers;
    int count, cap;
    int active;
    int dirty;
    wchar_t path[MAX_PATH];
} Document;

typedef struct App {
    HINSTANCE inst;
    HWND hwnd, toolbar, canvas, layers, status;
    HWND opacity_track, blend_combo;
    HWND btn_add, btn_dup, btn_del, btn_up, btn_down, btn_import;
    HWND brush_popup, paintbar;
    HFONT font, font_small;
    HBRUSH br_bg, br_panel, br_canvas, br_sel, br_btn, br_btn_hot;
    HCURSOR cur_arrow, cur_cross, cur_move, cur_hand;
    HCURSOR cur_nwse, cur_nesw, cur_we, cur_ns;
    HICON icon;

    Document doc;
    uint8_t *disp;
    int buf_w, buf_h;
    int comp_ok;
    int dirty_all;
    int has_dirty;
    RECT dirty;

    int tool;
    int tool_scroll;
    COLORREF color, color_bg;
    int pick_bg;
    int brush_size;
    int brush_opacity;
    int brush_preset;
    int shape_kind;
    int shape_fill;
    int shape_outline;
    int shape_stroke_preset;
    int stroke_width;
    int wand_tol;
    float stroke_dx, stroke_dy, stroke_len;

    float zoom, pan_x, pan_y;
    int fitted;

    int dragging, panning, space_down;
    int last_sx, last_sy;
    int move_lx, move_ly;
    float move_dx, move_dy;
    int have_stroke;
    float stroke_x, stroke_y;

    int resize_on, resize_handle;
    int resize_ox, resize_oy, resize_ow, resize_oh;
    uint8_t *resize_src;
    int resize_sw, resize_sh;

    int crop_on;
    float crop_x0, crop_y0, crop_x1, crop_y1;

    uint8_t *sel;
    int sel_w, sel_h, have_sel;
    float poly_x[DAGRIM_MAX_POLY];
    float poly_y[DAGRIM_MAX_POLY];
    int poly_n;

    int clone_ok;
    float clone_sx, clone_sy, clone_ox, clone_oy;

    int text_size;
    wchar_t text_buf[256];
    int filt_r;

    int layer_scroll;
    int dpi;
    int syncing;
    int opacity_undo;

    int new_w, new_h, new_white;
    int adj_b, adj_c, adj_s;
    wchar_t rename_buf[64];
    COLORREF cust_colors[16];
    wchar_t status_text[256];

    Document undo_s[DAGRIM_MAX_UNDO];
    Document redo_s[DAGRIM_MAX_UNDO];
    int undo_n, redo_n;
} App;

extern App g;

/* util */
void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t sz);
int clampi(int v, int lo, int hi);
float clampf(float v, float lo, float hi);
uint8_t clampu8(int v);
int wide_to_utf8(const wchar_t *s, char *o, int on);
int utf8_to_wide(const char *s, wchar_t *o, int on);
int read_file(const wchar_t *path, uint8_t **data, size_t *len);
int write_file(const wchar_t *path, const uint8_t *data, size_t len);
const wchar_t *path_ext(const wchar_t *path);
const wchar_t *path_filename(const wchar_t *path);
int ext_eq(const wchar_t *path, const wchar_t *ext);
UINT dpi_for(HWND h);
int ui_scale(int px);
int edit_focused(void);

/* layer / doc */
int layer_create(Layer *L, int w, int h, const wchar_t *name);
int layer_copy(Layer *dst, const Layer *src);
void layer_free(Layer *L);
void layer_fill_white(Layer *L);
int layer_scale_from(Layer *L, const uint8_t *src, int sw, int sh, int nw, int nh);
void doc_init(Document *d);
void doc_free(Document *d);
int doc_clone(Document *dst, const Document *src);
int doc_new(Document *d, int w, int h, int white);
Layer *doc_active(Document *d);
int doc_insert(Document *d, int at, Layer *L);
int doc_add_empty(Document *d);
int doc_duplicate_active(Document *d);
int doc_delete_active(Document *d);
int doc_move_active(Document *d, int dir);
void doc_ensure_canvas(Document *d, int w, int h);

/* composite */
void blend_pixel(uint8_t *dst, const uint8_t *src, int opacity, int blend);
void comp_invalidate_all(void);
void comp_invalidate_rect(int x, int y, int w, int h);
void comp_ensure(void);
int flatten_document(const Document *d, uint8_t *dst);

/* io */
int io_load_image(const wchar_t *path, uint8_t **bgra, int *w, int *h);
int io_load_image_mem(const uint8_t *data, size_t len, uint8_t **bgra, int *w, int *h);
int io_load_dib(const void *data, size_t len, uint8_t **bgra, int *w, int *h);
int io_load_hbitmap(HBITMAP hbmp, uint8_t **bgra, int *w, int *h);
int io_save_png(const wchar_t *path, const uint8_t *bgra, int w, int h);
int io_save_jpg(const wchar_t *path, const uint8_t *bgra, int w, int h, int quality);
int io_load_dagrim(const wchar_t *path, Document *d);
int io_save_dagrim(const wchar_t *path, const Document *d);

/* undo */
void undo_clear(void);
void undo_push(void);
int undo_can(void);
int redo_can(void);
void undo_perform(void);
void redo_perform(void);

/* adjust */
void adj_brightness_contrast(Layer *L, int brightness, int contrast);
void adj_saturation(Layer *L, int amount);
int adj_rotate90(Layer *L);
void adj_flip_h(Layer *L);
void adj_flip_v(Layer *L);
int adj_crop_doc(Document *d, int x, int y, int w, int h);
void adj_hue(Layer *L, int amount);
void adj_invert(Layer *L);
void adj_grayscale(Layer *L);
void filt_blur(Layer *L, int radius);
void filt_sharpen(Layer *L);
int doc_merge_down(Document *d);
int doc_flatten(Document *d);
int doc_canvas_size(Document *d, int w, int h);
int doc_image_size(Document *d, int w, int h);

void adj_auto_contrast(Layer *L);
void adj_threshold(Layer *L, int value);
void adj_posterize(Layer *L, int levels);
int adj_rotate_quarters(Layer *L, int turns);
int ui_value_dialog(const wchar_t *title, const wchar_t *label, int lo, int hi, int *value);
void sel_modify(int mode, int radius);

/* selection */
void sel_reset(void);
void sel_ensure(void);
void sel_all(void);
void sel_deselect(void);
void sel_invert(void);
int sel_mask(int docx, int docy);
void sel_from_rect(float x0, float y0, float x1, float y1, int ellipse);
void sel_from_poly(void);
void sel_from_wand(int x, int y);
void sel_fill_fg(void);
void sel_clear_pixels(void);
void sel_draw_overlay(HDC hdc);
int sel_bounds(int *x0, int *y0, int *x1, int *y1);

/* tools / view */
int view_to_doc(int vx, int vy, float *dx, float *dy);
void doc_to_view(float dx, float dy, int *vx, int *vy);
void view_fit(void);
void view_actual(void);
void view_zoom_at(int mx, int my, int zoom_in);
void tools_on_down(int sx, int sy, int button);
void tools_on_move(int sx, int sy);
void tools_on_up(int sx, int sy);
void tools_draw_overlay(HDC hdc);
void tools_apply_crop(void);
void tools_cancel_crop(void);
void tools_end_resize(void);
void tools_set_cursor(int sx, int sy);
void tools_set_tool(int tool);
const wchar_t *tools_name(int tool);
void tools_set_shape(int kind);
const wchar_t *brush_preset_name(int preset);
int brush_active_preset(void);
float brush_spacing(void);
void brush_paint(float fx, float fy, int mode);
void brush_render_preview(uint8_t *bgra, int w, int h, int preset);
int shape_contains(int kind, float px, float py, float x0, float y0, float x1, float y1);
void shape_norm_box(float *x0, float *y0, float *x1, float *y1);
void shape_raster_layer(Layer *L, float x0, float y0, float x1, float y1, int kind);
void ui_draw_tool_icon(HDC hdc, int tool, const RECT *rc, int selected);
void paintbar_register(HINSTANCE inst);
LRESULT CALLBACK PaintbarProc(HWND, UINT, WPARAM, LPARAM);
int shape_outline(int kind, float x0, float y0, float x1, float y1, POINT *out, int maxpt);
void brush_popup_show(void);
void brush_popup_hide(void);
void brush_popup_toggle(void);
void brush_popup_layout(void);
int brush_popup_visible(void);
LRESULT CALLBACK BrushPopupProc(HWND, UINT, WPARAM, LPARAM);

/* ui */
void ui_init_visuals(void);
void ui_free_visuals(void);
void ui_register_classes(HINSTANCE inst);
void ui_sync_layer_controls(void);
void ui_update_status(void);
void ui_choose_color(void);
void ui_scroll_tool_into_view(void);
LRESULT CALLBACK ToolbarProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CanvasProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LayerProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK StatusProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NewDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BcDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SatDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RenameDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ScaleDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK TextDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK HueDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BlurDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
void ui_choose_color_slot(int bg);

/* app */
void app_refresh(void);
void app_set_dirty(int dirty);
void update_title(void);
int confirm_discard(void);
void app_command(int id);
int app_open_path(const wchar_t *path);
int app_add_image_layer(uint8_t *bgra, int w, int h, const wchar_t *name, int push_undo);


void adj_auto_contrast(Layer *L);
void adj_threshold(Layer *L,int value);
void adj_posterize(Layer *L,int levels);
int adj_rotate_quarters(Layer *L,int turns);
int ui_value_dialog(const wchar_t*,const wchar_t*,int,int,int*);
void sel_modify(int mode,int radius);

#endif /* DAGRIM_APP_H */
