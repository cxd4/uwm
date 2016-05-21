/* $XConsortium: RubberBand.c,v 1.33 88/10/22 15:19:00 jim Exp $ */
/* derived from XCreateTerm.c */

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
 * 000 -- Loretta Guarino Reid, DEC Ultrix Engineering Group
 * 001 -- Ralph R. Swick, DEC/MIT Project Athena
 *	  tailor to uwm; use global resources created by uwm
 */

#include "uwm.h"
#include <X11/cursorfont.h>

#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define min(a,b) ( (a) > (b) ? (b) : (a) )
#define abs(a) ( (a) > 0 ? (a) : -(a))
#define makemult(a, b) ((b==1) ? (a) : (((int)((a) / (b))) * (b)) )

#define LARGEST_DIMENSION 0xffff	/* largest unsigned 16 bit number */

#define DCOUNT 2
#define PCOUNT 1 + (4 * 2 * DCOUNT)

#define BW	PBorderWidth		/* pop-up window border width */
#define IBW	PPadding		/* internal border width for pop-up */

AskUser(dpy, scr, window, x, y, width, height, hints)
	Display *dpy;
	int scr;
	Window window;
	int *x, *y, *width, *height;
	XSizeHints *hints;
	{
	/* XFontStruct *pfont; */
#define pfont	PFontInfo		/* text font for pop-up */
	Cursor ur, ul, ll, lr;          /* cursors for rubber banding    */
	int change_cursor = FALSE;
        int current_cursor;
        char *text;			/* text for prompt string       */
        int nz;                         /* count where zeros are        */
        int popw, poph;                 /* width and height of prompt window*/
        /* int pfore, pback; */         /* prompt foreground and background */
#define pbord	 PBorder
#define pback	 PTextBackground
#define pfore	 PTextForeground
	/* GC popGC, invertGC; */
	/* XGCValues xgc; */
#define popGC	 PopGC
#define invertGC DrawGC
        int x1, y1;                     /* location of mouse            */
        int x2, y2;                     /* other corner of box          */
        int rootx, rooty, mask;         /* for XQueryPointer          */
	Window root, subw;		/* for XQueryPointer */
        int xa = -1, ya = -1, xb = -1, yb = -1;
        int xinc = 1, yinc = 1;
        int minwidth, minheight;
        int maxwidth, maxheight;
        int defwidth, defheight;
        int chosen = -1;
        int stop;
        int changed;
        int dx, dy;
	int delta;
	XPoint box[PCOUNT];
        int hsize, vsize;
        XEvent e;                 /* someplace to put the event   */
        int events;                     /* what events we want.             */
        Window pop;                     /* pop up prompt window         */
	int i;
	char *name;
	Bool free_name;
	int width_offset, height_offset; /* to subtract if resize increments */
	XSetWindowAttributes attr;	/* for setting save unders flag */
	int border_width;		/* width of border around window */
	int two_borders;		/* twice border_width */
	int Rhsize, Rvsize, Rdx, Rdy;	/* real sizes */

	if (hints->flags & USPosition) {
	  *x = hints->x;
	  *y = hints->y;
	  *width = hints->width;
	  *height = hints->height;
          if (hints->flags & PMinSize) {
	      if (*width < hints->min_width) *width = hints->min_width;
	      if (*height < hints->min_height) *height = hints->min_height;
	  }
	  return;
	}

	free_name = False;		/* kludge for broken XFetchNames */
        if (!XFetchName(dpy, window, &name)) {	/* if failure */
	    name = "unnamed window";
	} else if (!name) {			/* if success but no name */
	    name = "";
	} else {				/* if has name */
	    free_name = True;
	}

	ur = XCreateFontCursor(dpy, XC_ur_angle);
	ul = XCreateFontCursor(dpy, XC_ul_angle);
	ll = XCreateFontCursor(dpy, XC_ll_angle);
	lr = XCreateFontCursor(dpy, XC_lr_angle);
	current_cursor = ul;

	events = ButtonPressMask | ButtonReleaseMask;
	/* pfont = XLoadQueryFont(dpy, "fixed"); */

	/* 
	 * go get the mouse as soon as you can 
	 */

	while (1) {
	        int res;
		if ((res = XGrabPointer (dpy, RootWindow(dpy, scr), FALSE, events, 
		  GrabModeAsync, GrabModeAsync, None, ul, CurrentTime )) == 
		    GrabSuccess)
		  break;
		sleep (1);
	}
	nz = strlen(name);		/* compute number of characters */
	text = (char *)malloc (nz + 13);
	(void) strcpy(text, name);
	(void) strcat(text, ": 0000x0000");
	nz += 11;
	popw = XTextWidth (pfont, text, nz) + 2 * IBW;
	poph = pfont->ascent+pfont->descent + 2 * IBW;

	/* pfore = WhitePixel(dpy, scr);
	 * pback = BlackPixel(dpy, scr);
         * xgc.foreground = pfore;
         * xgc.background = pback;
	 * xgc.font = pfont->fid;
	 * popGC = XCreateGC(dpy, RootWindow(dpy, scr),
	 *    GCForeground+GCBackground+GCFont, &xgc);
	 * xgc.function = GXinvert;
	 * xgc.subwindow_mode = IncludeInferiors;
	 * invertGC = XCreateGC(dpy, RootWindow(dpy, scr),
	 *   GCForeground+GCBackground+GCFont+GCFunction+GCSubwindowMode, &xgc);
	 */

	pop = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 
		0, 0, popw, poph, BW, pbord, pback);
	attr.save_under = True;
	attr.event_mask = (VisibilityChangeMask | ExposureMask);
	XChangeWindowAttributes (dpy, pop, (CWSaveUnder | CWEventMask), &attr);
	XMapWindow (dpy, pop);

	/*
	 * get the current size and border width to use as defaults
	 */
	{
	    Window root;
	    unsigned int dummyint;

	    XGetGeometry (dpy, window, &root, &dummyint, &dummyint,
			  &defwidth, &defheight, &border_width, &dummyint);
	    two_borders = border_width * 2;
	}

	if (hints->flags&PMinSize) {
	  minwidth = hints->min_width;
	  minheight = hints->min_height;
	} else {
	  minwidth = 0;
	  minheight = 0;
	}
	minwidth += two_borders;
	minheight += two_borders;

	if (hints->flags&PMaxSize) {
	  maxwidth = max(hints->max_width+two_borders, minwidth);
	  maxheight = max(hints->max_height+two_borders, minheight);
	} else {
	  maxwidth = LARGEST_DIMENSION;
	  maxheight = LARGEST_DIMENSION;
	}
	if (hints->flags&PResizeInc) {
	  xinc = hints->width_inc;
	  yinc = hints->height_inc;
	}
	if (xinc <= 0) xinc = 1;
	if (yinc <= 0) yinc = 1;

	if (hints->flags&PSize || hints->flags&USSize) {
	  defwidth = hints->width;
	  defheight = hints->height;
	} else if (hints->flags&PMinSize) {
	  defwidth = hints->min_width;
	  defheight = hints->min_height;
	} else if (hints->flags&PMaxSize) {
	  defwidth = hints->max_width;
	  defheight = hints->max_height;
        }
	defwidth += two_borders;
	defheight += two_borders;

      /* until there are better WM_HINTS, we'll assume that the client's
       * minimum width and height are the appropriate offsets to subtract
       * when resizing with an explicit resize increment.
       */
      if (hints->flags&PMinSize && hints->flags&PResizeInc) {
	  width_offset = hints->min_width;
	  height_offset = hints->min_height;
      } else
	  width_offset = height_offset = 0;

	width_offset += two_borders;
	height_offset += two_borders;

	XQueryPointer (dpy, RootWindow(dpy, scr), &root, &subw, 
	  &rootx, &rooty, &x1, &y1, &mask);
	hsize = defwidth;		/* already includes border_width */
	vsize = defheight;		/* already includes border_width */
	x2 = x1+hsize; 
	y2 = y1+vsize;
	stop = FALSE;
	changed = TRUE;

	while (stop == FALSE) {
	    if ( (xb != max (x1, x2)) || (yb != max (y1, y2))
		||(xa != min (x1, x2)) || (ya != min (y1, y2)) ) {
		xa = min (x1, x2);
		ya = min (y1, y2);
		xb = max (x1, x2);
		yb = max (y1, y2);
		for ( i = 0; i < PCOUNT; i += 4) {
                    box[i].x = xa; box[i].y = ya;
                    if (i+1 == PCOUNT) break;
                    box[i+1].x = xb; box[i+1].y = ya;
                    box[i+2].x = xb; box[i+2].y = yb;
                    box[i+3].x = xa; box[i+3].y = yb;
                }
	    }
	    if (XPending (dpy) &&
		XCheckWindowEvent (dpy, pop, ~NoEventMask, &e)) {
		switch (e.type) {
		  case VisibilityNotify:
		    if (((XVisibilityEvent *)&e)->state != 
		        VisibilityUnobscured) 
		      XRaiseWindow (dpy, pop);
		    continue;		/* go to top of while loop */
		  case Expose:
		    changed = TRUE;
		    break;		/* but go to next if statement */
		  default:
		    break;		/* ignore it */
		}
	    }
	    if (changed) {
	        int Hsize = (hsize - width_offset) / xinc;
		int Vsize = (vsize - height_offset) / yinc;
		int pos = 4;
		int n, digits;

		/* text consists of strcat (name, ": 0000x0000"), nz = len */

		changed = FALSE;
		n = digits = 0;
		if (Hsize > 999) {
		    text[nz - 9] = (n = (Hsize / 1000)) + '0';
		    Hsize -= n * 1000; n = 0; digits++;
		} else {
		    text[nz - 9] = ' ';
		}
		if (Hsize > 99 || digits) {
		    text[nz - 8] = (n = (Hsize / 100)) + '0';
		    Hsize -= n * 100; n = 0; digits++;
		} else {
		    text[nz - 8] = ' ';
		}
		if (Hsize > 9 || digits) {
		    text[nz - 7] = (n = (Hsize / 10)) + '0';
		    Hsize -= n * 10; n = 0; digits++;
		} else {
		    text[nz - 7] = ' ';
		}
		text[nz - 6] = Hsize + '0';

		n = digits = 0;
		if (Vsize > 999) {
		    text[nz - pos--] = (n = (Vsize / 1000)) + '0';
		    Vsize -= n * 1000; n = 0; digits++;
		} 
		if (Vsize > 99 || digits) {
		    text[nz - pos--] = (n = (Vsize / 100)) + '0';
		    Vsize -= n * 100; n = 0; digits++;
		} 
		if (Vsize > 9 || digits) {
		    text[nz - pos--] = (n = (Vsize / 10)) + '0';
		    Vsize -= n * 10; n = 0; digits++;
		} 
		text[nz - pos--] = Vsize + '0';

		while (pos > 0) text[nz - pos--] = ' ';

		XDrawImageString(dpy, pop, popGC, IBW, IBW+pfont->ascent,
			text, nz);
	    }
	    XDrawLines (dpy, RootWindow(dpy, scr), invertGC, box, PCOUNT, 
			CoordModeOrigin);
            if (XPending(dpy) &&
		XCheckMaskEvent(dpy, ButtonPressMask|ButtonReleaseMask, &e)) {
                if ((chosen < 0) && (e.type == ButtonPress)) {
			x1 = x2 = ((XButtonEvent *)&e)->x;
                        y1 = y2 = ((XButtonEvent *)&e)->y;
                        chosen = ((XButtonEvent *)&e)->button;
			if (chosen == Button2)
				change_cursor = TRUE;
                }
                else if ((e.type == ButtonRelease) &&
                        ((((XButtonEvent *)&e)->button) == chosen)) {
                	x2 = ((XButtonEvent *)&e)->x;
                	y2 = ((XButtonEvent *)&e)->y;
                        stop = TRUE;
                }
		else
                        XQueryPointer (dpy, RootWindow(dpy, scr), &root, 
			  &subw, &rootx, &rooty, &x2, &y2, &mask);
            }
            else        XQueryPointer (dpy, RootWindow(dpy, scr), &root, 
	    		  &subw, &rootx, &rooty, &x2, &y2, &mask);
	    if (change_cursor) {
                if ((x2 >= x1) && (y2 >= y1) &&
                    current_cursor != lr) {
                    XChangeActivePointerGrab (dpy, events, lr, CurrentTime );
                    current_cursor = lr;
               }
                else if ((x2 >= x1) && (y2 < y1) &&
                         current_cursor != ur) {
                    XChangeActivePointerGrab (dpy, events, ur, CurrentTime );
                    current_cursor = ur;
                }
                else if ((x2 < x1) && (y2 >= y1) &&
                         current_cursor != ll) {
                    XChangeActivePointerGrab (dpy, events, ll, CurrentTime );
                    current_cursor = ll;
                }
                else if ((x2 < x1) && (y2 < y1) &&
                         (current_cursor != ul)) {
                    XChangeActivePointerGrab (dpy, events, ul, CurrentTime );
                    current_cursor = ul;
                }
	    }
	    if (chosen != Button2) {
		x1 = x2;
		y1 = y2;
		if (chosen >= 0) {
			x2 = defwidth;
			if (chosen == Button1)
				y2 = defheight;
			else
				y2 = (DisplayHeight(dpy, scr) - y1);
			x2 = x1 + x2;
			y2 = y1 + y2;
		} else {
		    x2 = x1 + defwidth;
		    y2 = y1 + defheight;
		}
	    }

	    dx = max(min(abs (x2 - x1), maxwidth), minwidth);
	    dx = makemult(dx-minwidth, xinc)+minwidth; 
	    Rdx = dx - two_borders;

	    dy = max(min(abs(y2 - y1), maxheight), minheight);
	    dy = makemult(dy-minheight, yinc)+minheight; 
	    Rdy = dy - two_borders;
	    
	    if (hints->flags & PAspect) {
                if ((Rdx * hints->max_aspect.y > Rdy * hints->max_aspect.x)) {
		   delta = makemult( 
		       (Rdx*hints->max_aspect.y/hints->max_aspect.x) - Rdy, 
		       yinc); 
		   if ((dy+delta) <= maxheight)  dy += delta;
		   else {
		     delta = makemult(
		        Rdx - hints->max_aspect.x * Rdy/hints->max_aspect.y, 
			xinc);
		     if ((dx-delta) >= minwidth) dx -= delta;
		   }
		}
                if (Rdx * hints->min_aspect.y < Rdy * hints->min_aspect.x) {
		   delta = makemult( 
		        (hints->min_aspect.x * Rdy/hints->min_aspect.y) - Rdx,
			xinc);
		   if (dx+delta <= maxwidth) dx += delta;
		   else {
		     delta = makemult(
		       Rdy - (Rdx*hints->min_aspect.y / hints->min_aspect.x), 
		       yinc); 
		     if ((dy-delta) >= minheight) dy -= delta;
		   }
		}
		  
	    }

	    if (dx != hsize) {
	    	hsize = dx;
		changed = TRUE;
	    }
	    if (dy != vsize) {
	    	vsize = dy;
		changed = TRUE;
	    }
	    if (x2 < x1)
		x2 = x1 - dx;
	    else
		x2 = x1 + dx;

	    if (y2 < y1)
		y2 = y1 - dy;
	    else
	    	y2 = y1 + dy;
	}
	XUngrabPointer(dpy, CurrentTime);

	XDestroyWindow (dpy, pop);
	/* XFreeFont(dpy, pfont); */
	XFreeCursor (dpy, ur);
	XFreeCursor (dpy, ul);
	XFreeCursor (dpy, lr);
	XFreeCursor (dpy, ll);
	if (free_name) free(name);
	free(text);
	*x = min(x1, x2);
	*y = min(y1, y2);
	*width = hsize - two_borders;
	*height = vsize - two_borders;
	XSync(dpy, False);
	}
