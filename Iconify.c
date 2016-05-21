/* $XConsortium: Iconify.c,v 1.3 88/09/06 17:28:08 jim Exp $ */
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
 * 001 -- R. Kittell, DEC Storage A/D May 20, 1986
 *  Add optional warp of mouse to the upper right corner on de-iconify,
 *  and to the icon center on iconify.
 * 002 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
 *  Western Software Lab. Port to X11.
 */
 
#ifndef lint
static char *sccsid = "%W%	%G%";
#endif
 
#include "uwm.h"
 
Bool Iconify(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    XWindowAttributes window_info;	/* Event window info. */
    XWindowAttributes assoc_info;	/* Associated window info. */
    int mse_x, mse_y;			/* Mouse X and Y coordinates. */
    int sub_win_x, sub_win_y;		/* relative Mouse coordinates. */
    int icon_x, icon_y;			/* Icon U. L. X and Y coordinates. */
    int icon_w, icon_h;			/* Icon width and height. */
    int icon_bdr;			/* Icon border width. */
    int num_vectors;			/* Number of vectors in zap buffer. */
    int mmask;				/* Mouse state */
    Window root;			/* Mouse root window. */
    Window assoc;			/* Associated window. */
    Window sub_win;			/* Mouse position sub-window. */
    XSegment zap[MAX_ZAP_VECTORS];	/* Zap effect vertex buffer. */
    Bool iconifying;			/* Are we iconifying? */
 
    /*
     * Do not try to iconify the root window.
     */
    if (window == RootWindow(dpy, scr))
        return(FALSE);
 
    /*
     * Clear the vector buffer.
     */
    if (Zap) bzero(zap, sizeof(zap));
 
    /*
     * Get the mouse cursor position in case we must put a new
     * icon there.
     */
    XQueryPointer(dpy, RootWindow(dpy, scr), &root, &sub_win, 
    	&mse_x, &mse_y, &sub_win_x, &sub_win_y, &mmask);
 
    /*
     * Gather info about the window.
     */
    status = XGetWindowAttributes(dpy, window, &window_info);
    if (status == FAILURE) return(FALSE);
 
    /*
     * If the event window is an icon, de-iconify it and return.
     */
    if (IsIcon(window, mse_x, mse_y, TRUE, &assoc)) {
 
 
        /*
         * Gather info about the assoc window.
         */
        status = XGetWindowAttributes(dpy, assoc, &assoc_info);
        if (status == FAILURE) return(FALSE);
 
        /*
         * Store the zap vector buffer.
         */
        if (Zap) {
                num_vectors = StoreZap(
                    zap,
                    assoc_info.x - 1,
                    assoc_info.y - 1,
                    assoc_info.x + assoc_info.width +
                        (assoc_info.border_width << 1),
                    assoc_info.y + assoc_info.height +
                        (assoc_info.border_width << 1),
                    window_info.x - 1,
                    window_info.y - 1,
                    window_info.x + window_info.width +
                        (window_info.border_width << 1),
                    window_info.y + window_info.height +
                        (window_info.border_width << 1));
        }
 
        /*
         * Map the window and synchronize.
         */
        XMapRaised(dpy, assoc);
	XRemoveFromSaveSet(dpy, assoc);
 
            if (Zap) {
                /*
                 * Draw the zap lines.
                 */
                DrawZap();
            }
 
        /*
         * Unmap the icon window.
         */
        XUnmapWindow(dpy, window);
 
	/*
	 * Optionally warp the mouse to the upper right corner of the
         *  window.
         */
        if (WarpOnDeIconify) {
	    status = XGetWindowAttributes (dpy, assoc, &assoc_info);
            if (status == FAILURE) return(FALSE);
	    status = XWarpPointer (dpy, None, assoc, 
	    			 0, 0, 0, 0,
				 assoc_info.width >= 7 ?
                                 assoc_info.width - 7 : assoc_info.width / 2,
                                 assoc_info.height >= 10 ? 10 :
                                 assoc_info.height / 2);
            if (status == FAILURE) return(FALSE);
	}
    }
    else {
        /*
         * We have an icon window; all we have to do is
         * map it.
         */
        status = XGetWindowAttributes(dpy, assoc, &assoc_info);
        if (status == FAILURE) return(FALSE);
        icon_x = assoc_info.x;
        icon_y = assoc_info.y;
        icon_w = assoc_info.width;
        icon_h = assoc_info.height;
	icon_bdr = assoc_info.border_width;
 
        if (Zap) {
        /*
         * Store the zap effect vectors.
         */
        num_vectors = StoreZap(
            zap,
            window_info.x - 1,
            window_info.y - 1,
            window_info.x + window_info.width + (window_info.border_width << 1),
            window_info.y + window_info.height + (window_info.border_width << 1),
            icon_x - 1,
            icon_y - 1,
            icon_x + icon_w + (icon_bdr << 1),
            icon_y + icon_h + (icon_bdr << 1)
        );
        }
 
        /*
         * Map the icon window.
         */
        XMapWindow(dpy, assoc);
 
        if (Zap) {
        /*
         * Draw zap lines from the window to its icon.
         */
            DrawZap();
            DrawZap();
        }
 
        /*
         * Unmap the event window.
         */
	XAddToSaveSet(dpy, window);
        XUnmapWindow(dpy, window);
 
	/*
	 * Optionally warp the mouse to the icon center.
         */
        if (WarpOnIconify) {
            status = XGetWindowAttributes(dpy, assoc, &assoc_info);
            if (status == FAILURE) return(FALSE);
	    status = XWarpPointer (dpy, None, assoc, 
	    			 0, 0, 0, 0,
				 assoc_info.width / 2, assoc_info.height / 2);
            if (status == FAILURE) return(FALSE);
	}
 
    }
    return(TRUE);
}
