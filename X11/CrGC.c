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
 * Xlibemu
 *
 * Copyright 1993, 1994
 * Ulrich Leodolter, Austria 3051 St.Christophen, Ludmerfeld 33.
 *
 * This software may be freely copied, modified and redistributed without
 * fee provided that this copyright notice is preserved intact on all
 * copies and modified copies.
 *
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage or
 * its effect upon hardware or computer systems.
 */
/* $XConsortium: CrGC.c /main/26 1996/10/22 14:17:09 kaleb $ */
/*

Copyright (c) 1986  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include "Xlibint.h"

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

static XGCValues Const initial_GC = {
    GXcopy, 	/* function */
    AllPlanes,	/* plane_mask */
    0L,		/* foreground */
    1L,		/* background */
    0,		/* line_width */
    LineSolid,	/* line_style */
    CapButt,	/* cap_style */
    JoinMiter,	/* join_style */
    FillSolid,	/* fill_style */
    EvenOddRule,/* fill_rule */
    ArcPieSlice,/* arc_mode */
    (Pixmap)~0L,/* tile, impossible (unknown) resource */
    (Pixmap)~0L,/* stipple, impossible (unknown) resource */
    0,		/* ts_x_origin */
    0,		/* ts_y_origin */
    (Font)~0L,	/* font, impossible (unknown) resource */
    ClipByChildren, /* subwindow_mode */
    True,	/* graphics_exposures */
    0,		/* clip_x_origin */
    0,		/* clip_y_origin */
    None,	/* clip_mask */
    0,		/* dash_offset */
    4		/* dashes (list [4,4]) */
};

static void _XGenerateGCList();

GC XCreateGC (dpy, d, valuemask, values)
     register Display *dpy;
     Drawable d;		/* Window or Pixmap for which depth matches */
     unsigned long valuemask;	/* which ones to set initially */
     XGCValues *values;		/* the values themselves */
{
    register GC gc;
    register xCreateGCReq *req;
    register _XExtension *ext;

    LockDisplay(dpy);
    if ((gc = (GC)Xmalloc (sizeof(struct _XGC))) == NULL) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (NULL);
    }
    gc->rects = 0;
    gc->dashes = 0;
    gc->ext_data = NULL;
    gc->values = initial_GC;
    gc->dirty = 0L;

    valuemask &= (1L << (GCLastBit + 1)) - 1;
    if (valuemask) _XUpdateGCCache (gc, valuemask, values);

    __XcreateGCid(dpy, d, gc);

//    if(gc->dirty)
//       _XFlushGCCache(dpy, gc);

    UnlockDisplay(dpy);
    SyncHandle();
    return (gc);
    }


_XUpdateGCCache (gc, mask, attr)
    register unsigned long mask;
    register XGCValues *attr;
    register GC gc;
    {
    register XGCValues *gv = &gc->values;

    if (mask & GCFunction)
        if (gv->function != attr->function) {
	  gv->function = attr->function;
	  gc->dirty |= GCFunction;
	}
	
    if (mask & GCPlaneMask)
        if (gv->plane_mask != attr->plane_mask) {
            gv->plane_mask = attr->plane_mask;
	    gc->dirty |= GCPlaneMask;
	  }

    if (mask & GCForeground)
        if (gv->foreground != attr->foreground) {
            gv->foreground = attr->foreground;
	    gc->dirty |= GCForeground;
	  }

    if (mask & GCBackground)
        if (gv->background != attr->background) {
            gv->background = attr->background;
	    gc->dirty |= GCBackground;
	  }

    if (mask & GCLineWidth)
        if (gv->line_width != attr->line_width) {
            gv->line_width = attr->line_width;
	    gc->dirty |= GCLineWidth;
	  }

    if (mask & GCLineStyle)
        if (gv->line_style != attr->line_style) {
            gv->line_style = attr->line_style;
	    gc->dirty |= GCLineStyle;
	  }

    if (mask & GCCapStyle)
        if (gv->cap_style != attr->cap_style) {
            gv->cap_style = attr->cap_style;
	    gc->dirty |= GCCapStyle;
	  }
    
    if (mask & GCJoinStyle)
        if (gv->join_style != attr->join_style) {
            gv->join_style = attr->join_style;
	    gc->dirty |= GCJoinStyle;
	  }

    if (mask & GCFillStyle)
        if (gv->fill_style != attr->fill_style) {
            gv->fill_style = attr->fill_style;
	    gc->dirty |= GCFillStyle;
	  }

    if (mask & GCFillRule)
        if (gv->fill_rule != attr->fill_rule) {
    	    gv->fill_rule = attr->fill_rule;
	    gc->dirty |= GCFillRule;
	  }

    if (mask & GCArcMode)
        if (gv->arc_mode != attr->arc_mode) {
	    gv->arc_mode = attr->arc_mode;
	    gc->dirty |= GCArcMode;
	  }

    /* always write through tile change, since client may have changed pixmap contents */
    if (mask & GCTile) {
	    gv->tile = attr->tile;
	    gc->dirty |= GCTile;
	  }

    /* always write through stipple change, since client may have changed pixmap contents */
    if (mask & GCStipple) {
	    gv->stipple = attr->stipple;
	    gc->dirty |= GCStipple;
	  }

    if (mask & GCTileStipXOrigin)
        if (gv->ts_x_origin != attr->ts_x_origin) {
    	    gv->ts_x_origin = attr->ts_x_origin;
	    gc->dirty |= GCTileStipXOrigin;
	  }

    if (mask & GCTileStipYOrigin)
        if (gv->ts_y_origin != attr->ts_y_origin) {
	    gv->ts_y_origin = attr->ts_y_origin;
	    gc->dirty |= GCTileStipYOrigin;
	  }

    if (mask & GCFont)
        if (gv->font != attr->font) {
	    gv->font = attr->font;
	    gc->dirty |= GCFont;
	  }

    if (mask & GCSubwindowMode)
        if (gv->subwindow_mode != attr->subwindow_mode) {
	    gv->subwindow_mode = attr->subwindow_mode;
	    gc->dirty |= GCSubwindowMode;
	  }

    if (mask & GCGraphicsExposures)
        if (gv->graphics_exposures != attr->graphics_exposures) {
	    gv->graphics_exposures = attr->graphics_exposures;
	    gc->dirty |= GCGraphicsExposures;
	  }

    if (mask & GCClipXOrigin)
        if (gv->clip_x_origin != attr->clip_x_origin) {
	    gv->clip_x_origin = attr->clip_x_origin;
	    gc->dirty |= GCClipXOrigin;
	  }

    if (mask & GCClipYOrigin)
        if (gv->clip_y_origin != attr->clip_y_origin) {
	    gv->clip_y_origin = attr->clip_y_origin;
	    gc->dirty |= GCClipYOrigin;
	  }

    /* always write through mask change, since client may have changed pixmap contents */
    if (mask & GCClipMask) {
	    gv->clip_mask = attr->clip_mask;
	    gc->dirty |= GCClipMask;
	    gc->rects = 0;
	  }

    if (mask & GCDashOffset)
        if (gv->dash_offset != attr->dash_offset) {
	    gv->dash_offset = attr->dash_offset;
	    gc->dirty |= GCDashOffset;
	  }

    if (mask & GCDashList)
        if ((gv->dashes != attr->dashes) || (gc->dashes == True)) {
            gv->dashes = attr->dashes;
	    gc->dirty |= GCDashList;
	    gc->dashes = 0;
	    }
    return;
    }


void XFlushGC(dpy, gc)
     Display *dpy;
     GC gc;
{
    FlushGC(dpy, gc);
}

GContext XGContextFromGC(gc)
    GC gc;
    { return (gc->gid); }


