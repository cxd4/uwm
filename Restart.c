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

/* $XConsortium: copyright.h,v 1.5 89/12/22 16:11:28 rws Exp $ */
/*

Copyright 1985, 1986, 1987, 1988, 1989 by the
Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

/*
 * MODIFICATION HISTORY
 *
 * 000 -- M. Gancarz, DEC Ultrix Engineering Group
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
    Western Software Lab. Convert to X11.
 */

#ifndef lint
static char *sccsid = "%W%	%G%";
#endif

#include <stdlib.h>
#include "uwm.h"

Bool Restart(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XButtonEvent button_event;
    int percentage;
    int i;

    percentage = VOLUME_PERCENTAGE(Volume);
    XBell(dpy, percentage);
    XBell(dpy, percentage);
    XSync (dpy, 0);

    /*
     * Gag, rely on operating system to close connection because we don't
     * to mess ourselves up in case the exec fails.
     */
#if 0
    execvp(*Argv, Argv, Environ);
#else
    execvp(*Argv, Argv);
#endif
    for (i = 0; i < 4; i++) {
	XBell(dpy, percentage);
	percentage += 10;
	if (percentage > 100) percentage = 100;
    }
    XSync (dpy, 0);
    fprintf(stderr, "%s:  Restart failed!\n", *Argv);
}

Bool Quit(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    exit(0);
}
