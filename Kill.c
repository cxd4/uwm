/* $XConsortium: Kill.c,v 1.2 88/09/06 17:28:26 jim Exp $ */

#include <X11/copyright.h>

#include "uwm.h"

Bool
Kill(window, mask, button, x, y)
Window window;                          /* Event window. */
int mask;                               /* Button/key mask. */
int button;                             /* Button event detail. */
int x, y;                               /* Event mouse position. */
{
    Window *assoc;

    if (IsIcon(window, x, y, TRUE, &assoc))
        XKillClient(dpy, assoc);
    else
        XKillClient(dpy, window);
    return FALSE ;
}
