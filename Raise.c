/* $XConsortium: Raise.c,v 1.2 88/09/06 17:28:38 jim Exp $ */
 
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
 * 001 -- R. Kittell, DEC Storage A/D May 19, 1986
 *  Optionally warp mouse to upper right corner of window after raise.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 *  Western Software Lab. Convert to X11.
 */
 
#ifndef lint
static char *sccsid = "@(#)Raise.c	3.8	1/24/86";
#endif
 
#include "uwm.h"
 
Bool Raise(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                           /* Button event detail. */
int x, y;                               /* Event mouse position. */
 
{
XWindowAttributes winfo;			/* Window details for warp */
int status;
 
    /*
     * If the window is not the root window, raise the window and return.
     */
    if (window != RootWindow(dpy, scr)) {
        XRaiseWindow(dpy, window);
	/*
	 * Optionally warp the mouse to the upper left corner of the window.
	 */
	if (WarpOnRaise) {
	    status = XGetWindowAttributes (dpy, window, &winfo);
            if (status == FAILURE) return(FALSE);
 
	    status = XWarpPointer (dpy, None, window, 
	    		 0, 0, 0, 0,
			 winfo.width >= 7 ? winfo.width - 7 : winfo.width / 2,
                         winfo.height >= 10 ? 10 : winfo.height / 2);
            if (status == FAILURE) return(FALSE);
        }
    }
    return(FALSE);
}
