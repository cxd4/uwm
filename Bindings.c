#ifndef lint
static char *rcsid_Bindings_c = "$XConsortium: Bindings.c,v 1.15 88/09/22 16:08:10 jim Exp $";
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
 */

#ifndef lint
static char *sccsid = "@(#)Bindings.c	3.8	1/24/86";
#endif

/*
 * Bindings.c 	Default bindings for the X window manager 'uwm'
 *
 * Note: Any changes to this file should also be added to the file
 * /usr/new/lib/X/uwm/default.uwmrc to keep users informed as to the bindings
 * contained herein.
 */

char *DefaultBindings[] = {
"resetbindings;resetvariables;resetmenus;noautoselect;delta=5;freeze",
"grid;zap;pushabsolute;push=1;hiconpad=5;viconpad=5;hmenupad=3;vmenupad=0",
"iconfont=fixed;menufont=fixed;resizefont=fixed;volume=0;noresizerelative",
"f.newiconify=m:w|i:delta l",
"f.raise=m:w|i:delta l",
"f.lower=m:w|i:l u",
"f.raise=m:w:m d",
"f.resize=m:w:delta m",
"f.iconify=m:i:m u",
"f.raise=m:w|i:r d",
"f.move=m:w|i:delta r",
"f.circledown=m:r:l d",
"f.circleup=m:r:r d",
"f.circledown=m|s::l d",
"f.menu=:r:m d:\"WindowOps\"",
"f.menu=m|s::m d:\"WindowOps\"",
"f.menu=m|s::m d:\"Preferences\"",
"f.circleup=m|s::r d",
"f.iconify=m|c:w|i:l d",
"f.newiconify=m|l:w|i:l d",
"f.raise=m|l:w|i:l u",
"f.pushright=m|l:w|i:r d",
"f.pushleft=m|c:w|i:r d",
"f.pushup=m|l:w|i:m d",
"f.pushdown=m|c:w|i:m d",
"menu=\"WindowOps\"{",
"New Window:!\"xterm&\"",
"RefreshScreen:f.refresh",
"Redraw:f.redraw",
"Move:f.move",
"Resize:f.resize",
"Lower:f.lower",
"Raise:f.raise",
"CircUp:f.circleup",
"CircDown:f.circledown",
"AutoIconify:f.iconify",
"LowerIconify:f.newiconify",
"NewIconify:f.newiconify",
"Focus:f.focus",
"Freeze:f.pause",
"UnFreeze:f.continue",
"Restart:f.restart",
"\" \":f.beep",
"KillWindow:f.kill",
"\" \":f.beep",
"Exit:f.exit",
"}",
"menu=\"Preferences\"{",
"Bell Loud:!\"xset b 7&\"",
"Bell Normal:!\"xset b 3&\"",
"Bell Off:!\"xset b off&\"",
"Click Loud:!\"xset c 8&\"",
"Click Soft:!\"xset c on&\"",
"Click Off:!\"xset c off&\"",
"Lock On:!\"xset l on&\"",
"Lock Off:!\"xset l off&\"",
"Mouse Fast:!\"xset m 4 2&\"",
"Mouse Normal:!\"xset m 2 5&\"",
"Mouse Slow:!\"xset m 1 1&\"",
"}",
0	/* Must NOT be removed. */
};
