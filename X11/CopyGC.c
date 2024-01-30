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
/* $XConsortium: CopyGC.c /main/9 1996/10/22 14:17:01 kaleb $ */
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

XCopyGC (dpy, srcGC, mask, destGC)
     register Display *dpy;
     unsigned long mask;		/* which ones to set initially */
     GC srcGC, destGC;
{
    register XGCValues *destgv = &destGC->values,
    		       *srcgv = &srcGC->values;

    LockDisplay(dpy);

    mask &= (1L << (GCLastBit + 1)) - 1;
    /* if some of the source values to be copied are "dirty", flush them 
       out before sending the CopyGC request. */
    if (srcGC->dirty & mask)
         _XFlushGCCache(dpy, srcGC);

    /* mark the copied values as dirty in the destination. */
    destGC->dirty |= mask;

    if (mask & GCFunction)
    	destgv->function = srcgv->function;
	
    if (mask & GCPlaneMask)
        destgv->plane_mask = srcgv->plane_mask;

    if (mask & GCForeground)
        destgv->foreground = srcgv->foreground;

    if (mask & GCBackground)
        destgv->background = srcgv->background;

    if (mask & GCLineWidth)
        destgv->line_width = srcgv->line_width;

    if (mask & GCLineStyle)
        destgv->line_style = srcgv->line_style;

    if (mask & GCCapStyle)
        destgv->cap_style = srcgv->cap_style;
    
    if (mask & GCJoinStyle)
        destgv->join_style = srcgv->join_style;

    if (mask & GCFillStyle)
    	destgv->fill_style = srcgv->fill_style;

    if (mask & GCFillRule) 
        destgv->fill_rule = srcgv->fill_rule;

    if (mask & GCArcMode)
        destgv->arc_mode = srcgv->arc_mode;

    if (mask & GCTile)
        {
        destgv->tile = srcgv->tile;
        if(destgv->tile = 0xffffffff) /* invalid pixmap sent by GTK */
           destGC->dirty &= ~GCTile;
        }

    if (mask & GCStipple)
        {
        destgv->stipple = srcgv->stipple;
        if(destgv->stipple = 0xffffffff) /* invalid pixmap sent by GTK */
           destGC->dirty &= ~GCStipple;
        }

    if (mask & GCTileStipXOrigin)
        destgv->ts_x_origin = srcgv->ts_x_origin;

    if (mask & GCTileStipYOrigin)
        destgv->ts_y_origin = srcgv->ts_y_origin;

    if (mask & GCFont) 
        {
        destgv->font = srcgv->font;
        if(destgv->font = 0xffffffff) /* invalid font sent by GTK */
           destGC->dirty &= ~GCFont;
	}

    if (mask & GCSubwindowMode) 
        destgv->subwindow_mode = srcgv->subwindow_mode;

    if (mask & GCGraphicsExposures) 
        destgv->graphics_exposures = srcgv->graphics_exposures;

    if (mask & GCClipXOrigin) 
        destgv->clip_x_origin = srcgv->clip_x_origin;

    if (mask & GCClipYOrigin) 
        destgv->clip_y_origin = srcgv->clip_y_origin;

    if (mask & GCClipMask) {
	destGC->rects = srcGC->rects;
        destgv->clip_mask = srcgv->clip_mask;
        if(destgv->clip_mask = 0xffffffff) /* invalid pixmap sent by GTK */
           destGC->dirty &= ~GCClipMask;
	}

    if (mask & GCDashOffset) 
        destgv->dash_offset = srcgv->dash_offset;

    if (mask & GCDashList) {
	destGC->dashes = srcGC->dashes;
        destgv->dashes = srcgv->dashes;
	}
    /* now call kernel routine */
    __XCopyGC(dpy, srcGC, destGC);
    destGC->dirty = 0;
    UnlockDisplay(dpy);
    SyncHandle();
    }


