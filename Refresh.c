#ifndef lint
static char *rcsid_Refresh_c = "$XConsortium: Refresh.c,v 1.5 88/09/06 17:28:39 jim Exp $";
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
static char *sccsid = "@(#)Refresh.c	3.8	1/24/86";
#endif

#include "uwm.h"

Bool Refresh(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    return( Redraw(RootWindow(dpy,scr), mask, button, x, y) );
}



Bool Redraw(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XWindowAttributes winfo;		/* window info. */
    Window w;				/* Refresh window. */

    /*
     * Get info on the target window.
     */
    status = XGetWindowAttributes(dpy, window, &winfo);
    if (status == FAILURE) Error("Refresh -> Can't query target window.");

    /*
     * Create and map a window which covers the target window, then destroy it.
     */
    if ((w = XCreateSimpleWindow(dpy, window, 0, 0, 
    			   winfo.width, winfo.height, 0,
                           (Pixmap) 0, (Pixmap) 0)) == NULL)
        Error("Refresh -> Can't create refresh window.");
    XMapWindow(dpy, w);
    XDestroyWindow(dpy, w);
    XFlush(dpy);

    return(FALSE);
}
