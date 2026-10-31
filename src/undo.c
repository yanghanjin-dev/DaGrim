#include "app.h"

static void stack_free(Document *s, int *n)
{
    int i;
    for (i = 0; i < *n; i++) doc_free(&s[i]);
    *n = 0;
}

void undo_clear(void)
{
    stack_free(g.undo_s, &g.undo_n);
    stack_free(g.redo_s, &g.redo_n);
}

void undo_push(void)
{
    stack_free(g.redo_s, &g.redo_n);
    if (g.undo_n == DAGRIM_MAX_UNDO) {
        doc_free(&g.undo_s[0]);
        memmove(&g.undo_s[0], &g.undo_s[1], (DAGRIM_MAX_UNDO - 1) * sizeof(Document));
        memset(&g.undo_s[DAGRIM_MAX_UNDO - 1], 0, sizeof(Document));
        g.undo_n--;
    }
    doc_init(&g.undo_s[g.undo_n]);
    if (doc_clone(&g.undo_s[g.undo_n], &g.doc)) g.undo_n++;
}

int undo_can(void)
{
    return g.undo_n > 0;
}

int redo_can(void)
{
    return g.redo_n > 0;
}

void undo_perform(void)
{
    Document cur;
    if (g.undo_n <= 0) return;
    doc_init(&cur);
    if (!doc_clone(&cur, &g.doc)) return;
    doc_free(&g.doc);
    g.doc = g.undo_s[g.undo_n - 1];
    memset(&g.undo_s[g.undo_n - 1], 0, sizeof(Document));
    g.undo_n--;
    if (g.redo_n == DAGRIM_MAX_UNDO) {
        doc_free(&g.redo_s[0]);
        memmove(&g.redo_s[0], &g.redo_s[1], (DAGRIM_MAX_UNDO - 1) * sizeof(Document));
        g.redo_n--;
    }
    g.redo_s[g.redo_n++] = cur;
    g.doc.dirty = 1;
    sel_reset();
    comp_invalidate_all();
}

void redo_perform(void)
{
    Document cur;
    if (g.redo_n <= 0) return;
    doc_init(&cur);
    if (!doc_clone(&cur, &g.doc)) return;
    doc_free(&g.doc);
    g.doc = g.redo_s[g.redo_n - 1];
    memset(&g.redo_s[g.redo_n - 1], 0, sizeof(Document));
    g.redo_n--;
    if (g.undo_n == DAGRIM_MAX_UNDO) {
        doc_free(&g.undo_s[0]);
        memmove(&g.undo_s[0], &g.undo_s[1], (DAGRIM_MAX_UNDO - 1) * sizeof(Document));
        g.undo_n--;
    }
    g.undo_s[g.undo_n++] = cur;
    g.doc.dirty = 1;
    sel_reset();
    comp_invalidate_all();
}
