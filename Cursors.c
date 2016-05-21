#ifndef lint
static char *rcsid_StoreCursors_c = "$XConsortium: Cursors.c,v 1.6 88/09/06 17:27:45 jim Exp $";
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
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 *        Convert to X11
 */

#ifndef lint
static char *sccsid = "@(#)StoreCursors.c	3.8	1/24/86";
#endif

#include "uwm.h"
#include <X11/cursorfont.h>

/*
 * Store all the cursors into global variables.
 */
StoreCursors()
{
    /*
     * Main uwm cursor and movement cursor.
     */
    ArrowCrossCursor = XCreateFontCursor(dpy, XC_fleur);
    if (ArrowCrossCursor == FAILURE) {
	Error("StoreCursors -> Unable to store ArrowCrossCursor.");
    }

    /*
     * Text cursor used in icons.
     */
    TextCursor = XCreateFontCursor(dpy, XC_top_left_arrow);
    if (TextCursor == FAILURE) {
	Error("StoreCursors -> Unable to store TextCursor.");
    }

    /*
     * Icon cursor used to iconify windows.
     */
    IconCursor = XCreateFontCursor(dpy, XC_icon);	
    if (IconCursor == FAILURE) {
	Error("StoreCursors -> Unable to store IconCursor.");
    }

    /*
     * Menu cursor.
     */
    MenuCursor = XCreateFontCursor(dpy, XC_sb_left_arrow);
    if (MenuCursor == FAILURE) {
	Error("StoreCursors -> Unable to store MenuCursor.");
    }

    /*
     * Left button main cursor.
     */
    LeftButtonCursor = XCreateFontCursor(dpy, XC_leftbutton);	
    if (LeftButtonCursor == FAILURE) {
	Error("StoreCursors -> Unable to store LeftButtonCursor.");
    }

    /*
     * Middle button main cursor.
     */
    MiddleButtonCursor = XCreateFontCursor(dpy, XC_middlebutton);	
    if (MiddleButtonCursor == FAILURE) {
	Error("StoreCursors -> Unable to store MiddleButtonCursor.");
    }

    /*
     * Right button main cursor.
     */
    RightButtonCursor = XCreateFontCursor(dpy, XC_rightbutton);	
    if (RightButtonCursor == FAILURE) {
	Error("StoreCursors -> Unable to store RightButtonCursor.");
    }

    /*
     * Targer cursor used to identify a window for an action.
     */
    TargetCursor = XCreateFontCursor(dpy, XC_hand2);	
    if (TargetCursor == FAILURE) {
	Error("StoreCursors -> Unable to store TargetCursor.");
    }
}

