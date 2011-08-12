/*
 * i3bar - an xcb-based status- and ws-bar for i3
 *
 * © 2010-2011 Axel Wagner and contributors
 *
 * See file LICNSE for license information
 *
 */
#ifndef COMMON_H_
#define COMMON_H_

typedef struct rect_t rect;

struct ev_loop* main_loop;
char            *statusline;
char            *statusline_buffer;

struct rect_t {
    int x;
    int y;
    int w;
    int h;
};

#include <stdbool.h>

#include "queue.h"
#include "child.h"
#include "ipc.h"
#include "outputs.h"
#include "util.h"
#include "workspaces.h"
#include "xcb.h"
#include "ucs2_to_utf8.h"
#include "config.h"

#endif
