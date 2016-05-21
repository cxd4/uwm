#ifndef lint
static char *rcsid_StoreGridBox_c = "$XConsortium: GridBox.c,v 1.4 88/09/06 17:28:06 jim Exp $";
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
    Western Software Lab. Converted to X11.
 */

#ifndef lint
static char *sccsid = "@(#)StoreGridBox.c	3.8	1/24/86";
#endif
/*
 *	StoreGridBox - This subroutine is used by the X Window Manager (xwm)
 *	to store the vertices for the movement resize grid and box in a
 *	vertex list.
 */

#include "uwm.h"

/*
 * Store the vertices for the movement resize grid and box in a vertex list.
 */
int StoreGridBox(box, ulx, uly, lrx, lry)
register XSegment box[];
int ulx;			/* Upper left X coordinate. */
int uly;			/* Upper left Y coordinate. */
int lrx;			/* Lower right X coordinate. */
int lry;			/* Lower right Y coordinate. */
{
    register int xthird, ythird;
    int x1third, y1third;
    int x2third, y2third;

    /*
     * Xor in.
     */
    box[0].x1 = ulx; box[0].y1 = uly;
    box[0].x2 = lrx; box[0].y2 = uly;

    box[1].x1 = lrx; box[1].y1 = uly;
    box[1].x2 = lrx; box[1].y2 = lry;

    box[2].x1 = lrx; box[2].y1 = lry;
    box[2].x2 = ulx; box[2].y2 = lry;

    box[3].x1 = ulx; box[3].y1 = lry;
    box[3].x2 = ulx; box[3].y2 = uly;


    /*
     * These are the X and Y calculations for the parts of the grid that
     * are dependent on the division by 3 calculations.
     */

    /*
     * Y dimension third.
     */
    ythird = (lry - uly) / 3;
    y1third = uly + ythird;
    y2third = y1third + ythird;
   
    /*
     * X dimension third.
     */
    xthird = (lrx - ulx) / 3;
    x1third = ulx + xthird;
    x2third = x1third + xthird;

    box[4].x1 = x1third; box[4].y1 = lry;
    box[4].x2 = x1third; box[4].y2 = uly;

    box[5].x1 = x2third; box[5].y1 = lry;
    box[5].x2 = x2third; box[5].y2 = uly;

    box[6].x1 = ulx; box[6].y1 = y1third;
    box[6].x2 = lrx; box[6].y2 = y1third;

    box[7].x1 = ulx; box[7].y1 = y2third;
    box[7].x2 = lrx; box[7].y2 = y2third;

    /*
     * Do not erase if we're freezing the screen.
     */
    if (Freeze)
        return(8);

    /*
     * From here on we're retracing the segments to clear the
     * grid using GXxor.
     */
    box[8].x1 = ulx; box[8].y1 = uly;
    box[8].x2 = lrx; box[8].y2 = uly;

    box[9].x1 = lrx; box[9].y1 = uly;
    box[9].x2 = lrx; box[9].y2 = lry;

    box[10].x1 = lrx; box[10].y1 = lry;
    box[10].x2 = ulx; box[10].y2 = lry;

    box[11].x1 = ulx; box[11].y1 = lry;
    box[11].x2 = ulx; box[11].y2 = uly;

    box[12].x1 = x1third; box[12].y1 = lry;
    box[12].x2 = x1third; box[12].y2 = uly;

    box[13].x1 = x2third; box[13].y1 = lry;
    box[13].x2 = x2third; box[13].y2 = uly;

    box[14].x1 = ulx; box[14].y1 = y1third;
    box[14].x2 = lrx; box[14].y2 = y1third;

    box[15].x1 = ulx; box[15].y1 = y2third;
    box[15].x2 = lrx; box[15].y2 = y2third;


    /*
     * Total number of segments is 16.
     */
    return(16);
}
