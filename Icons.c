/* $XConsortium: Icons.c,v 1.13 89/04/22 12:11:20 rws Exp $ */
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
 * 000 -- L. Guarino Reid, DEC Ultrix Engineering Group
 */
 
#ifndef lint
static char *sccsid = "%W%	%G%";
#endif
 
#include "uwm.h"
#include <X11/Xatom.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

typedef struct _windowList {
  struct _windowList *next;
  Window window;
  Window icon;
  Bool own;
  Pixmap pixmap;
} WindowListRec, *WindowList;
  
WindowList Icons = NULL;


/* the client should pass us a bitmap (single-plane pixmap with background=0
 * and foreground = 1).  It is our responsibility to convert it to a pixmap
 * of the appropriate depth for a window tile and also color it with the
 * appropriate background and foreground pixels.
 *
 * we'll use the (global) IconGC for the fore/background pixels.
 */

static Pixmap MakePixmapFromBitmap( bitmap, width_return, height_return )
Pixmap bitmap;
unsigned int *width_return, *height_return;
{
    Pixmap tile;
    Window junkW;
    int junk, width, height;

    if (!XGetGeometry( dpy, bitmap, &junkW, &junk, &junk,
		      &width, &height, &junk, &junk )) {
        Warning( "client passed invalid pixmap for icon." );
	return( NULL );
    }

    tile = XCreatePixmap( dpy, RootWindow(dpy, scr), width, height,
			  DefaultDepth(dpy, scr) );

    /* use the IconGC's foreground & background, so we don't have to
     * create another (and add yet another user configuration option.
     * someday this may need to be split out.
     */
    XCopyPlane( dpy, bitmap, tile, IconGC, 0, 0, width, height, 0, 0, 1 );

    if (width_return)  *width_return = width;
    if (height_return) *height_return = height;

    return( tile );
}


char *
GetIconName(window)
Window window;
{
    char *name;

    if (XGetIconName( dpy, window, &name )) return( name );

    if (XFetchName( dpy, window, &name )) return( name );

    return( NULL );
}

Bool IsIcon(icon, x, y, mousePositioned, assoc)
Window icon;
Window *assoc;
{
  WindowList ptr;
  Window MakeIcon();

  for (ptr = Icons; ptr; ptr = ptr->next) {
    if (ptr->icon == icon) {
      if (assoc) *assoc = ptr->window; 
      return(TRUE);
    }
    if (ptr->window == icon) {
      if (assoc) *assoc = ptr->icon; 
      return(FALSE);
    }
  }
  if (assoc) *assoc = MakeIcon(icon, x, y, mousePositioned);
  return(FALSE);
}


RemoveIcon(window)
Window window;
{
  WindowList ptr, ptr1;

  for (ptr = Icons; ptr; ptr = ptr->next) 
    if (ptr->window == window) {
      if (ptr->own) {
	  XDestroyWindow(dpy, ptr->icon);
	  if (ptr->pixmap != IBackground) XFreePixmap(dpy, ptr->pixmap);
      }
      break;
    }
  if (ptr) {
    if (ptr==Icons) Icons = Icons->next;
    else 
      for (ptr1 = Icons; ptr1->next; ptr1 = ptr1->next) 
        if (ptr1->next == ptr) {
          ptr1->next = ptr->next;
	  break;
        };
    free(ptr);
    }
}

GetDefaultSize(window, icon_w, icon_h)
Window window;
int *icon_w, *icon_h;
{
    char *name;				/* Event window name. */

          /*
           * Determine the size of the icon window.
           */ 
          name = GetIconName(window);
          *icon_h = IFontInfo->ascent + IFontInfo->descent;
          if (name) {
	    *icon_w = XTextWidth(IFontInfo, name, strlen(name));
            if (*icon_w == 0)
              *icon_w = *icon_h;
	  } else 
	    *icon_w = *icon_h;
     }

Window MakeIcon(window, x, y, mousePositioned)
Window window;                          /* associated window. */
int x, y;                               /* Event mouse position. */
Bool mousePositioned;
{
    Window icon;			/* icon window. */
    int icon_x, icon_y;			/* Icon U. L. X and Y coordinates. */
    int icon_w, icon_h;			/* Icon width and height. */
    int icon_bdr;			/* Icon border width. */
    int mask;				/* Icon event mask */
    int depth;				/* for XGetGeometry */
    XSetWindowAttributes iconValues;	/* for icon window creation */
    XWMHints *wmhints;			/* see if icon position provided */
    XWMHints *XGetWMHints();
    Window AddIcon();
 
   iconValues.background_pixmap = IBackground;
   mask = (KeyPressMask|ExposureMask|StructureNotifyMask);
   /*
    * Process window manager hints.
    */ 
    if (wmhints = XGetWMHints(dpy, window)) {
      if (wmhints->flags&IconWindowHint) {
	  Window iw = wmhints->icon_window;
	  free ((char *) wmhints);
          return (AddIcon(window, iw, FALSE, 
	  	  (StructureNotifyMask), (Pixmap)NULL));
      } else if (wmhints->flags&IconPixmapHint) { 
          iconValues.background_pixmap =
	      MakePixmapFromBitmap( wmhints->icon_pixmap, &icon_w, &icon_h );
	  if (iconValues.background_pixmap)
	      mask = (StructureNotifyMask);
	  else {
	      iconValues.background_pixmap = IBackground;
	      wmhints->flags &= ~IconPixmapHint;
	      GetDefaultSize(window, &icon_w, &icon_h);
	  }
      }
      else GetDefaultSize(window, &icon_w, &icon_h);
    }
    else GetDefaultSize(window, &icon_w, &icon_h);

     /*
      * Fix up sizes by padding.
      */ 
    if (!wmhints || !(wmhints->flags&(IconPixmapHint|IconWindowHint))) {
      icon_w += (HIconPad << 1);
      icon_h += (VIconPad << 1);
    }

     /*
      * Set the icon border attributes.
      */ 
    if (!wmhints || !(wmhints->flags&IconWindowHint)) {
      icon_bdr = IBorderWidth;
      iconValues.border_pixel = IBorder;
    }
 
    if (wmhints && (wmhints->flags&IconPositionHint)) {
         icon_x = wmhints->icon_x;
	 icon_y = wmhints->icon_y;
    } else {
      if (mousePositioned) {
        /*
         * Determine the coordinates of the icon window;
         * normalize so that we don't lose the icon off the
         * edge of the screen.
         */
        icon_x = x - (icon_w >> 1) + 1;
        if (icon_x < 0) icon_x = 0;
        icon_y = y - (icon_h >> 1) + 1;
        if (icon_y < 0) icon_y = 0;
        if ((icon_x - 1 + icon_w + (icon_bdr << 1)) > ScreenWidth) {
           icon_x = ScreenWidth - icon_w - (icon_bdr << 1) + 1;
        }
        if ((icon_y - 1 + icon_h + (icon_bdr << 1)) > ScreenHeight) {
           icon_y = ScreenHeight - icon_h - (icon_bdr << 1) + 1;
        }
      }
      else {
        icon_x = x + (icon_w >> 1);
        icon_y = y + (icon_h >> 1);
      }
        
    }

   /*
    * Create the icon window.
    */
   icon = XCreateWindow(
                dpy, RootWindow(dpy, scr),
                icon_x, icon_y,
                icon_w, icon_h,
                icon_bdr, 0, CopyFromParent, CopyFromParent,
		CWBorderPixel+CWBackPixmap, &iconValues);

#ifdef SHAPE
   if ((iconValues.background_pixmap != IBackground) &&
       (wmhints->flags&IconMaskHint) &&
       XShapeQueryExtension(dpy)) {
	XSetWindowBorderWidth(dpy, icon, 0);
	XShapeCombineMask(dpy, icon, ShapeBounding, 0, 0, wmhints->icon_mask,
			  ShapeSet);
    }
#endif

   if (wmhints) free ((char *) wmhints);

   return(AddIcon(window, icon, TRUE, mask, iconValues.background_pixmap));
 
}

Window AddIcon(window, icon, own, mask, background)
Window window, icon;
Bool own;
int mask;
Pixmap background;
{
  WindowList ptr;

   if (icon == NULL) return(NULL);
   /*
    * Use the text cursor whenever the mouse is in the icon window.
    */
   XDefineCursor(dpy, icon, TextCursor);
    
   /*
    * Select "key pressed", "window exposure" and "unmap window"
    * events for the icon window.
    */
   uwmExpressInterest (icon, mask);
    
    /*
     * Set the event window's icon window to be the new icon window.
     */
    ptr = (WindowList) malloc(sizeof(WindowListRec));
    ptr->window = window;
    ptr->icon = icon;
    ptr->own = own;
    ptr->pixmap = background;
    ptr->next = Icons;
    Icons = ptr;

    return(icon);
}
