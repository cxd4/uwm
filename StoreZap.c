#ifndef lint
static char *rcsid_StoreZap_c = "$XConsortium: StoreZap.c,v 1.4 88/09/06 17:28:44 jim Exp $";
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
 * 001 -- Loretta Guarino Reid, DEC Ultrix Engineering Group,
    Western Software Lab. Convert to X11.
 */

#ifndef lint
static char *sccsid = "@(#)StoreZap.c	3.8	1/24/86";
#endif
/*
 *	StoreZap - This subroutine is used by the X Window Manager (xwm)
 *	to store the vertices for the iconify, uniconify zap.
 *
 */

#include "uwm.h"

/*
 * Store the vertices for the zap effect.
 */
int StoreZap(zap, ulx_1, uly_1, lrx_1, lry_1, ulx_2, uly_2, lrx_2, lry_2)
    register XSegment zap[];
    int ulx_1;		/* From window, upper left X coordinate. */
    int uly_1;		/* From window, upper left Y coordinate. */
    int lrx_1;		/* From window, lower right X coordinate. */
    int lry_1;		/* From window, lower right Y coordinate. */
    int ulx_2;		/* To window, upper left X coordinate. */
    int uly_2;		/* To window, upper left Y coordinate. */
    int lrx_2;		/* To window, lower right X coordinate. */
    int lry_2;		/* To window, lower right Y coordinate. */
{

    /*
     * Xor in.
     */
    zap[0].x1 = ulx_1;	zap[0].y1 = uly_1;
    zap[0].x2 = ulx_2;	zap[0].y2 = uly_2;

    zap[1].x1 = lrx_1;	zap[1].y1 = uly_1;
    zap[1].x2 = lrx_2;	zap[1].y2 = uly_2;

    zap[2].x1 = lrx_1;	zap[2].y1 = lry_1;
    zap[2].x2 = lrx_2;	zap[2].y2 = lry_2;

    zap[3].x1 = ulx_1;	zap[3].y1 = lry_1;
    zap[3].x2 = ulx_2;	zap[3].y2 = lry_2;

    /*
     * Xor out.
     */
    zap[4].x1 = ulx_1;	zap[4].y1 = uly_1;
    zap[4].x2 = ulx_2;	zap[4].y2 = uly_2;

    zap[5].x1 = lrx_1;	zap[5].y1 = uly_1;
    zap[5].x2 = lrx_2;	zap[5].y2 = uly_2;

    zap[6].x1 = lrx_1;	zap[6].y1 = lry_1;
    zap[6].x2 = lrx_2;	zap[6].y2 = lry_2;

    zap[7].x1 = ulx_1;	zap[7].y1 = lry_1;
    zap[7].x2 = ulx_2;	zap[7].y2 = lry_2;
    /*
     * Total number of vertices is 8.
     */
    return(8);
}
