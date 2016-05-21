#ifndef lint
static char *rcsid_Menu_c = "$XConsortium: Menu.c,v 1.25 88/09/30 11:41:11 jim Exp $";
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
 * 001 -- L. Guarino Reid, DEC Ultrix Engineering Group, Western Software Lab
 *	  February 16, 1987
 *	  Change menu implementation so that it uses EnterWindow, LeaveWindow,
 *	  and MouseMotion events to track the mouse, instead of polling.
 * 002 -- L. Guarino Reid, DEC Ultrix Engineering Group, Western Software Lab
 *	  April 30, 1987. Convert to X11.
 * 003 -- L. Guarino Reid, DEC Ultrix Engineering Group, Western Software Lab
 *	  June 18, 1987. Change call to system to handle signals move smoothly.
 */

#ifndef lint
static char *sccsid = "@(#)Menu.c	3.8	1/24/86";
#endif

#include <signal.h>
#include "uwm.h"

Bool alternateGC = True;	/* true if only 2 colors are used */

#define DisplayLine(w, pane, width, height, str, fg, bg, inv) \
         if (alternateGC) { \
	     if (inv) \
	         XFillRectangle(dpy, w, MenuInvGC, 0, pane, width, height); \
	     else \
                 XDrawString(dpy, w, MenuGC, HMenuPad, pane + VMenuPad + MFontInfo->ascent, str, strlen(str)); \
         } else { \
             XSetForeground(dpy, MenuGC, bg); \
	     XFillRectangle(dpy, w, MenuGC, 0, pane, width, height); \
             XSetForeground(dpy, MenuGC, fg); \
             XDrawString(dpy, w, MenuGC, HMenuPad, pane + VMenuPad + MFontInfo->ascent, str, strlen(str)); \
         }

/*
 * HP-UX does have vfork, but A/UX doesn't
 */
#if (defined(SYSV) || defined(macII)) && !defined(hpux)
#define vfork() fork()
#endif

/* the following procedure is a copy of the implementation of system, 
 * modified to reset the handling of SIGINT, SIGQUIT, and SIGHUP before
 * exec-ing
 */
execute(s)
char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();
	char **oldenviron;
	extern char **environ, **newenviron;

	oldenviron = environ;
	environ = newenviron;
	if ((pid = vfork()) == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	environ = oldenviron;

	/* 
	 * The following cast is necessary because newer compilers declare
	 * signal() to be (void (*)()) whereas old ones use (int (*)()).
	 */
	istat = (int (*)()) signal(SIGINT, SIG_IGN);
	qstat = (int (*)()) signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}

Bool Menu(window, mask, button, x, y, menu)
Window window;				/* Event window. */
int mask;				/* Button/key mask. */
int button;				/* Button event detail. */
int x, y;				/* Event mouse position. */
MenuInfo *menu;
{
    XEvent button_event;		/* Button event packet. */
    int event_x, event_y;		/* location of button event */
    Bool func_stat;			/* Function status return. */
    Window sub_window;			/* Current subwindow. */
    int cur_item = 0;			/* Current menu item. */
    int hi_lite = 0;			/* Current highlighted item. */
    int i;				/* Iteration counter. */
    int hlfg, hlbg;			/* Hi-liter pixels. */
    MenuLine *ml;			/* Menu lines pointer. */
    char *hlname;			/* Pointer to hi-liter name. */
    char *strbuf;			/* String buffer for IsTextNL. */
    Bool checkMotion = TRUE;		/* To Restore Cursor Position */
    char *malloc();
    int funcbutton;			/* function button for window select */

    /*
     * Change the cursor.
     */
    XChangeActivePointerGrab(dpy, EVENTMASK, MenuCursor, CurrentTime);

    /*
     * Map the menu.
     */
    MapMenu(menu, x, y);
    if (Autoselect) {
        event_x = (menu->width >> 2) * 3;
        event_y = (menu->iheight >> 1) * 3;
        XWarpPointer(dpy, None, menu->w, 0, 0, 0, 0, event_x, event_y);
	goto hilite;
    }
    else {
        XWarpPointer(dpy, None, menu->w, 0, 0, 0, 0, 
    		(menu->width >> 2) * 3, menu->iheight >> 1);
        XFlush(dpy);
    }

    /*
     * Main loop.
     */
    while (TRUE) {

        /*
         *  Get next event for menu.
         */
        if (!GetButton(&button_event)) continue;
	switch (button_event.type) {

            case LeaveNotify:
	        /*
	         * If the mouse has moved out of the menu sideways, abort
	         * the menu operation. Reset the cursor and unmap the menu.
		 * An extraneous LeaveNotify can come through if you are
		 * popping up menu from a menu. If you moved the cursor toward
		 * the edge of the original menu without going outside and
		 * the new menu is smaller in width than the old menu, the
		 * new menu will pop up and then disappear.
	         */
		if (((XAnyEvent *)&button_event)->window == menu->w) {
			event_x = ((XLeaveWindowEvent * )&button_event)->x;
	       		event_y = ((XLeaveWindowEvent * )&button_event)->y;
			if (event_x < 0 || event_x > menu->width) {
            		   ResetCursor(button);
			   UnmapMenu(menu);
          	  	   return(FALSE);
        		}
		}
		goto hilite;

            case EnterNotify:
	        event_x = ((XEnterWindowEvent * )&button_event)->x;
	        event_y = ((XEnterWindowEvent * )&button_event)->y;
		goto hilite;
            case MotionNotify:
		{
	        event_x = ((XPointerMovedEvent * )&button_event)->x;
	        event_y = ((XPointerMovedEvent * )&button_event)->y;
		if ((checkMotion == TRUE) &&
		    ((event_x != (menu->width >> 2) * 3) ||
		     (event_y != (menu->iheight >> 1))))
		   checkMotion = FALSE;	 /* Mouse Has Moved From Top Pane */
hilite:
        	/*
         	* If the mouse has moved below or above the menu, but is still
         	* within the same vertical plane, then simply adjust the values
         	* so the user doesn't fall off the edge.
         	*/
        	if (event_y >= menu->height) 
		  event_y = menu->height - 1;
        	else if (event_y < 0) 
		  event_y = 0;
		  
        	/*
         	* If the mouse has moved to another item in the menu,
         	* highlight the new item.
         	*/
        	cur_item = event_y / menu->iheight;
        	if (cur_item != hi_lite) {

            	/*
             	* Remove highlighting on old item.
             	*/
            	if (hi_lite) {
                	DisplayLine(menu->w, hi_lite * menu->iheight,
                            menu->width, menu->iheight, hlname,
                            hlfg, hlbg, 1);
			XFlush(dpy);
            	}

           	/*
             	* Highlight new item.
             	*/
            	hi_lite = cur_item;
            	if (cur_item) {
                	for(i = 1, ml = menu->line; ml; i++, ml = ml->next) {
                    		if (i == cur_item) break;
               	 	}
                	DisplayLine(menu->w, cur_item * menu->iheight,
                            menu->width, menu->iheight, ml->name,
                            menu->hlfg.pixel, menu->hlbg.pixel, 1);
/*                	XSetForeground(dpy, MenuGC, menu->hlfg.pixel );
			XDrawRectangle(dpy, menu->w, MenuGC, 1, 
				cur_item * menu->iheight + 1, 
				menu->width - 3, menu->iheight - 3);
*/
			XFlush(dpy);
             		hlfg = ml->fg.pixel;
            		hlbg = ml->bg.pixel;
            		hlname = ml->name;
            	}
		}
        	break;

            case ButtonRelease:
	        /* have we released the invoking button? */
	        if (((XButtonReleasedEvent *)&button_event)->button == button) {
		    /*
		     * If no item was selected, 
		     * close the menu, reset the pointer's position and return
		     */
		    if (!cur_item) {
			/* If Needed Reset The Cursor Position */
			if (checkMotion == TRUE)
			  XWarpPointer (dpy, None, RootWindow(dpy, scr),
					0, 0, 0, 0, x, y);
		         ResetCursor(button);
			 UnmapMenu(menu);
		         return(TRUE);
		     }

		     /*
		      * Get a pointer to the menu line selected.
		      */
		     --cur_item;
		     for(i = 0, ml = menu->line; ml; i++, ml = ml->next) {
		         if (i == cur_item) break;
		     }

		     /*
		      * Perform the selected menu line action.
		      */
		     switch (ml->type) {

		         case IsShellCommand:
		             UnmapMenu(menu);
		             execute(ml->text);
		             break;

		         case IsText:
		             UnmapMenu(menu);
		             XStoreBytes(dpy, ml->text, strlen(ml->text));
		             break;

		         case IsTextNL:
		             UnmapMenu(menu);
		             strbuf = (char *)malloc(strlen(ml->text) + 2);
		             strcpy(strbuf, ml->text);
		             strcat(strbuf, "\n");
		             XStoreBytes(dpy, strbuf, strlen(strbuf));
		             free(strbuf);
		             break;

		         case IsUwmFunction:
			     /* change cursor and grab next button event
			      * to select the target window */
			     if (XGrabPointer( dpy, RootWindow(dpy, scr),
					       TRUE, EVENTMASK, GrabModeAsync,
					       GrabModeAsync, None,
					       TargetCursor, CurrentTime )
				   != GrabSuccess )
			         Error( "Could not grab pointer" );
		             funcbutton = GetContext (&sub_window,
						      &event_x, &event_y);
		             UnmapMenu(menu);
		             if (sub_window != menu->w)
			       func_stat =
		                 (*ml->func) (sub_window, mask, funcbutton,
					      event_x, event_y);
			     else func_stat = FALSE;
			     if (!func_stat) {
			       /* eat the next ButtonRelease */
			       while (TRUE) {
				   if (GetButton(&button_event) &&
				       button_event.type == ButtonRelease)
				     break;
			       }
			     }
			     XUngrabPointer( dpy, CurrentTime );
		             break;

		         case IsImmFunction:
		             UnmapMenu(menu);
 		            (*ml->func) (
			      sub_window, mask, button, event_x, 
			      event_y);
		             break;
		 
		         case IsMenuFunction:
		             while (TRUE) {
		                if (!GetButton(&button_event)) continue;
		                if (button_event.type != ButtonPress) continue;
		                if ((((XButtonPressedEvent *)&button_event)->state != mask) 
				 || (((XButtonPressedEvent *)&button_event)->button != button)) 
				 {
		                     UnmapMenu(menu);
		                     return(TRUE);
		                 }
		                 break;
		             }
		             UnmapMenu(menu);
		             func_stat = 
			     	Menu(menu->w, mask, button, x, y, ml->menu);
		             return(func_stat);
		             break;

		         default:
 		            Error("Menu -> Internal type error.");
		     }
		     return(TRUE);
		  
                 } 
		 /* else a different button was released. Fall through: */
            default:
                    /*
                     * Some other button event occurred, so abort the menu
                     * operation.
                     */
		    ResetCursor(button);
		    /* If Needed Reset The Cursor Position */
		    if (checkMotion == TRUE)
		      XWarpPointer (dpy, None, RootWindow(dpy, scr),
				    0, 0, 0, 0, x, y);
                    UnmapMenu(menu);
                    return(TRUE);
		
	}
     }
  }
}


/*
 * Create the menu windows for later use.
 */
CreateMenus()
{
    MenuLink *ptr;

    /*
     * If MaxColors isn't set, then jam it to an impossibly high
     * number.
     */
    if (MaxColors == 0)
        MaxColors = 25000;

    for(ptr = Menus; ptr; ptr = ptr->next)
        InitMenu(ptr->menu);
}

/*
 * Initialize a menu.
 */
InitMenu(menu)
MenuInfo *menu;
{
    MenuLine *ml;		/* Menu lines pointer. */
    int width;			/* Width of an item name. */
    int maxwidth;		/* Maximum width of item names. */
    int len;			/* Length of an item name. */
    int count = 1;		/* Number of items + 1 for name. */
    XSetWindowAttributes attr;

    /*
     * Determine the name of the longest menu item.
     */
    maxwidth = XTextWidth(MFontInfo, menu->name, strlen(menu->name));
    if (maxwidth == 0)
        Error("InitMenu -> Couldn't get length of menu name");

    for(ml = menu->line; ml; ml = ml->next) {
        if ((len = strlen(ml->name)) == 0)
            break;
        width = XTextWidth(MFontInfo, ml->name, strlen(ml->name));
        if (width == 0) 
	  Error("InitMenu -> Couldn't get length of menu item name");
        if (width > maxwidth) maxwidth = width;
        count++;
    }

    /*
     * Get the color cells for the menu items.
     */
    GetMenuColors(menu);

    /*
     * Stash the menu parameters in the menu info structure.
     */
    menu->iheight = MFontInfo->ascent + MFontInfo->descent + (VMenuPad << 1);
    menu->height = menu->iheight * count;
    menu->width = maxwidth + (HMenuPad << 1);
    menu->image = NULL;

    /*
     * Create the menu window.
     */
    attr.border_pixel = MBorder;
    /*
     * SaveUnders Enabled If The Server Supports It
     */
    attr.save_under = DoesSaveUnders( DefaultScreenOfDisplay( dpy ) );
    /*
     * We want enter, leave, and mouse motion events for menus.
     */
    attr.event_mask = ( EnterWindowMask | LeaveWindowMask | PointerMotionMask );
    /*
     * Define a cursor for the window
     */
    attr.cursor = MenuCursor;
    menu->w = XCreateWindow( dpy, RootWindow( dpy, scr ),
			     0, 0,
			     menu->width, menu->height,
			     MBorderWidth,
			     DefaultDepth( dpy, scr ),
			     InputOutput,
			     DefaultVisual( dpy, scr ),
	CWBorderPixel | CWSaveUnder | CWEventMask | CWCursor,
			     &attr );

    uwmExpressInterest (menu->w, attr.event_mask);

    if (menu->w == NULL) Error("InitMenu -> Couldn't create menu window");

    /*
     * For monochrome servers, we *do* want to set a background pixel
     */
    if (alternateGC) {
	XSetWindowBackground( dpy, menu->w, MBackground );
    }

    /*
     * Store the window name.
     */
    XStoreName(dpy, menu->w, menu->name);

}

/*
 * Map a menu.
 */
MapMenu(menu, x, y)
MenuInfo *menu;
int x, y;
{
    int item;
    Window w;
    MenuLine *ml;
    XWindowChanges values;

    w = menu->w;

    /*
     * Move the menu into place, normalizing the coordinates, if necessary;
     * then map it.
     */
    x -= (menu->width >> 1);
    if (x < 0) x = 0;
    else if (x + menu->width >= ScreenWidth)
        x = ScreenWidth - menu->width - (MBorderWidth << 1);
    if (y < 0) y = 0;
    else if (y + menu->height >= ScreenHeight)
        y = ScreenHeight - menu->height - (MBorderWidth << 1);
    values.x = x;
    values.y = y;
    values.stack_mode = Above;
    XConfigureWindow(dpy, w, CWX|CWY|CWStackMode, &values);

    /*
     * Map the window and draw the text items.
     */
    XMapWindow(dpy, w);
    DisplayLine(w, 0, menu->width, menu->iheight, menu->name,
                menu->bg.pixel, menu->fg.pixel, 0);

    if (alternateGC) {
        XFillRectangle(dpy, menu->w, MenuInvGC, 0, 0,
		       menu->width, menu->iheight);
        XDrawRectangle(dpy, menu->w, MenuInvGC, 1, 1,
		       menu->width - 3, menu->iheight - 3);
    } else {
        XSetForeground(dpy, MenuGC, menu->bg.pixel );
        XDrawRectangle(dpy, menu->w, MenuGC, 1, 1, menu->width - 3, 
		       menu->iheight - 3);
    }

    item = menu->iheight;
    for(ml = menu->line; ml; ml = ml->next) {
        DisplayLine(w, item, menu->width, menu->iheight, ml->name,
                    ml->fg.pixel, ml->bg.pixel, 0);
        item += menu->iheight;
    }

    /*
     * Position the mouse cursor in the menu header (or in the first item
     * if "autoselect" is set).
     */

    XFlush(dpy);
}

/*
 * Unmap a menu, restoring the contents of the screen underneath
 * if necessary. (Restore portion is a future.)
 */
UnmapMenu(menu)
MenuInfo *menu;
{
    /*
     * Unmap and flush.
     */
    XUnmapWindow(dpy, menu->w);
    XFlush(dpy);
}

/*
 * Get the context for invoking a window manager function; return the button
 * pressed.
 */
int GetContext(w, x, y)
Window *w;
int *x, *y;
{
    XEvent button_event;  /* Button input event. */

    while (TRUE) {

        /*
         * Get the next mouse button event.  Spin our wheels until
         * a button event is returned (ie. GetButton == TRUE).
         * Note that mouse events within an icon window are handled
         * in the "GetButton" function or by the icon's owner if
         * it is not uwm.
         */
        if (!GetButton(&button_event)) continue;

        /*
         * If the button event received is not a ButtonPress event
         * then continue until we find one.
         */
        if (button_event.type != ButtonPress) continue;

        /*
         * Okay, determine the event window and mouse coordinates.
         */
        status = XTranslateCoordinates(dpy, 
				    RootWindow(dpy, scr), 
				    RootWindow(dpy, scr),
                                    ((XButtonPressedEvent *)&button_event)->x, 
                                    ((XButtonPressedEvent *)&button_event)->y, 
                                    x, y,
                                    w);

        if (status == FAILURE) continue;

        if (*w == 0)
            *w = RootWindow(dpy, scr);

        return (((XButtonPressedEvent *) &button_event)->button);
    }
}

/*
 * Get the color cells for a menu.  This function is slightly brain-damaged
 * in that once MaxColors <= 1, then it refuses to even try to allocate any
 * more colors, even though the colors may have already been allocated.  It
 * probably ought to be done right someday.
 */
GetMenuColors(menu)
MenuInfo *menu;
{
    register MenuLine *ml;		/* Menu lines pointer. */

    /*
     * If we have more than 2 colors available, then attempt to get
     * the color map entries requested by the user.
     * Otherwise, default to standard black and white.
     *
     * The boolean "alternateGC" is true iff we can use just two colors.
     */

    if (DisplayCells(dpy, scr) > 2) {
        /*
         * Get the menu header colors first.
         */
        if (!(menu->foreground && menu->background && MaxColors > 1 &&
              XParseColor(dpy, DefaultColormap(dpy, scr), menu->foreground, &menu->fg) &&
              XAllocColor(dpy, DefaultColormap(dpy, scr), &menu->fg) &&
              XParseColor(dpy, DefaultColormap(dpy, scr), menu->background, &menu->bg) &&
              XAllocColor(dpy, DefaultColormap(dpy, scr), &menu->bg))) {
            menu->fg.pixel = MTextForeground;
            menu->bg.pixel = MTextBackground;
        } else {
            AdjustMaxColors(menu->fg.pixel);
            AdjustMaxColors(menu->bg.pixel);
	    alternateGC = False;	/* since we just allocated colors */
        }

        /*
         * Get the menu highlight colors.
         */
        if (!(menu->fghighlight && menu->bghighlight && MaxColors > 1 &&
              XParseColor(
	        dpy, DefaultColormap(dpy, scr), menu->fghighlight, &menu->hlfg) &&
              XAllocColor(dpy, DefaultColormap(dpy, scr), &menu->hlfg) &&
              XParseColor(
	        dpy, DefaultColormap(dpy, scr), menu->bghighlight, &menu->hlbg) &&
              XAllocColor(dpy, DefaultColormap(dpy, scr), &menu->hlbg))) {
            menu->hlfg.pixel = MTextBackground;
            menu->hlbg.pixel = MTextForeground;
        } else {
            AdjustMaxColors(menu->hlfg.pixel);
            AdjustMaxColors(menu->hlbg.pixel);
	    alternateGC = False;
        }

        /*
         * Get the menu item colors.
         */
        for(ml = menu->line; ml; ml = ml->next) {
            if (!(ml->foreground && ml->background && MaxColors > 1 &&
                  XParseColor(dpy, DefaultColormap(dpy, scr), ml->foreground, &ml->fg) &&
                  XAllocColor(dpy, DefaultColormap(dpy, scr), &ml->fg) &&
                  XParseColor(dpy, DefaultColormap(dpy, scr), ml->background, &ml->bg) &&
                  XAllocColor(dpy, DefaultColormap(dpy, scr), &ml->bg))) {
                ml->fg.pixel = MTextForeground;
                ml->bg.pixel = MTextBackground;
            } else {
                AdjustMaxColors(ml->fg.pixel);
                AdjustMaxColors(ml->bg.pixel);
            }
        }

    } else {

        /*
         * Only 2 colors available, so default to standard black and white.
         */
        menu->fg.pixel = MTextForeground;
        menu->bg.pixel = MTextBackground;
        menu->hlfg.pixel = MTextBackground;
        menu->hlbg.pixel = MTextForeground;
        for(ml = menu->line; ml; ml = ml->next) {
            ml->fg.pixel = MTextForeground;
            ml->bg.pixel = MTextBackground;
        }
    }
}

/*
 * Decrement "MaxColors" if this pixel value has never been used in a
 * menu before.
 */
AdjustMaxColors(pixel)
int pixel;
{
    register MenuLink *mptr;
    register MenuLine *lptr;
    int count = 0;

    for(mptr = Menus; mptr; mptr = mptr->next) {
        if (mptr->menu->fg.pixel == pixel) ++count;
        if (mptr->menu->bg.pixel == pixel) ++count;
        if (mptr->menu->hlfg.pixel == pixel) ++count;
        if (mptr->menu->hlbg.pixel == pixel) ++count;
        for(lptr = mptr->menu->line; lptr; lptr = lptr->next) {
            if (lptr->fg.pixel == pixel) ++count;
            if (lptr->bg.pixel == pixel) ++count;
        }
        if (count > 1) return;
    }
    --MaxColors;
}

