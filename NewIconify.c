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
 *  Western Software Lab. Port to X11.
 */

#ifndef lint
static char *sccsid = "%W%	%G%";
#endif

#include "uwm.h"

Bool NewIconify(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XWindowAttributes window_info;	/* Event window info. */
    XWindowAttributes icon_info;	/* Icon window info. */
    char *name;				/* Event window name. */
    int mse_x, mse_y;			/* Mouse X and Y coordinates. */
    int icon_x, icon_y;			/* Icon U. L. X and Y coordinates. */
    int icon_w, icon_h;			/* Icon width and height. */
    int icon_bdr;			/* Icon border width. */
    int prev_x;				/* Previous event window X location. */
    int prev_y;				/* Previous event window Y location. */
    int cur_x;				/* Current event window X location. */
    int cur_y;				/* Current event window Y location. */
    int root_x;				/* Root window X location. */
    int root_y;				/* Root window Y location. */
    int ulx, uly;			/* Event window upper left X and Y. */
    int lrx, lry;			/* Event window lower right X and Y. */
    int init_ulx, init_uly;		/* Init window upper left X and Y. */
    int init_lrx, init_lry;		/* Init window lower right X and Y. */
    int num_vectors;			/* Number of vectors in box. */
    int status;				/* Routine call return status. */
    int ptrmask;			/* pointer query state. */
    Window root;			/* Mouse root window. */
    Window icon;			/* Icon window. */
    Window sub_win;			/* Mouse position sub-window. */
    XEvent button_event;		/* Button event packet. */
    XSegment box[MAX_BOX_VECTORS];	/* Box vertex buffer. */
    XSegment zap[MAX_ZAP_VECTORS];	/* Zap effect vertex buffer. */
    Bool iconifying;			/* Are we iconifying? */

    /*
     * Do not lower or iconify the root window.
     */
    if (window == RootWindow(dpy, scr))
        return(FALSE);

    /*
     * Change the cursor to the icon cursor.
     */
    XChangeActivePointerGrab(dpy, EVENTMASK, ArrowCrossCursor, CurrentTime);

    /*
     * Clear the vector buffers.
     */
    bzero(box, sizeof(box));
    if (Zap) bzero(zap, sizeof(zap));
    
    /*
     * Get info on the event window.
     */
    status = XGetWindowAttributes(dpy, window, &window_info);
    if (status == FAILURE) return(FALSE);

    /*
     * Are we iconifying or de-iconifying?
     */
    if (!IsIcon(window, x, y, FALSE, &icon)) {

        /*
         * Window => Icon (Iconifying).
         */
        /*
         * If an icon window doesn't exist for the event window, then
         * make one.
         */
        iconifying = TRUE;
    }
    else {

        /*
         * Icon => Window (DeIconifying).
         */

        /*
         * We call the normal window the "icon" window only to simplify
         * the code later on in the function.
         */
        iconifying = FALSE;
    }
    /*
     * Get info on the icon window.
     */
    status = XGetWindowAttributes(dpy, icon, &icon_info);
    if (status == FAILURE) return(FALSE);

    /*
     * Determine the height, width, and borderwidth of the icon.
     */
    icon_h = icon_info.height;
    icon_w = icon_info.width;
    icon_bdr = icon_info.border_width;


    /*
     * Initialize the movement variables.
     */
    init_ulx = ulx = x - (icon_w >> 1) - icon_bdr;
    init_uly = uly = y - (icon_h >> 1) - icon_bdr;
    init_lrx = lrx = x + (icon_w >> 1) + icon_bdr - 1;
    init_lry = lry = y + (icon_h >> 1) + icon_bdr - 1;
    prev_x = x;
    prev_y = y;


    /*
     * Store the box.
     */
    if (Grid)
        num_vectors = StoreGridBox(box, ulx, uly, lrx, lry);
    else num_vectors = StoreBox(box, ulx, uly, lrx, lry);

    /*
     * Freeze the server, if requested by the user.
     * This results in a solid box instead of a flickering one.
     */
    if (Freeze)
        XGrabServer(dpy);

    /*
     * Process any outstanding events before drawing the box.
     */
    while (QLength(dpy) > 0) {
        XPeekEvent(dpy, &button_event);
        if (((XAnyEvent *)&button_event)->window == RootWindow(dpy, scr) &&
	    (button_event.type == ButtonPress || 
	     button_event.type == ButtonRelease))
            break;
        GetButton(&button_event);
    }

    /*
     * Draw the box.
     */
    DrawBox();
    if (Freeze)
        Frozen = window;

    /*
     * We spin our wheels here looking for mouse movement or a change
     * in the status of the buttons.
     */
    while (TRUE) {

	/*
	 * Check to see if we have a change in mouse button status.
	 * This is how we get out of this "while" loop.
	 */
	if (XPending(dpy) && !ProcessRequests(box, num_vectors) && GetButton(&button_event)) {
            /*
             * Process the pending events, this sequence is the only
             * way out of the loop and the routine.
             */

            if ((button_event.type != ButtonPress) && 
	        (button_event.type != ButtonRelease)) {
                continue; /* spurious menu event... */
            }

            /*
             * If we froze the server, then erase the last lines drawn.
             */
            if (Freeze) {
                DrawBox();
                Frozen = (Window)0;
                XUngrabServer(dpy);
            }

            /*
             * Save the mouse cursor location.
             */
	    if (button_event.type == ButtonPress ||
	        button_event.type == ButtonRelease) {
	    	mse_x = ((XButtonEvent *)&button_event)->x;
	    	mse_y = ((XButtonEvent *)&button_event)->y;
	    	break;
	    }
	}
	else {
	    /*
	     * Continue to track the mouse until we get a change
             * in button status.
	     */
            XQueryPointer(dpy, RootWindow(dpy, scr), 
	    	&root, &sub_win, &root_x, &root_y, &cur_x, &cur_y, 
		&ptrmask);

            /*
             * If the mouse has moved, then make sure the box follows it.
             */
            if ((cur_x != prev_x) || (cur_y != prev_y)) {

                /*
                 * If we've frozen the server, then erase the old box first!
                 */
                if (Freeze)
                    DrawBox();
    
                /*
                 * Set the new box position.
                 */
                ulx += cur_x - prev_x;
                uly += cur_y - prev_y;
                lrx += cur_x - prev_x;
                lry += cur_y - prev_y;
    
                /*
                 * Calculate the vectors for the new box.
                 */
                if (Grid)
                    num_vectors = StoreGridBox(box, ulx, uly, lrx, lry);
                else num_vectors = StoreBox(box, ulx, uly, lrx, lry);
    
                /*
                 * Draw the new box.
                 */
                if (Freeze)
                    DrawBox();
            }

            /*
             * Save the old box position.
             */
            prev_x = cur_x;
            prev_y = cur_y;

            /*
             * If server is not frozen, then draw the "flicker" box.
             */
            if (!Freeze) {
                DrawBox();
                DrawBox();
            }
	}
    }

    /*
     * If the button is not a button release of the same button pressed,
     * then abort the operation.
     */
    if ((button_event.type != ButtonRelease) ||
        (((XButtonReleasedEvent *)&button_event)->button != button)) {
	    ResetCursor(button);
	    return(TRUE);
    }

    /*
     * If we are here we have committed to iconifying/deiconifying.
     */

    /*
     * Determine the coordinates of the icon or window;
     * normalize the window or icon coordinates if the user so desires.
     */
    icon_x = mse_x - (icon_w >> 1) - icon_bdr;
    icon_y = mse_y - (icon_h >> 1) - icon_bdr;
    if ((NIcon && iconifying) || (NWindow && !iconifying)) {
        if (icon_x < 0) icon_x = 0;
        if (icon_y < 0) icon_y = 0;
        if ((icon_x - 1 + icon_w + (icon_bdr << 1)) > ScreenWidth) {
            icon_x = ScreenWidth - icon_w - (icon_bdr << 1) + 1;
        }
        if ((icon_y - 1 + icon_h + (icon_bdr << 1)) > ScreenHeight) {
            icon_y = ScreenHeight - icon_h - (icon_bdr << 1) + 1;
        }
    }

    /*
     * Move the window into place.
     */
    XMoveWindow(dpy, icon, icon_x, icon_y);

    /*
     * Map the icon window.
     */
    XMapRaised(dpy, icon);
    if (!iconifying) XRemoveFromSaveSet(dpy, icon);

    if (Zap) {
        num_vectors = StoreZap(zap, window_info.x, window_info.y,
                               window_info.x + window_info.width +
                               (window_info.border_width << 1),
                               window_info.y + window_info.height +
                               (window_info.border_width << 1),
                               ulx, uly, lrx, lry);
        DrawZap();
        DrawZap();
    }

    /*
     * Unmap the event window.
     */
    if (iconifying) XAddToSaveSet(dpy, window);
    XUnmapWindow(dpy, window);
    return(TRUE);
}
