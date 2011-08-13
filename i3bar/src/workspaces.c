/*
 * i3bar - an xcb-based status- and ws-bar for i3
 *
 * © 2010-2011 Axel Wagner and contributors
 *
 * See file LICNSE for license information
 *
 * src/workspaces.c: Maintaining the workspace-lists
 *
 */
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <zion/parser.h>

#include "common.h"

int workspace_output_cb(zion_op_t *p, const char *key, size_t key_len, int type, const zion_json_val_t *val) {
    i3_ws *walk = (i3_ws *)p->target;
    char *output_name = strndup(val->s, p->val_len);
    walk->output = get_output_by_name(output_name);
    FREE(output_name);

    TAILQ_INSERT_TAIL(walk->output->workspaces, walk, tailq);
    return 1;
}

int workspace_name_cb(zion_op_t *p, const char *key, size_t key_len, int type, const zion_json_val_t *val) {
    i3_ws *walk = (i3_ws *)p->target;
    walk->name = strndup(val->s, p->val_len);
    int ucs2_len;
    xcb_char2b_t *ucs2_name = (xcb_char2b_t *)convert_utf8_to_ucs2(walk->name, &ucs2_len);
    walk->ucs2_name = ucs2_name;
    walk->name_glyphs = ucs2_len;
    walk->name_width = predict_text_extents(ucs2_name, ucs2_len);
    return 1;
}

int arr_cb(zion_ap_t *p, int type, const zion_json_val_t *val) {
    i3_ws *walk = calloc(1, sizeof(i3_ws));
    if (walk == NULL) {
        ELOG("calloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    walk->num = -1;
    zion_op_t *op = p->data;
    op->target = (void *)walk;
    op = op->data;
    op->target = calloc(1, sizeof(rect));
    if (op->target == NULL) {
        ELOG("calloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return 1;
}

/*
 * Start parsing the received json-string
 *
 */
void parse_workspaces_json(char *json) {
    static zion_op_t rect_parser;
    static zion_op_t ws_parser;
    static zion_ap_t arr_parser;
    static bool init = true;
    if (init) {
        zion_op_init(&rect_parser);
        zion_op_add_offset(&rect_parser, "x", 1, ZION_JSON_INT, offsetof(rect, x));
        zion_op_add_offset(&rect_parser, "y", 1, ZION_JSON_INT, offsetof(rect, y));
        zion_op_add_offset(&rect_parser, "width", 5, ZION_JSON_INT, offsetof(rect, w));
        zion_op_add_offset(&rect_parser, "height", 6, ZION_JSON_INT, offsetof(rect, h));

        zion_op_init(&ws_parser);
        zion_op_add_offset(&ws_parser, "num", 3, ZION_JSON_INT, offsetof(i3_ws, num));
        zion_op_add_cb(&ws_parser, "name", 4, ZION_JSON_STRING, &workspace_name_cb);
        zion_op_add_offset(&ws_parser, "visible", 7, ZION_JSON_BOOL, offsetof(i3_ws, visible));
        zion_op_add_offset(&ws_parser, "focused", 7, ZION_JSON_BOOL, offsetof(i3_ws, focused));
        zion_op_add_offset(&ws_parser, "urgent", 6, ZION_JSON_BOOL, offsetof(i3_ws, urgent));
        zion_op_add_op(&ws_parser, "rect", 4, &rect_parser);
        zion_op_add_cb(&ws_parser, "output", 6, ZION_JSON_STRING, &workspace_output_cb);
        /* Kind of uglyish workaround 'til we get awesome macromagic in zion */
        ws_parser.data = &rect_parser;
        zion_ap_init(&arr_parser);
        zion_ap_add_op(&arr_parser, &ws_parser);
    }
    free_workspaces();
    zion_ap_add_cb(&arr_parser, ZION_JSON_OBJECT, &arr_cb);
    arr_parser.data = &ws_parser;
    const char *input = json;
    zion_ap_start(&arr_parser, &input, strlen(json));
}

/*
 * free() all workspace data-structures. Does not free() the heads of the tailqueues.
 *
 */
void free_workspaces() {
    i3_output *outputs_walk;
    if (outputs == NULL) {
        return;
    }
    i3_ws     *ws_walk;

    SLIST_FOREACH(outputs_walk, outputs, slist) {
        if (outputs_walk->workspaces != NULL && !TAILQ_EMPTY(outputs_walk->workspaces)) {
            TAILQ_FOREACH(ws_walk, outputs_walk->workspaces, tailq) {
                FREE(ws_walk->name);
                FREE(ws_walk->ucs2_name);
            }
            FREE_TAILQ(outputs_walk->workspaces, i3_ws);
        }
    }
}
