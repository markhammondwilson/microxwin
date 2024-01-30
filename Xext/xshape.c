/*
 * Copyright (C) 2000-2007  Vasant Kanchan, http://www.microxwin.com
 *
 * This software may be freely copied and redistributed without
 * fee provided that this copyright notice is preserved intact on all
 * copies and modified copies. 
 *
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage or
 * its effect upon hardware or computer systems.
 */
/*
 * $XConsortium: XShape.c,v 1.22 91/01/12 11:19:44 rws Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */
#include "Xlibint.h"
#include "region.h"			/* in Xlib sources */
/****************************************************************************
 *                                                                          *
 *			    Shape public interfaces                         *
 *                                                                          *
 ****************************************************************************/
extern int ShapeReqCode;
extern int ShapeEventBase;

Bool XShapeQueryExtension (dpy, event_basep, error_basep)
    Display *dpy;
    int *event_basep, *error_basep;
{
#ifdef SHAPE
   *event_basep = ShapeEventBase;
   *error_basep = 0;
   return True;
#else
   return False;
#endif
}


Status XShapeQueryVersion(dpy, major_versionp, minor_versionp)
    Display *dpy;
    int	    *major_versionp, *minor_versionp;
{
#ifdef SHAPE
   *major_versionp = 1;
   *minor_versionp = 0;
   return 1;
#else
   return 0;
#endif
}

void XShapeCombineRegion(dpy, dest, destKind, xOff, yOff, r, op)
register Display    *dpy;
Window		    dest;
int		    destKind, op, xOff, yOff;
register REGION	    *r;
{
    register long nbytes;
    register int i;
    register XRectangle *xr, *pr;
    register BOX *pb;

    xr = (XRectangle *)Xmalloc(r->numRects * sizeof (XRectangle));
    for (pr = xr, pb = r->rects, i = r->numRects; --i >= 0; pr++, pb++) {
        pr->x = pb->x1;
	pr->y = pb->y1;
	pr->width = pb->x2 - pb->x1;
	pr->height = pb->y2 - pb->y1;
     }
    XShapeCombineRectangles (dpy, dest, destKind, xOff, yOff,
			      xr, r->numRects, op, YXBanded);
    Xfree(xr);
    return;
}

XRectangle *XShapeGetRectangles (dpy, window, kind, count, ordering)
    register Display	*dpy;
    Window		window;
    int			kind;
    int			*count;	/* RETURN */
    int			*ordering; /* RETURN */
{
  int n, size;
  XRectangle *xr;
  
  LockDisplay(dpy);
  size=128;
retry:
  n=size;
  xr = (XRectangle *) Xmalloc (n * sizeof (XRectangle));
  if(!__XShapeGetRectangles(dpy, window, kind, xr, &n))
     {
     UnlockDisplay(dpy);
     Xfree(xr);
     return(NULL);
     }
  if(n < size)
     {
     *count = n;
     *ordering = Unsorted;
     UnlockDisplay(dpy);
     return xr;
     }
  Xfree(xr);
  size=n+1;
  goto  retry;
}


