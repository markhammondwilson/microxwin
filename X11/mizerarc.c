/* $Id: mizerarc.c,v 1.1.1.1 2004/10/28 11:13:32 lleukkun Exp $ */
/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author:  Bob Scheifler, MIT X Consortium

********************************************************/

/* $Xorg: mizerarc.c,v 1.4 2001/02/09 02:05:22 xorgcvs Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include <math.h>
#include <X11/X.h>
#include <X11/Xprotostr.h>
#include "Xlibint.h"
#include "draw.h"
#include "mizerarc.h"

#define FULLCIRCLE (360 * 64)
#define OCTANT (45 * 64)
#define QUADRANT (90 * 64)
#define HALFCIRCLE (180 * 64)
#define QUADRANT3 (270 * 64)

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define Dsin(d)	((d) == 0 ? 0.0 : ((d) == QUADRANT ? 1.0 : \
		 ((d) == HALFCIRCLE ? 0.0 : \
		 ((d) == QUADRANT3 ? -1.0 : sin((double)d*(M_PI/11520.0))))))

#define Dcos(d)	((d) == 0 ? 1.0 : ((d) == QUADRANT ? 0.0 : \
		 ((d) == HALFCIRCLE ? -1.0 : \
		 ((d) == QUADRANT3 ? 0.0 : cos((double)d*(M_PI/11520.0))))))

#define EPSILON45 64

typedef struct {
    int skipStart;
    int haveStart;
    DDXPointRec startPt;
    int haveLast;
    int skipLast;
    DDXPointRec endPt;
    int dashIndex;
    int dashOffset;
    int dashIndexInit;
    int dashOffsetInit;
} DashInfo;

static miZeroArcPtRec oob = {65536, 65536, 0};

/*
 * (x - l)^2 / (W/2)^2  + (y + H/2)^2 / (H/2)^2 = 1
 *
 * where l is either 0 or .5
 *
 * alpha = 4(W^2)
 * beta = 4(H^2)
 * gamma = 0
 * u = 2(W^2)H
 * v = 4(H^2)l
 * k = -4(H^2)(l^2)
 *
 */

Bool
miZeroArcSetup(arc, info, ok360)
    register xArc *arc;
    register miZeroArcRec *info;
    Bool ok360;
{
    int l;
    int angle1, angle2;
    int startseg, endseg;
    int startAngle, endAngle;
    int i, overlap;
    miZeroArcPtRec start, end;

    l = arc->width & 1;
    if (arc->width == arc->height)
    {
	info->alpha = 4;
	info->beta = 4;
	info->k1 = -8;
	info->k3 = -16;
	info->b = 12;
	info->a = (arc->width << 2) - 12;
	info->d = 17 - (arc->width << 1);
	if (l)
	{
	    info->b -= 4;
	    info->a += 4;
	    info->d -= 7;
	}
    }
    else if (!arc->width || !arc->height)
    {
	info->alpha = 0;
	info->beta = 0;
	info->k1 = 0;
	info->k3 = 0;
	info->a = -(int)arc->height;
	info->b = 0;
	info->d = -1;
    }
    else
    {
	/* initial conditions */
	info->alpha = (arc->width * arc->width) << 2;
	info->beta = (arc->height * arc->height) << 2;
	info->k1 = info->beta << 1;
	info->k3 = info->k1 + (info->alpha << 1);
	info->b = l ? 0 : -info->beta;
	info->a = info->alpha * arc->height;
	info->d = info->b - (info->a >> 1) - (info->alpha >> 2);
	if (l)
	    info->d -= info->beta >> 2;
	info->a -= info->b;
	/* take first step, d < 0 always */
	info->b -= info->k1;
	info->a += info->k1;
	info->d += info->b;
	/* octant change, b < 0 always */
	info->k1 = -info->k1;
	info->k3 = -info->k3;
	info->b = -info->b;
	info->d = info->b - info->a - info->d;
	info->a = info->a - (info->b << 1);
    }
    info->dx = 1;
    info->dy = 0;
    info->w = (arc->width + 1) >> 1;
    info->h = arc->height >> 1;
    info->xorg = arc->x + (arc->width >> 1);
    info->yorg = arc->y;
    info->xorgo = info->xorg + l;
    info->yorgo = info->yorg + arc->height;
    if (!arc->width)
    {
	if (!arc->height)
	{
	    info->x = 0;
	    info->y = 0;
	    info->initialMask = 0;
	    info->startAngle = 0;
	    info->endAngle = 0;
	    info->start = oob;
	    info->end = oob;
	    return FALSE;
	}
	info->x = 0;
	info->y = 1;
    }
    else
    {
	info->x = 1;
	info->y = 0;
    }
    angle1 = arc->angle1;
    angle2 = arc->angle2;
    if ((angle1 == 0) && (angle2 >= FULLCIRCLE))
    {
	startAngle = 0;
	endAngle = 0;
    }
    else
    {
	if (angle2 > FULLCIRCLE)
	    angle2 = FULLCIRCLE;
	else if (angle2 < -FULLCIRCLE)
	    angle2 = -FULLCIRCLE;
	if (angle2 < 0)
	{
	    startAngle = angle1 + angle2;
	    endAngle = angle1;
	}
	else
	{
	    startAngle = angle1;
	    endAngle = angle1 + angle2;
	}
	if (startAngle < 0)
	    startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
	if (startAngle >= FULLCIRCLE)
	    startAngle = startAngle % FULLCIRCLE;
	if (endAngle < 0)
	    endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
	if (endAngle >= FULLCIRCLE)
	    endAngle = endAngle % FULLCIRCLE;
    }
    info->startAngle = startAngle;
    info->endAngle = endAngle;
    if (ok360 && (startAngle == endAngle) && arc->angle2 &&
	arc->width && arc->height)
    {
	info->initialMask = 0xf;
	info->start = oob;
	info->end = oob;
	return TRUE;
    }
    startseg = startAngle / OCTANT;
    if (!arc->height || (((startseg + 1) & 2) && arc->width))
    {
	start.x = Dcos(startAngle) * ((arc->width + 1) / 2.0);
	if (start.x < 0)
	    start.x = -start.x;
	start.y = -1;
    }
    else
    {
	start.y = Dsin(startAngle) * (arc->height / 2.0);
	if (start.y < 0)
	    start.y = -start.y;
	start.y = info->h - start.y;
	start.x = 65536;
    }
    endseg = endAngle / OCTANT;
    if (!arc->height || (((endseg + 1) & 2) && arc->width))
    {
	end.x = Dcos(endAngle) * ((arc->width + 1) / 2.0);
	if (end.x < 0)
	    end.x = -end.x;
	end.y = -1;
    }
    else
    {
	end.y = Dsin(endAngle) * (arc->height / 2.0);
	if (end.y < 0)
	    end.y = -end.y;
	end.y = info->h - end.y;
	end.x = 65536;
    }
    info->firstx = start.x;
    info->firsty = start.y;
    info->initialMask = 0;
    overlap = arc->angle2 && (endAngle <= startAngle);
    for (i = 0; i < 4; i++)
    {
	if (overlap ?
	    ((i * QUADRANT <= endAngle) || ((i + 1) * QUADRANT > startAngle)) :
	    ((i * QUADRANT <= endAngle) && ((i + 1) * QUADRANT > startAngle)))
	    info->initialMask |= (1 << i);
    }
    start.mask = info->initialMask;
    end.mask = info->initialMask;
    startseg >>= 1;
    endseg >>= 1;
    overlap = overlap && (endseg == startseg);
    if (start.x != end.x || start.y != end.y || !overlap)
    {
	if (startseg & 1)
	{
	    if (!overlap)
		info->initialMask &= ~(1 << startseg);
	    if (start.x > end.x || start.y > end.y)
		end.mask &= ~(1 << startseg);
	}
	else
	{
	    start.mask &= ~(1 << startseg);
	    if (((start.x < end.x || start.y < end.y) ||
		 (start.x == end.x && start.y == end.y && (endseg & 1))) &&
		!overlap)
		end.mask &= ~(1 << startseg);
	}
	if (endseg & 1)
	{
	    end.mask &= ~(1 << endseg);
	    if (((start.x > end.x || start.y > end.y) ||
		 (start.x == end.x && start.y == end.y && !(startseg & 1))) &&
		!overlap)
		start.mask &= ~(1 << endseg);
	}
	else
	{
	    if (!overlap)
		info->initialMask &= ~(1 << endseg);
	    if (start.x < end.x || start.y < end.y)
		start.mask &= ~(1 << endseg);
	}
    }
    /* take care of case when start and stop are both near 45 */
    /* handle here rather than adding extra code to pixelization loops */
    if (startAngle &&
	((start.y < 0 && end.y >= 0) || (start.y >= 0 && end.y < 0)))
    {
	i = (startAngle + OCTANT) % OCTANT;
	if (i < EPSILON45 || i > OCTANT - EPSILON45)
	{
	    i = (endAngle + OCTANT) % OCTANT;
	    if (i < EPSILON45 || i > OCTANT - EPSILON45)
	    {
		if (start.y < 0)
		{
		    i = Dsin(startAngle) * (arc->height / 2.0);
		    if (i < 0)
			i = -i;
		    if (info->h - i == end.y)
			start.mask = end.mask;
		}
		else
		{
		    i = Dsin(endAngle) * (arc->height / 2.0);
		    if (i < 0)
			i = -i;
		    if (info->h - i == start.y)
			end.mask = start.mask;
		}
	    }
	}
    }
    if (startseg & 1)
    {
	info->start = start;
	info->end = oob;
    }
    else
    {
	info->end = start;
	info->start = oob;
    }
    if (endseg & 1)
    {
	info->altend = end;
	if (info->altend.x < info->end.x || info->altend.y < info->end.y)
	{
	    miZeroArcPtRec tmp;
	    tmp = info->altend;
	    info->altend = info->end;
	    info->end = tmp;
	}
	info->altstart = oob;
    }
    else
    {
	info->altstart = end;
	if (info->altstart.x < info->start.x ||
	    info->altstart.y < info->start.y)
	{
	    miZeroArcPtRec tmp;
	    tmp = info->altstart;
	    info->altstart = info->start;
	    info->start = tmp;
	}
	info->altend = oob;
    }
    if (!info->start.x || !info->start.y)
    {
	info->initialMask = info->start.mask;
	info->start = info->altstart;
    }
    if (!arc->width && (arc->height == 1))
    {
	/* kludge! */
	info->initialMask |= info->end.mask;
	info->initialMask |= info->initialMask << 1;
	info->end.x = 0;
	info->end.mask = 0;
    }
    return FALSE;
}

#define Pixelate(xval,yval) \
    { \
	pts->x = xval; \
	pts->y = yval; \
	pts++; \
    }

#define DoPix(idx,xval,yval) if (mask & (1 << idx)) Pixelate(xval, yval);

DDXPointPtr
miZeroArcPts(arc, pts)
    xArc *arc;
    register DDXPointPtr pts;
{
    miZeroArcRec info;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx, dy;
    Bool do360;

    do360 = miZeroArcSetup(arc, &info, TRUE);
    MIARCSETUP();
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(1, info.xorgo, info.yorg);
	DoPix(3, info.xorgo, info.yorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int yorgh = info.yorg + info.h;
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

	while (1)
	{
	    Pixelate(info.xorg + x, info.yorg + y);
	    Pixelate(info.xorg - x, info.yorg + y);
	    Pixelate(info.xorg - x, info.yorgo - y);
	    Pixelate(info.xorg + x, info.yorgo - y);
	    if (a < 0)
		break;
	    Pixelate(xorghp - y, yorgh - x);
	    Pixelate(xorghn + y, yorgh - x);
	    Pixelate(xorghn + y, yorgh + x);
	    Pixelate(xorghp - y, yorgh + x);
	    MIARCCIRCLESTEP(;);
	}
	if (x > 1 && pts[-1].x == pts[-5].x && pts[-1].y == pts[-5].y)
	    pts -= 4;
	x = info.w;
	y = info.h;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(;);
	    Pixelate(info.xorg + x, info.yorg + y);
	    Pixelate(info.xorgo - x, info.yorg + y);
	    Pixelate(info.xorgo - x, info.yorgo - y);
	    Pixelate(info.xorg + x, info.yorgo - y);
	    MIARCSTEP(;,;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
	    DoPix(0, info.xorg + x, info.yorg + y);
	    DoPix(1, info.xorgo - x, info.yorg + y);
	    DoPix(2, info.xorgo - x, info.yorgo - y);
	    DoPix(3, info.xorg + x, info.yorgo - y);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(;,;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    DoPix(0, info.xorg + x, info.yorg + y);
    DoPix(2, info.xorgo - x, info.yorgo - y);
    if (arc->height & 1)
    {
	DoPix(1, info.xorgo - x, info.yorg + y);
	DoPix(3, info.xorg + x, info.yorgo - y);
    }
    return pts;
}

#undef DoPix
#define DoPix(idx,xval,yval) \
    if (mask & (1 << idx)) \
    { \
	arcPts[idx]->x = xval; \
	arcPts[idx]->y = yval; \
	arcPts[idx]++; \
    }

/*
 * Draw single arc.
 */
int
XDrawArc(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    int			angle1,
    int			angle2)
{
    XArc		arc;
    arc.x=x;
    arc.y=y;
    arc.width=width;
    arc.height=height;
    arc.angle1=angle1;
    arc.angle2=angle2;
    return(XDrawArcs(display, d, gc, &arc, 1));
}
/*
 * Draw multiple arcs.
 */
int
XDrawArcs(
    Display*		display,
    Drawable		d,
    GC			gc,
    XArc		*parcs,
    int			narcs)
{
    int maxPts = 0;
    register int n, maxw = 0;
    register xArc *arc;
    register int i;
    DDXPointPtr points, pts, oddPts;
    register DDXPointPtr pt;
    int numPts;

    LockDisplay(display);

    for (arc = (xArc *)parcs, i = narcs; --i >= 0; arc++)
    {
	    if (arc->width > arc->height)
		n = arc->width + (arc->height >> 1);
	    else
		n = arc->height + (arc->width >> 1);
	    if (n > maxPts)
		maxPts = n;
    }
    if (!maxPts)
        {
        UnlockDisplay(display);
	return 0;
        }
    numPts = maxPts << 2;
    points = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * numPts);
    for (arc = (xArc *)parcs, i = narcs; --i >= 0; arc++)
    {
	pts = miZeroArcPts(arc, points);
        n = pts - points;
        XDrawPoints(display, d, gc, (XPoint *)points, n, CoordModeOrigin);
    }
    DEALLOCATE_LOCAL(points);
    UnlockDisplay(display);
    return 0;
}
