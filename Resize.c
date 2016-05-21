#ifndef lint
static char *rcsid_Resize_c = "$XConsortium: Resize.c,v 1.33 89/01/10 12:13:13 jim Exp $";
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
static char *sccsid = "@(#)Resize.c	3.8	1/24/86";
#endif

#include "uwm.h"
#include <X11/Xatom.h>

#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define min(a,b) ( (a) > (b) ? (b) : (a) )
#define makemult(a, b) ((b==1) ? (a) : (((int)((a) / (b))) * (b)) )

#define LARGEST_DIMENSION 0xffff	/* largest unsigned 16 bit number */

Bool Resize(window, mask, button, x0, y0)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x0, y0;                             /* Event mouse position; root coords */
{
    XWindowAttributes window_info;	/* Event window info. */
    int x1, y1;                         /* fixed box corner   */
    int x2, y2;                         /* moving box corner   */
    int x, y;
    int xinc, yinc;
    int minwidth, minheight;
    int maxwidth, maxheight;
    int defwidth, defheight;
    int ox, oy;				/* which quadrant of window */
    int pop_x, pop_y;			/* location of pop window */
    int hsize, vsize;			/* dynamic size */
    int delta;				
    int root_x, root_y;			/* root window coordinates */
    int ptrmask;			/* pointer status word */
    int num_vectors;			/* Number of vectors to XDraw. */
    Window assoc;			/* Window represented by the icon. */
    Window sub_win;			/* Mouse query sub window. */
    Window root;			/* Root query window. */
    XEvent button_event;		/* Button event packet. */
    XSegment box[MAX_BOX_VECTORS];	/* Box drawing vertex buffer. */
    XSegment zap[MAX_ZAP_VECTORS];	/* Zap drawing vertex buffer. */
    Bool stop;				/* Should the window stop changing? */
    XSizeHints sizehints;
    XWindowChanges values;
    int width_offset, height_offset;	/* to subtract if resize increments */
    int x_offset, y_offset;		/* add to pointer to get anchor */
    int two_borders;			/* border_width * 2 */
    int bw;				/* shorthand for border_width */
    int Rhsize, Rvsize;			/* real versions of size */

    /*
     * Do nothing if the event window is the root window.
     */
    if (window == RootWindow(dpy, scr))
        return(FALSE);

    /*
     * Gather info about the event window.
     */
    status = XGetWindowAttributes(dpy, window, &window_info);
    if (status == FAILURE) return(FALSE);

    /*
     * Do not resize an icon window (NULL for assoc means don't create too). 
     */
    if (IsIcon(window, 0, 0, FALSE, NULL))
        return(FALSE);

    /*
     * (x0,y0) are in root window coordinates and correspond to where the user
     * pressed the button that started this action.  window_info.(x,y) are
     * in root coordinates and represent the upper left, OUTER corner of the
     * window.  we really want to allow the user to drag out the window WITH
     * its border.
     */

    bw = window_info.border_width;
    two_borders = (bw * 2);

    /*
     * Clear the vector buffers.
     */
    bzero(box, sizeof(box));
    if (Zap) bzero(zap, sizeof(zap));

    /*
     * If we are here then we have a resize operation in progress.
     */

    /*
     * Turn on the resize cursor.
     */
    XChangeActivePointerGrab(dpy, EVENTMASK, ArrowCrossCursor, CurrentTime);

    /*
     * calculate fixed point (x1, y1) and varying point (x2, y2) for the
     * inside of the window
     */

    hsize = defwidth = window_info.width; 
    vsize = defheight = window_info.height;
    x1 = window_info.x + window_info.border_width;
    y1 = window_info.y + window_info.border_width;
    x2 = x1 + hsize;
    y2 = y1 + vsize;

    /*
     * Get the event window resize hint.
     */
    sizehints.flags = 0;
    XGetSizeHints(dpy, window, &sizehints, XA_WM_NORMAL_HINTS); 
    CheckConsistency(&sizehints);

    /* until there are better WM_HINTS, we'll assume that the client's
     * minimum width and height are the appropriate offsets to subtract
     * when resizing with an explicit resize increment.
     */
    if (sizehints.flags&PMinSize && sizehints.flags&PResizeInc) {
        width_offset = sizehints.min_width;
        height_offset = sizehints.min_height;
    } else
        width_offset = height_offset = 0;

    /*
     * decide what resize mode we are in. Always rubberband if window
     * is too small.
     */
    if (window_info.width > 2 && window_info.height > 2) {
      ox = ((x0 - window_info.x) * 3) / (window_info.width + two_borders);
      oy = ((y0 - window_info.y) * 3) / (window_info.height + two_borders);
      if ((ox + oy) & 1) {
	if (ox & 1) {
	    /* fix up size hints so that we will never change width */
	    sizehints.min_width = sizehints.max_width = window_info.width;
	    if ((sizehints.flags&PMinSize) == 0) {
	      sizehints.min_height = 0;
	      sizehints.flags |= PMinSize;
	    }
	    if ((sizehints.flags&PMaxSize) == 0) {
	      sizehints.max_height = LARGEST_DIMENSION;
	      sizehints.flags |= PMaxSize;
	    }
	}
	if (oy & 1) {
	    /* fix up size hints so that we will never change height */
	    sizehints.min_height = sizehints.max_height = window_info.height;
	    if ((sizehints.flags&PMinSize)==0) {
	      sizehints.min_width = 0;
	      sizehints.flags |= PMinSize;
	    }
	    if ((sizehints.flags&PMaxSize)==0) {
	      sizehints.max_width = LARGEST_DIMENSION;
	      sizehints.flags |= PMaxSize;
	    }
	}
      }
    }
    else ox = oy = 2;

    /*
     * bump locations and sizes to get outside dimensions from inside
     */

    x1 -= bw;
    y1 -= bw;
    x2 += bw;
    y2 += bw;
    hsize += two_borders;
    vsize += two_borders;
    width_offset += two_borders;
    height_offset += two_borders;

    /* change fixed point to one that shouldn't move */
    if (oy == 0) { 
	y = y1; y1 = y2; y2 = y;
    }
    if (ox == 0) { 
	x = x1; x1 = x2; x2 = x;
    }

    if (sizehints.flags&PMinSize) {
        minwidth = sizehints.min_width;
        minheight = sizehints.min_height;
    } else {
        minwidth = 0;
        minheight = 0;
    }
    minwidth += two_borders;
    minheight += two_borders;

    if (sizehints.flags&PMaxSize) {
        maxwidth = max(sizehints.max_width + two_borders, minwidth);
        maxheight = max(sizehints.max_height + two_borders, minheight);
    } else {
	maxwidth = LARGEST_DIMENSION;
	maxheight = LARGEST_DIMENSION;
    }

    if (sizehints.flags&PResizeInc) {
        xinc = sizehints.width_inc;
        yinc = sizehints.height_inc;
    } else {
        xinc = 1;
        yinc = 1;
    }

    switch (ox) {
        case 0: pop_x = x1 - PWidth; break;
        case 1: pop_x = x1 + (hsize-PWidth)/2; break;
        case 2: pop_x = x1; break;
    }
    switch (oy) {
        case 0: pop_y = y1 - PHeight; break;
        case 1: pop_y = y1 + (vsize-PHeight)/2; break;
        case 2: pop_y = y1; break;
     }
    values.x = pop_x;
    values.y = pop_y;
    values.stack_mode = Above;
    XConfigureWindow(dpy, Pop, CWX|CWY|CWStackMode, &values);
    XMapWindow(dpy, Pop);

    if (Grid) {
    	num_vectors = StoreGridBox(
	    box,
	    MIN(x1, x2), MIN(y1, y2),
	    MAX(x1, x2), MAX(y1, y2)
	);
    }
    else {
    	num_vectors = StoreBox(
	    box,
	    MIN(x1, x2), MIN(y1, y2),
	    MAX(x1, x2), MAX(y1, y2)
	);
    }

    /*
     * If we freeze the server, then we will draw solid
     * lines instead of flickering ones during resizing.
     */
    if (Freeze) XGrabServer(dpy);

    /*
     * Process any pending exposure events before drawing the box.
     */
    while (QLength(dpy) > 0) {
        XPeekEvent(dpy, &button_event);
        if (((XAnyEvent *)&button_event)->window == RootWindow(dpy, scr) &&
	    (button_event.type == ButtonPress || 
	     button_event.type == ButtonRelease))
             break;
        GetButton(&button_event);
    }

    if (ResizeRelative) {
	x_offset = x2 - x0;
	y_offset = y2 - y0;
    } else {
	x_offset = y_offset = 0;
    }

    /*
     * Now draw the box.
     */
    DrawBox();
    Frozen = window;

    stop = FALSE;
    x = -1; y = -1;

    while (!stop) {
	if (x != x2 || y != y2) {
	
	    x = x2; y = y2;
	    
            /*
             * If we've frozen the server, then erase
             * the old box.
             */
            if (Freeze)
                DrawBox();

	    if (Grid) {
	    	num_vectors = StoreGridBox(
		    box,
		    MIN(x1, x), MIN(y1, y),
		    MAX(x1, x), MAX(y1, y)
		);
	    }
	    else {
	    	num_vectors = StoreBox(
		    box,
		    MIN(x1, x), MIN(y1, y),
		    MAX(x1, x), MAX(y1, y)
		);
	    }

            if (Freeze)
                DrawBox();

	    {
	        int Hsize = (hsize - width_offset) / xinc;
		int Vsize = (vsize - height_offset) / yinc;
		int pos = 5;
		int n, digits;

		n = digits = 0;
		if (Hsize > 999) {
		    PText[0] = (n = (Hsize / 1000)) + '0';
		    Hsize -= n * 1000; n = 0; digits++;
		} else {
		    PText[0] = ' ';
		}
		if (Hsize > 99 || digits) {
		    PText[1] = (n = (Hsize / 100)) + '0';
		    Hsize -= n * 100; n = 0; digits++;
		} else {
		    PText[1] = ' ';
		}
		if (Hsize > 9 || digits) {
		    PText[2] = (n = (Hsize / 10)) + '0';
		    Hsize -= n * 10; n = 0; digits++;
		} else {
		    PText[2] = ' ';
		}
		PText[3] = Hsize + '0';

		n = digits = 0;
		if (Vsize > 999) {
		    PText[pos++] = (n = (Vsize / 1000)) + '0';
		    Vsize -= n * 1000; n = 0; digits++;
		} 
		if (Vsize > 99 || digits) {
		    PText[pos++] = (n = (Vsize / 100)) + '0';
		    Vsize -= n * 100; n = 0; digits++;
		} 
		if (Vsize > 9 || digits) {
		    PText[pos++] = (n = (Vsize / 10)) + '0';
		    Vsize -= n * 10; n = 0; digits++;
		} 
		PText[pos++] = Vsize + '0';

		while (pos < PTEXT_LENGTH) PText[pos++] = ' ';
	    }

	    /*
	     * If the font is not fixed width we have to
	     * clear the window to guarantee that the characters
	     * that were there before are erased.
	     */
	    if (!(PFontInfo->per_char)) XClearWindow(dpy, Pop);
	    XDrawImageString(
	        dpy, Pop, PopGC,
	        PPadding, PPadding+PFontInfo->ascent,
	        PText, PTextSize);
	}

        if (!Freeze) {
            DrawBox();
            DrawBox();
        }

	if (XPending(dpy) && !ProcessRequests(box, num_vectors) && GetButton(&button_event)) {

            if ((button_event.type != ButtonPress) && 
	        (button_event.type != ButtonRelease)) {
                continue; /* spurious menu event... */
            }

            if (Freeze) {
                DrawBox();
                Frozen = (Window)0;
                XUngrabServer(dpy);
            }

	    if ((button_event.type == ButtonRelease) &&
                (((XButtonReleasedEvent *)&button_event)->button == button)){
		x2 = ((XButtonReleasedEvent *)&button_event)->x + x_offset;
		y2 = ((XButtonReleasedEvent *)&button_event)->y + y_offset;
		stop = TRUE;
	    }
	    else {
		XUnmapWindow(dpy, Pop);
		ResetCursor(button);
		return(TRUE);
	    }
	}
	else {
	    XQueryPointer(dpy, RootWindow(dpy, scr), &root, 
	    		&sub_win, &root_x, &root_y, &x2, &y2, &ptrmask);
	    x2 += x_offset;		/* get to anchor point */
	    y2 += y_offset;
	    root_x += x_offset;
	    root_y += y_offset;
	}


	hsize = max(min(abs (x2 - x1), maxwidth), minwidth);
	hsize = makemult(hsize-minwidth, xinc)+minwidth;
	Rhsize = hsize - two_borders;
 
	vsize = max(min(abs(y2 - y1), maxheight), minheight);
	vsize = makemult(vsize-minheight, yinc)+minheight; 
	Rvsize = vsize - two_borders;

	if (sizehints.flags & PAspect) {
            if ((Rhsize * sizehints.max_aspect.y > 
	          Rvsize * sizehints.max_aspect.x)) {
	       delta = makemult( 
			 (Rhsize*sizehints.max_aspect.y /
		         sizehints.max_aspect.x)
			  - Rvsize, 
		       yinc); 
	       if ((vsize+delta <= maxheight))  vsize += delta;
	       else {
	         delta = makemult(Rhsize - 
		     (sizehints.max_aspect.x * Rvsize/sizehints.max_aspect.y), 
		     xinc);
		 if (hsize-delta >= minwidth) hsize -= delta; 
	       }
            }  
            if (Rhsize * sizehints.min_aspect.y < Rvsize * 
		    sizehints.min_aspect.x) {
	       delta = makemult( 
		        (sizehints.min_aspect.x * 
			  Rvsize/sizehints.min_aspect.y) - Rhsize, 
	 	        xinc);
	       if (hsize+delta <= maxwidth) hsize += delta;
	       else {
	         delta = makemult(
		       Rvsize - 
		         (Rhsize*sizehints.min_aspect.y /
			 sizehints.min_aspect.x), 
		       yinc); 
	         if ((vsize-delta >= minheight))  vsize -= delta; 
	       }
	    }
 		  
      }
      if (ox == 0)
 	x2 = x1 - hsize;
      else
	x2 = x1 + hsize;

      if (oy == 0)
	y2 = y1 - vsize;
      else
    	y2 = y1 + vsize;
	    
    }
    if (x2 < x1) {
       x = x1; x1 = x2; x2 = x;
    }
    if (y2 < y1) {
        y = y1; y1 = y2; y2 = y;
    }
    if ((x1!=window_info.x) || (y1 != window_info.y) || 
        (Rhsize != window_info.width) ||
        (Rvsize != window_info.height)) {
        XMoveResizeWindow(dpy, window, x1, y1, Rhsize, Rvsize);
    }
    XUnmapWindow(dpy, Pop);
    return(TRUE);
}

CheckConsistency(hints)
XSizeHints *hints;
{
  if (hints->min_height < 0) hints->min_height = 0;
  if (hints->min_width < 0)  hints->min_width = 0;

  if (hints->max_height <= 0 || hints->max_width <= 0)
      hints->flags &= ~PMaxSize;

  hints->min_height = min(LARGEST_DIMENSION, hints->min_height);
  hints->min_width =  min(LARGEST_DIMENSION,  hints->min_width);

  hints->max_height = min(LARGEST_DIMENSION, hints->max_height);
  hints->max_width =  min(LARGEST_DIMENSION,  hints->max_width);

  if ((hints->flags&PMinSize) && (hints->flags&PMaxSize) && 
   ((hints->min_height > hints->max_height) ||
    (hints->min_width > hints->max_width)))
	hints->flags &= ~(PMinSize|PMaxSize);

  if ((hints->flags&PAspect) && 
   (hints->min_aspect.x * hints->max_aspect.y > 
     hints->max_aspect.x * hints->min_aspect.y))
	hints->flags &= ~(PAspect);
}
