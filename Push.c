#ifndef lint
static char *rcsid_Push_c = "$XConsortium: Push.c,v 1.4 88/09/06 17:28:36 jim Exp $";
#endif	lint

#include <X11/copyright.h>

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 *
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

 

/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
    Western Software Lab. Convert to X11.
 */

#ifndef lint
static char *sccsid = "@(#)Push.c	3.8	1/24/86";
#endif

#include "uwm.h"

#define PUSH_DOWN	1
#define PUSH_UP		2
#define PUSH_LEFT	3
#define PUSH_RIGHT	4

extern Bool PushAll();

Bool PushDown(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
    return(PushAll(window, PUSH_DOWN));
}

Bool PushUp(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
    return(PushAll(window, PUSH_UP));
}

Bool PushLeft(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
    return(PushAll(window, PUSH_LEFT));
}

Bool PushRight(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
    return(PushAll(window, PUSH_RIGHT));
}

Bool PushAll(w, direction)
Window w;
int direction;
{
    XWindowAttributes winfo;			/* Event window information. */
    XWindowAttributes rinfo;			/* Root window information. */
    int xofs, yofs;			/* Movement offsets. */
    int x, y;				/* New window position. */

    /*
     * Do not try to move the root window.
     */
    if (w == RootWindow(dpy, scr))
        return(FALSE);

    /*
     * Gather info on the event window.
     */
    status = XGetWindowAttributes(dpy, w, &winfo);
    if (status == FAILURE) return(FALSE);

    /*
     * Calculate the movement offsets.
     */
    switch(direction) {
    case PUSH_DOWN:
        xofs = 0;
        yofs = Push ? (winfo.height / Pushval) : Pushval;
        break;
    case PUSH_UP:
        xofs = 0;
        yofs = 0 - (Push ? (winfo.height / Pushval) : Pushval);
        break;
    case PUSH_LEFT:
        xofs = 0 - (Push ? (winfo.width / Pushval) : Pushval);
        yofs = 0;
        break;
    case PUSH_RIGHT:
        xofs = Push ? (winfo.width / Pushval) : Pushval;
        yofs = 0;
        break;
    }

    /*
     * Calculate the new window position.
     */
    x = winfo.x + xofs;
    y = winfo.y + yofs;

    /*
     * Normalize the new window coordinates so we don't
     * lose the window off the edge of the screen.
     */
    if (x < (0 - winfo.width + CURSOR_WIDTH - (winfo.border_width << 1)))
        x = 0 - winfo.width + CURSOR_WIDTH - (winfo.border_width << 1);
    if (y < (0 - winfo.height + CURSOR_HEIGHT - (winfo.border_width << 1)))
        y = 0 - winfo.height + CURSOR_HEIGHT - (winfo.border_width << 1);
    if (x > (ScreenWidth - CURSOR_WIDTH))
        x = ScreenWidth - CURSOR_WIDTH;
    if (y > (ScreenHeight - CURSOR_HEIGHT))
        y = ScreenHeight - CURSOR_HEIGHT;

    /*
     * Move the window into place.
     */
    XMoveWindow(dpy, w, x, y);

    return(FALSE);
}
