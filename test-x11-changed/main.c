/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009-2011 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * test-x11-changed/main.c: Utility to test if the contents of the X11 server
 * have changed within the given delay. Can be used to verify that a specified
 * IPC command does not take longer than <delay> μsec to have a visible effect.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <getopt.h>
#include <err.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "libi3.h"
#include "i3/ipc.h"

#define FREE(pointer) do { \
        if (pointer != NULL) { \
                free(pointer); \
                pointer = NULL; \
        } \
} \
while (0)


static void init_x_and_imlib() {
    Display *disp = XOpenDisplay(NULL);
    if (!disp)
        errx(EXIT_FAILURE, "Cannot open X display.\n");

    int screen = DefaultScreen(disp);

    imlib_context_set_display(disp);
    imlib_context_set_visual(DefaultVisual(disp, screen));
    imlib_context_set_colormap(DefaultColormap(disp, screen));
    imlib_context_set_color_modifier(NULL);
    imlib_context_set_operation(IMLIB_OP_COPY);
    imlib_context_set_drawable(RootWindow(disp, screen));
}

int main(int argc, char *argv[]) {
    int opt, optind = 0;
    struct option long_opts[] = {
        {"ipc-command", required_argument, 0, 'c'},
        {"ipc_command", required_argument, 0, 'c'},
        {"delay", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    char *ipc_command = NULL;
    int delay = -1;

    while ((opt = getopt_long(argc, argv, "c:d:h", long_opts, &optind)) != -1) {
        switch (opt) {
            case 'c':
                FREE(ipc_command);
                ipc_command = sstrdup(optarg);
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'h':
                printf("todo\n");
                break;
            default:
                printf("?? getopt returned %c\n", opt);
        }
    }

    if (!ipc_command || delay == -1) {
        printf("You need to specify --ipc-command and --delay!\n");
        return EXIT_FAILURE;
    }

    /* NOTE that we cannot produce any output before taking the screenshots —
     * that results in a race condition. Even though we can force stdout to be
     * flushed using fflush(stdout), the terminal emulator will draw the text a
     * bit later, so our first screenshot does not contain the output, but the
     * second one will, tampering with our test. */

    init_x_and_imlib();
    Imlib_Image first_shot, second_shot;

    /* 1: Take a screenshot */
    first_shot = imlib_create_image_from_drawable(0, 0, 0, 1920, 800, 1);

    /* 2: Send the specified IPC command to i3 */
    char *socket_path = socket_path_from_x11();
    int fd = ipc_connect(socket_path);
    if (ipc_send_message(fd, strlen(ipc_command), I3_IPC_MESSAGE_TYPE_COMMAND,
                         (uint8_t*)ipc_command) == -1)
        err(EXIT_FAILURE, "IPC: write()");

    /* 3: Sleep for the specified interval */
    usleep(delay * 1);

    /* 4: Take another screenshot */
    second_shot = imlib_create_image_from_drawable(0, 0, 0, 1920, 800, 1);

    /* 5: Compare the screenshots */
    imlib_context_set_image(first_shot);
    uint32_t *data = imlib_image_get_data();

    imlib_context_set_image(second_shot);
    uint32_t *ldata = imlib_image_get_data();
    int len = imlib_image_get_width() * imlib_image_get_height();

    bool shots_equal = (memcmp(data, ldata, len) == 0);
    if (shots_equal)
        printf("same images\n");
    else printf("some differences\n");

    /* 6: Save both screenshots for debugging */
    // TODO

    /* We signal equality using exit code 2 and inequality with exit code 3,
     * since 0 and 1 already have a well-defined meaning. */
    return (shots_equal ? 2 : 3);
}
