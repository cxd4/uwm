#ifndef lint
static char *rcsid_MoveOpaque_c = "$XConsortium: MoveOpaque.c,v 1.12 88/10/22 15:20:54 jim Exp $";
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
static char *sccsid = "@(#)MoveOpaque.c	1.7	1/24/86";
#endif

#include "uwm.h"
#include <X11/Xatom.h>

Bool MoveOpaque(window, mask, button, x, y)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
{
    int prev_x, prev_y;			/* Previous mouse location. */
    int cur_x, cur_y;			/* Current mouse location. */
    int win_x, win_y;			/* Current window location. */
    int root_x;				/* Root window X location. */
    int root_y;				/* Root window Y location. */
    int ptrmask;			/* state of ptr when queried */
    XWindowAttributes window_info;	/* Event window information. */
    Window sub_window;			/* Query mouse event sub-window. */
    Window root;			/* Query mouse event root. */
    XEvent button_event;		/* Button event packet. */

    /*
     * Do not try to move the root window.
     */
    if (window == RootWindow(dpy, scr))
        return(FALSE);

    /*
     * Change the cursor.
     */
    XChangeActivePointerGrab(dpy, EVENTMASK, ArrowCrossCursor, CurrentTime);

    /*
     * Gather info on the event window.
     */
    status = XGetWindowAttributes(dpy, window, &window_info);
    if (status == FAILURE) return(FALSE);

    /*
     * Initialize movement variables.
     */
    prev_x = cur_x = x;
    prev_y = cur_y = y;
    win_x = window_info.x;
    win_y = window_info.y;

    /*
     * Main loop.
     */
    while (TRUE) {

        XQueryPointer(dpy, RootWindow(dpy, scr), 
		&root, &sub_window, &root_x, &root_y, &cur_x, &cur_y, 
		&ptrmask);

    	/*
	 * Check to see if we have a change in mouse button status.
	 * This is how we get out of this "while" loop.
	 */
	if (XCheckMaskEvent(dpy,
			    ButtonPressMask|ButtonReleaseMask,
			    &button_event)) {
	    XButtonEvent *be = (XButtonEvent *) &button_event;

	    /*
	     * If the button event was something other than the
             * release of the original button pressed, then move the
             * window back to where it was originally.
	     */
            if ((button_event.type != ButtonRelease) || 
	        (be->button != button)) {
                ResetCursor(button);
		XMoveWindow(dpy, window, window_info.x, window_info.y);
	    } else {
		win_x += (be->x_root - prev_x);
		win_y += (be->y_root - prev_y);
		XMoveWindow (dpy, window, win_x, win_y);
	    }

            return(TRUE);
	}
	
        /*
         * Take care of all the little things that have changed; 
         * i.e., move the window, if necessary.
         */
        if ((cur_x != prev_x) || (cur_y != prev_y)) {
            win_x += (cur_x - prev_x);
            win_y += (cur_y - prev_y);
            XMoveWindow(dpy, window, win_x, win_y);
    	    prev_x = cur_x;
    	    prev_y = cur_y;
    	}
    }
}

