/* $XConsortium: Kill.c,v 1.2 88/09/06 17:28:26 jim Exp $ */

/* $XConsortium: copyright.h,v 1.5 89/12/22 16:11:28 rws Exp $ */
/*

Copyright 1985, 1986, 1987, 1988, 1989 by the
Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

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
