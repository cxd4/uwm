#ifndef lint
static char *rcsid_XError_c = "$XConsortium: XError.c,v 1.7 88/09/22 10:53:35 jim Exp $";
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
static char *sccsid = "@(#)XError.c	3.8	1/24/86";
#endif

#include "uwm.h"

int XError(dpy, rep)
Display *dpy;
XErrorEvent *rep;
{
#ifdef DEBUG
    char buffer[BUFSIZ];
    XGetErrorText(dpy, rep->error_code, buffer, BUFSIZ);
    fprintf(stderr, "uwm: X error occurred during a uwm operation.\n");
    fprintf(stderr, "     Description: '%s'\n", buffer);
    fprintf(stderr, "     Request code: %d\n", rep->request_code);
    fprintf(stderr, "     Request function: %d\n", rep->func);
    fprintf(stderr, "     Request window 0x%x\n", rep->window);
    fprintf(stderr, "     Error Serial #%d\n", rep->serial);
    fprintf(stderr, "     Current serial #%d\n", dpy->request);
#endif
    x_error_occurred = TRUE;
    last_error_event = *rep;
    return (0);
}
