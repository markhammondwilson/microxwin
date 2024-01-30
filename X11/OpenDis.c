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
/* $XConsortium: OpenDis.c /main/110 1996/02/02 14:09:01 kaleb $ */
/* $XFree86: xc/lib/X11/OpenDis.c,v 3.3 1996/02/04 08:54:22 dawes Exp $ */
/*

Copyright (c) 1985, 1986  X Consortium

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

#define NEED_REPLIES
#define NEED_EVENTS
#include "Xlibint.h"
#include <X11/Xatom.h>
#include <stdio.h>
#include <X11/x11ioctl.h>
#include <X11/xlibglue.h>

#ifdef X_NOT_STDC_ENV
extern char *getenv();
#endif

#ifdef XTHREADS
#include "locking.h"
int  (*_XInitDisplayLock_fn)(Display *dpy) = NULL;
void (*_XFreeDisplayLock_fn)(Display *dpy) = NULL;

#define InitDisplayLock(d)	(_XInitDisplayLock_fn ? (*_XInitDisplayLock_fn)(d) : Success)
#define FreeDisplayLock(d)	if (_XFreeDisplayLock_fn) (*_XFreeDisplayLock_fn)(d)
#else
#define InitDisplayLock(dis) Success
#define FreeDisplayLock(dis)
#endif /* XTHREADS */

static void OutOfMemory ();
/* 
 * Connects to a server, creates a Display object and returns a pointer to
 * the newly created Display back to the caller.
 */
#if NeedFunctionPrototypes
Display *XOpenDisplay (
	register _Xconst char *display)
#else
Display *XOpenDisplay (display)
	register char *display;
#endif
{
	register Display *dpy;		/* New Display object being created. */
	register int i;
	char *display_name;		/* pointer to display name */
	register Screen *sp;
	Depth *dp;
	XGCValues values;

	/*
	 * If the display specifier string supplied as an argument to this 
	 * routine is NULL or a pointer to NULL, read the DISPLAY variable.
	 */
	if (display == NULL || *display == '\0') {
		if ((display_name = getenv("DISPLAY")) == NULL) {
			/* Oops! No DISPLAY environment variable - error. */
			return(NULL);
		}
	}
	else {
		/* Display is non-NULL, copy the pointer */
		display_name = (char *)display;
	}
/*
 * Set the default error handlers.  This allows the global variables to
 * default to NULL for use with shared libraries.
 */
	if (_XErrorFunction == NULL) (void) XSetErrorHandler (NULL);
	if (_XIOErrorFunction == NULL) (void) XSetIOErrorHandler (NULL);

/*
 * Attempt to allocate a display structure. Return NULL if allocation fails.
 */
	if ((dpy = (Display *)Xcalloc(1, sizeof(Display))) == NULL) {
		return(NULL);
	}


	/* Initialize as much of the display structure as we can.
	 * Initialize pointers to NULL so that XFreeDisplayStructure will
	 * work if we run out of memory before we finish initializing.
	 */
	dpy->display_name	= strdup(display_name);
	dpy->keysyms		= (KeySym *) NULL;
	dpy->modifiermap	= NULL;
	dpy->lock_meaning	= NoSymbol;
	dpy->keysyms_per_keycode = 0;
	dpy->xdefaults		= (char *)NULL;
	dpy->scratch_length	= 0L;
	dpy->scratch_buffer	= NULL;
	dpy->key_bindings	= NULL;
	dpy->ext_procs		= (_XExtension *)NULL;
	dpy->ext_data		= (XExtData *)NULL;
	dpy->ext_number 	= 0;
	dpy->db 		= (struct _XrmHashBucketRec *)NULL;
	dpy->cursor_font	= None;
	dpy->flags		= 0;
	dpy->async_handlers	= NULL;
	dpy->screens		= NULL;
	dpy->vendor		= NULL;
	dpy->buffer		= NULL;
	dpy->atoms		= NULL;
	dpy->error_vec		= NULL;
	dpy->context_db		= NULL;
	dpy->free_funcs		= NULL;
	dpy->pixmap_format	= NULL;
	dpy->cms.clientCmaps	= NULL;
	dpy->cms.defaultCCCs	= NULL;
	dpy->cms.perVisualIntensityMaps = NULL;
	dpy->im_filters		= NULL;
 	dpy->bigreq_size	= 0;
	dpy->lock		= NULL;
	dpy->lock_fns		= NULL;
	dpy->qfree		= NULL;
	dpy->next_event_serial_num = 1;
	dpy->im_fd_info		= NULL;
	dpy->im_fd_length	= 0;
	dpy->conn_watchers	= NULL;
	dpy->watcher_count	= 0;
	dpy->filedes		= NULL;
	dpy->flushes		= NULL;
	dpy->xcmisc_opcode	= 0;
	dpy->xkb_info		= NULL;

/* set up by server */
	dpy->proto_major_version= 11;
	dpy->proto_minor_version= 6;
	dpy->motion_buffer	= 0;
	dpy->max_request_size	= 256*1024;
	dpy->release 		= 11;
/*	dpy->resource_base	= u.setup->ridBase;
	dpy->resource_mask	= u.setup->ridMask;
	dpy->nformats		= u.setup->numFormats;
	dpy->nscreens		= u.setup->numRoots;
	dpy->byte_order		= u.setup->imageByteOrder;
	dpy->bitmap_unit	= u.setup->bitmapScanlineUnit;
	dpy->bitmap_pad		= u.setup->bitmapScanlinePad;
	dpy->bitmap_bit_order   = u.setup->bitmapBitOrder; */
/*
 * Setup other information in this display structure.
 */
	dpy->vendor= strdup("MicroXwin Release 1.2");
	dpy->vnumber = X_PROTOCOL;
	dpy->resource_alloc = _XAllocID;
	dpy->synchandler = NULL;
	dpy->savedsynchandler = NULL;
	dpy->request = 0;
	dpy->last_request_read = 0;
	dpy->last_req = 0;
	dpy->default_screen = 0;

	/* Initialize the display lock */
	if (InitDisplayLock(dpy) != 0) {
	        OutOfMemory (dpy);
		return(NULL);
	}
	if (!_XPollfdCacheInit(dpy)) {
	        OutOfMemory (dpy);
		return(NULL);
	}	

	/* Set up the input event queue and input event queue parameters. */
	dpy->head = dpy->tail = NULL;
	dpy->qlen = 0;

	/* Set up free-function record */
	if ((dpy->free_funcs = (_XFreeFuncRec *)Xcalloc(1,
							sizeof(_XFreeFuncRec)))
	    == NULL) {
	    OutOfMemory (dpy);
	    return(NULL);
	}

/*
 * Now iterate down setup information.....
 */
	dpy->nformats=2;	/* 1,8,16,32 bits/ pixel */
	dpy->pixmap_format = 
	    (ScreenFormat *)Xmalloc(
		(unsigned) (dpy->nformats *sizeof(ScreenFormat)));
	if (dpy->pixmap_format == NULL) {
	        OutOfMemory (dpy);
		return(NULL);
	}

/*
 * next the Screen structures.
 */
	dpy->nscreens=1;
	dpy->screens = 
	    (Screen *)Xmalloc((unsigned) dpy->nscreens*sizeof(Screen));
	if (dpy->screens == NULL) {
	        OutOfMemory (dpy);
		return(NULL);
	}
/*
 * Now go deal with each screen structure.
 */
	sp = &dpy->screens[0];
	sp->display= dpy;
	sp->ndepths= 1;
	sp->ext_data   = NULL;
/*
 * lets set up the depth structures.
 */
        sp->depths = (Depth *)Xmalloc(
			(unsigned)sp->ndepths*sizeof(Depth));
        if (sp->depths == NULL) {
     		OutOfMemory (dpy);
     		return(NULL);
        }
	sp->root_visual =(Visual *)Xmalloc(sizeof(Visual));
	dp = &sp->depths[0];
	dp->nvisuals = 1;
	dp->visuals=sp->root_visual;

	if(__Open(dpy))
           {
           OutOfMemory (dpy);
	   return (NULL);
           }
	dp->depth=sp->root_depth;
	values.foreground = sp->black_pixel;
	values.background = sp->white_pixel;
	if ((sp->default_gc = XCreateGC (dpy, sp->root,
					     GCForeground|GCBackground,
					     &values)) == NULL) {
		OutOfMemory (dpy);
		return (NULL);
	}
 	return(dpy);
}
/* XFreeDisplayStructure frees all the storage associated with a 
 * Display.  It is used by XOpenDisplay if it runs out of memory,
 * and also by XCloseDisplay.   It needs to check whether all pointers
 * are non-NULL before dereferencing them, since it may be called
 * by XOpenDisplay before the Display structure is fully formed.
 * XOpenDisplay must be sure to initialize all the pointers to NULL
 * before the first possible call on this.
 */

void _XFreeDisplayStructure(dpy)
	register Display *dpy;
{
	while (dpy->ext_procs) {
	    _XExtension *ext = dpy->ext_procs;
	    dpy->ext_procs = ext->next;
	    if (ext->name)
		Xfree (ext->name);
	    Xfree ((char *)ext);
	}
	if (dpy->im_filters)
	   (*dpy->free_funcs->im_filters)(dpy);
	if (dpy->cms.clientCmaps)
	   (*dpy->free_funcs->clientCmaps)(dpy);
	if (dpy->cms.defaultCCCs)
	   (*dpy->free_funcs->defaultCCCs)(dpy);
	if (dpy->cms.perVisualIntensityMaps)
	   (*dpy->free_funcs->intensityMaps)(dpy);
	if (dpy->atoms)
	    (*dpy->free_funcs->atoms)(dpy);
	if (dpy->modifiermap)
	   (*dpy->free_funcs->modifiermap)(dpy->modifiermap);
	if (dpy->key_bindings)
	   (*dpy->free_funcs->key_bindings)(dpy);
	if (dpy->context_db)
	   (*dpy->free_funcs->context_db)(dpy);
	if (dpy->xkb_info)
	   (*dpy->free_funcs->xkb)(dpy);

	if (dpy->screens) {
	    register int i;

            for (i = 0; i < dpy->nscreens; i++) {
		Screen *sp = &dpy->screens[i];

		if (sp->depths) {
		   register int j;

		   for (j = 0; j < sp->ndepths; j++) {
			Depth *dp = &sp->depths[j];

			if (dp->visuals) {
			   register int k;

			   for (k = 0; k < dp->nvisuals; k++)
			     _XFreeExtData (dp->visuals[k].ext_data);
			   Xfree ((char *) dp->visuals);
			   }
			}

		   Xfree ((char *) sp->depths);
		   }

		_XFreeExtData (sp->ext_data);
		}

	    Xfree ((char *)dpy->screens);
	    }
	
	if (dpy->pixmap_format) {
	    register int i;

	    for (i = 0; i < dpy->nformats; i++)
	      _XFreeExtData (dpy->pixmap_format[i].ext_data);
            Xfree ((char *)dpy->pixmap_format);
	    }

	if (dpy->display_name)
	   Xfree (dpy->display_name);
	if (dpy->vendor)
	   Xfree (dpy->vendor);

        if (dpy->buffer)
	   Xfree (dpy->buffer);
	if (dpy->keysyms)
	   Xfree ((char *) dpy->keysyms);
	if (dpy->xdefaults)
	   Xfree (dpy->xdefaults);
	if (dpy->error_vec)
	    Xfree ((char *)dpy->error_vec);

	_XFreeExtData (dpy->ext_data);
	if (dpy->free_funcs)
	    Xfree ((char *)dpy->free_funcs);
 	if (dpy->scratch_buffer)
 	    Xfree (dpy->scratch_buffer);
	FreeDisplayLock(dpy);

	if (dpy->qfree) {
	    register _XQEvent *qelt = dpy->qfree;

	    while (qelt) {
		register _XQEvent *qnxt = qelt->next;
		Xfree ((char *) qelt);
		qelt = qnxt;
	    }
	}
	while (dpy->im_fd_info) {
	    struct _XConnectionInfo *conni = dpy->im_fd_info;
	    dpy->im_fd_info = conni->next;
	    if (conni->watch_data)
		Xfree (conni->watch_data);
	    Xfree (conni);
	}
	if (dpy->conn_watchers) {
	    struct _XConnWatchInfo *watcher = dpy->conn_watchers;
	    dpy->conn_watchers = watcher->next;
	    Xfree (watcher);
	}
	if (dpy->filedes)
	    Xfree (dpy->filedes);

        Xfree(((_XPrivDisplay)dpy)->private11);
	Xfree ((char *)dpy);
}

/* OutOfMemory is called if malloc fails.  XOpenDisplay returns NULL
   after this returns. */

static void OutOfMemory (dpy)
    Display *dpy;
{
    _XFreeDisplayStructure (dpy);
}
/*
 * Support for XOpenDisplay.
 */
int __Open(Display *dpy)
{
   Screen *scr;
   Visual *visual;
   ScreenFormat *pfmt;
   XSetWindowAttributes attr;
   X11Info xinfo;
   char *dpisettings;		/* dpi setting */
   int dpi;
   
   scr=dpy->screens;
   visual=scr->root_visual;
   pfmt=dpy->pixmap_format;

   /* Async this is the command buffer */
   ((_XPrivDisplay)dpy)->private11 =(XPointer )Xmalloc(CMDBUFSIZE*sizeof(int));
   if (((_XPrivDisplay)dpy)->private11 == NULL)
      return(-1);
   ((_XPrivDisplay)dpy)->private8=0; /* command count */
   /* open graphics device and ioctl graphics information */
   if(__XOpen(dpy, &xinfo))
      return(-1);

   dpy->resource_id= 4;
   dpy->resource_shift=xinfo.res_shift;
   dpy->resource_base=xinfo.res_base;
   dpy->min_keycode = xinfo.min_keycode;
   dpy->max_keycode = xinfo.max_keycode;
   dpy->byte_order=LSBFirst;	/* screen byte order, LSBFirst, MSBFirst */
   dpy->bitmap_unit=8;		/* padding and data requirements */
   dpy->bitmap_pad=8;		/* padding requirements on bitmaps */
   dpy->bitmap_bit_order=LSBFirst;	/* LeastSignificant or MostSignificant */
   dpy->motion_buffer= xinfo.motionbufsize;
   pfmt->depth=1;      
   pfmt->bits_per_pixel=1; 
   pfmt->scanline_pad=8;   
   pfmt->ext_data = NULL;

   pfmt++;
   pfmt->depth= 16;      
   pfmt->bits_per_pixel= 16; 
   pfmt->scanline_pad=32; 
   pfmt->ext_data = NULL;
#if 0
   pfmt++;
   pfmt->depth= 8;      
   pfmt->bits_per_pixel= 8; 
   pfmt->scanline_pad=32;   	/* 4 byte allignment for images & pixmap */
   pfmt->ext_data = NULL;

   pfmt++;
   pfmt->depth= 32;      
   pfmt->bits_per_pixel= 32; 
   pfmt->scanline_pad=32;   	/* 4 byte allignment for images & pixmap */
   pfmt->ext_data = NULL;
#endif
   scr->width	= xinfo.width;
   scr->height	= xinfo.height;
   /*
    * Assume defaulat 75 dpi monitor, matches X.org.
    * User can change this with DISPLAYDPI environment.
    */
   dpi=750;
   if((dpisettings = getenv("DISPLAYDPI")) != NULL)
      dpi=atoi(dpisettings)*10;
   scr->mwidth	= (scr->width*254) / dpi;
   scr->mheight	= (scr->height*254) / dpi;
   scr->root_depth = xinfo.bitsperpixel;

   visual->class = xinfo.class;
   visual->map_entries = 64; /* 64 enetris per color modeinfo.map_entries */
   visual->bits_per_rgb = xinfo.bits_per_rgb;
   visual->red_mask= xinfo.red_mask; 
   visual->green_mask= xinfo.green_mask; 
   visual->blue_mask= xinfo.blue_mask;
   visual->ext_data = NULL;
   visual->visualid = 1;	/* visual id of this visual */
   scr->white_pixel = xinfo.white;
   scr->black_pixel = xinfo.black;
   scr->cmap=xinfo.cmapid;
   scr->root=xinfo.rootid;
   scr->max_maps=scr->min_maps=1;
   scr->root_input_mask = xinfo.root_mask;
   scr->backing_store= 0;
   scr->save_unders = 0;
   return(0);
}
