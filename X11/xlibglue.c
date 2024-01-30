/*								 
 * Module Name - xlibglue.c
 *
 * Description -
 *     Xlib kernel interface.
 *
 * Copyright (C) 2000-2007  Vasant Kanchan, http://www.microxwin.com
 *
 * This software may be freely copied and redistributed without
 * fee provided that this copyright notice is preserved intact on all
 * copies and modified copies. 
 * 
 * However this file cannot be modified without consent of Vasant Kanchan.
 * Please contact vkanchan@microxwin.com
 *
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage or
 * its effect upon hardware or computer systems.
 */
#include "fcntl.h"
#include "Xlibint.h"
#include <X11/x11ioctl.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/render.h>
//#define XCMDDEBUG 1 
#include <X11/xlibglue.h>
typedef	unsigned short	pixel_t; 

#ifdef XCMDDEBUG
#define RAWFLUSH(dpy) __XFlush(dpy)
#else
#define RAWFLUSH(dpy) 	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 
#endif

/*
 * Copy d bytes from buffer into the command queue
 */
#define copydpy(dpy, buf, d) 	\
{				\
   memcpy( (_wrptr((dpy)) + _wrindex((dpy))), (buf), (d));		\
   _wrindex((dpy)) +=((d)+3) >> 2; /* increment index in ints */	\
}
void __XFlush(Display *dpy)
{
   if(_wrindex(dpy))
      {
      write(dpy->fd, _wrptr(dpy), _wrindex(dpy)*sizeof(int)); 
      _wrindex(dpy)=0; 
      }
   return;
}
/*
 * Support for XOpenDisplay.
 */
int __XOpen(Display *dpy, X11Info *xip)
{
   int fd;
#ifdef __NetBSD__
   int i;
   char buf[64];
   /* scan for an open device */
   for(i=0; i < 64; ++i)
      {
      sprintf(buf, "/dev/x11dev%d", i);
      fd=open(buf, O_RDWR, 0);
      if(fd < 0)
         continue;
      break;
      }
   if(i ==  64)
      return (-1);
#else
   fd=open("/dev/x11dev", O_RDWR, 0);
   if(fd < 0)
      return (-1);
#endif
   dpy->fd=fd;
   /* mark connection close on exec */
   fcntl(dpy->fd, F_SETFD, 1); 
   /* get all the display variables */
   if(ioctl(fd, X11GETPARAM, xip) < 0)
      {
      printf("Could not ioctl graphics device\n");
      return (-1);
      }
   return 0;      
}
int __XClose(Display *dpy)
{
   close(dpy->fd); /* close graphics device */
}
/*
 * Returns the number of queued events from kernel
 */
int __XPendingEvent(Display *dpy)
{
   int count=0;

   ioctl(dpy->fd, X11GETEVENTCOUNT, &count);
   return(count);
}
int __XReadKernel(Display *dpy, XEvent *event, int count)
{
   int n;
   n=read(dpy->fd, event, count*sizeof(XEvent));
   if(n <= 0)
      return 0;
   return(n/sizeof(XEvent));
}
/* Window calls */
int
XUnmapWindow (Display* dpy, Window window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 0, window);
   UnlockDisplay(dpy);
   return(code);
}
int
XUnmapSubwindows (Display* dpy, Window window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 1, window);
   UnlockDisplay(dpy);
   return(code);
}
Bool XTranslateCoordinates(
    Display*		dpy,
    Window		src_w,
    Window		dest_w,
    int			src_x,
    int			src_y,
    int*		dest_x_return,
    int*		dest_y_return,
    Window*		child_return)
{
   Bool code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC8(dpy, 2, src_w, dest_w, src_x, src_y,
       dest_x_return, dest_y_return, child_return);
   UnlockDisplay(dpy);
   return(code);
}
int
XReparentWindow (Display *dpy, Window window, Window parent, int x, int y)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD5(dpy, 3, window, parent, x, y);
   UnlockDisplay(dpy);
   return(code);
}

static int _XConfWindowChanges (values, mask, changes)
    unsigned long *values;
    register unsigned int mask;
    XWindowChanges *changes;
{
    register unsigned long *value = values;
    unsigned int nvalues;

    if (mask & CWX)
	*value++ = changes->x;
	
    if (mask & CWY)
    	*value++ = changes->y;

    if (mask & CWWidth)
    	*value++ = changes->width;

    if (mask & CWHeight)
    	*value++ = changes->height;

    if (mask & CWBorderWidth)
    	*value++ = changes->border_width;

    if (mask & CWSibling)
	*value++ = changes->sibling;

    if (mask & CWStackMode)
        *value++ = changes->stack_mode;

    nvalues = (value - values);
    return nvalues;
}
int
XConfigureWindow (Display*		dpy,
		  Window		w,
		  unsigned int		value_mask,
		  XWindowChanges*	changes)		 
{
   unsigned long values[32];
   int size;
   int code=1;

   size=_XConfWindowChanges (values, value_mask, changes);
   if(!size)
      return(code);
   LockDisplay(dpy);
   dpy->request++;
   CHKSPACE(dpy, 3 + size);
   XCMDRAW3(dpy, 4 | (( 3 + size) << 16), w, value_mask);
   copydpy(dpy, values, size*sizeof(unsigned long));
   RAWFLUSH(dpy);
   UnlockDisplay(dpy);
   return(code);
}
Status
XGetGeometry (Display*		dpy,
	      Drawable		d,
	      Window *		root_return,
	      int*		x_return,
	      int*		y_return,
	      unsigned int*	width_return,
	      unsigned int*	height_return,
	      unsigned int*	border_width_return,
	      unsigned int*	depth_return)
{
   Status code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC9(dpy, 5,  d, root_return, x_return, y_return,
	width_return, height_return, border_width_return, depth_return);
   UnlockDisplay(dpy);
   return(code);
}
int
XMapWindow (Display* dpy, Window window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 6, window);
   UnlockDisplay(dpy);
   return(code);
}
int
XMapSubwindows (Display* dpy, Window window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 7, window);
   UnlockDisplay(dpy);
   return(code);
}
Status
XGetWindowAttributes (register Display *dpy,
		      Window w,
		      XWindowAttributes *attr)
{
   Status code=1;
   Screen *scr;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 8, w, attr);
   attr->screen = dpy->screens;
   scr=attr->screen;
   attr->visual = scr->root_visual;
   UnlockDisplay(dpy);
   return(code);
}
int
XDestroyWindow (Display *dpy, Window window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 9, window);
   UnlockDisplay(dpy);
   return(code);
}
int
XDestroySubwindows (Display *dpy, Window parent)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 10, parent);
   UnlockDisplay(dpy);
   return(code);
}
int
XClearArea (Display*		dpy,
	    Window		window, 
	    int			x,
	    int			y,
	    unsigned int	width,
	    unsigned int	height,
	    Bool		exposures)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD7(dpy, 11, window, x, y, width, height, exposures);
   UnlockDisplay(dpy);
   return(code);
}
int
XCirculateSubwindows (Display *dpy, Window parent, int direction)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD3(dpy, 12, parent, direction);
   UnlockDisplay(dpy);
   return(code);
}
static int _XProcWindowAttributes (values, valuemask, attributes)
    unsigned long *values;
    register unsigned long valuemask;
    register XSetWindowAttributes *attributes;
{
    register unsigned long *value = values;
    unsigned int nvalues;

    if (valuemask & CWBackPixmap)
	*value++ = attributes->background_pixmap;
	
    if (valuemask & CWBackPixel)
    	*value++ = attributes->background_pixel;

    if (valuemask & CWBorderPixmap)
    	*value++ = attributes->border_pixmap;

    if (valuemask & CWBorderPixel)
    	*value++ = attributes->border_pixel;

    if (valuemask & CWBitGravity)
    	*value++ = attributes->bit_gravity;

    if (valuemask & CWWinGravity)
	*value++ = attributes->win_gravity;

    if (valuemask & CWBackingStore)
        *value++ = attributes->backing_store;
    
    if (valuemask & CWBackingPlanes)
	*value++ = attributes->backing_planes;

    if (valuemask & CWBackingPixel)
    	*value++ = attributes->backing_pixel;

    if (valuemask & CWOverrideRedirect)
    	*value++ = attributes->override_redirect;

    if (valuemask & CWSaveUnder)
    	*value++ = attributes->save_under;

    if (valuemask & CWEventMask)
	*value++ = attributes->event_mask;

    if (valuemask & CWDontPropagate)
	*value++ = attributes->do_not_propagate_mask;

    if (valuemask & CWColormap)
	*value++ = attributes->colormap;

    if (valuemask & CWCursor)
	*value++ = attributes->cursor;

    nvalues = (value - values);
    return nvalues;
}
Window
XCreateWindow (Display*   	dpy,
	       Window		parent,
	       int		x,
	       int		y,
	       unsigned int	width,
	       unsigned int	height,
	       unsigned int	border_width,
	       int		depth,
	       unsigned int	class,
	       Visual*		visual,
	       unsigned long	valuemask,
	       XSetWindowAttributes* attributes)
{
   unsigned long values[32];
   XID code=0;
   int size;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   size=_XProcWindowAttributes(values, valuemask, attributes);
   CHKSPACE(dpy, 11 + size);
   XCMDRAW11(dpy, 13 | ((11+size) << 16),  parent, x, y, width, height, border_width,
         depth, class, valuemask, code);
   copydpy(dpy, values, size*sizeof(unsigned long));
   RAWFLUSH(dpy);
   UnlockDisplay(dpy);
   return((Window)code);
}
int
XChangeWindowAttributes (Display*	dpy,
			 Window		window,
			 unsigned long	valuemask,
			 XSetWindowAttributes* attributes)
{
   unsigned long values[32];
   int code=1;
   int size;

   LockDisplay(dpy);
   dpy->request++;
   size=_XProcWindowAttributes(values, valuemask, attributes);
   CHKSPACE(dpy, 3 + size);
   XCMDRAW3(dpy, 14 | ((3 + size) << 16),  window, valuemask);
   copydpy(dpy, values, size*sizeof(unsigned long));
   RAWFLUSH(dpy);
   UnlockDisplay(dpy);
   return(code);
}

int __XQueryChildren(Display *dpy,
	   Window	w,
	   Window *	root_return,
	   Window *	parent_return,
	   Window *	children,
           int *	count)
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC7(dpy, 15,  w, root_return, parent_return,
	children, count, &ret);
   return ret;
}

/* color calls */
int  XAllocColor(
    Display*		dpy,
    Colormap		xidcolormap,
    XColor*		screen_in_out)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 16,  xidcolormap, screen_in_out);
   UnlockDisplay(dpy);
   return(code);
}
int XFreeColors(
    Display*		dpy,
    Colormap		xidcolormap,
    unsigned long*	pixels,
    int			npixels,
    unsigned long	planes)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC5(dpy, 17, xidcolormap, pixels, npixels,planes);
   UnlockDisplay(dpy);
   return(code);
}
int XQueryColors(
    Display*		dpy,
    Colormap		xidcolormap,
    XColor*		defs_in_out,
    int			ncolors)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC4(dpy, 18, xidcolormap, defs_in_out, ncolors);
   UnlockDisplay(dpy);
   return(code);
}

Colormap   XCreateColormap(
    Display*		dpy,
    Window		w,
    Visual		*visual,
    int			alloc)
{
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   XCMDSYNC5(dpy, 19,  w, visual, alloc, code);
   UnlockDisplay(dpy);
   return((Colormap)code);
}
int  XFreeColormap(
    Display*		dpy,
    Colormap		cmap)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 20, cmap);
   UnlockDisplay(dpy);
   return(code);
}
int  XInstallColormap(
    Display*		dpy,
    Colormap		cmap)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 21, cmap);
   UnlockDisplay(dpy);
   return(code);
}
int  XUninstallColormap(
    Display*		dpy,
    Colormap		cmap)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 22, cmap);
   UnlockDisplay(dpy);
   return(code);
}

Colormap   XCopyColormapAndFree(
    Display*		dpy,
    Colormap		cmapid)
{
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   XCMD3(dpy, 23, cmapid, code);
   UnlockDisplay(dpy);
   return((Colormap)code);
}

int XAllocColorCells(
    Display*  dpy,
    Colormap  xidcmap,
    Bool      contig,
    unsigned long*  plane_masks,
    unsigned int    planes,
    unsigned long*  pixels_return,
    unsigned int    ncolors)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC7(dpy, 24, xidcmap, contig, plane_masks,
         planes, pixels_return, ncolors);
   UnlockDisplay(dpy);
   return(code);
}
int XAllocColorPlanes(
   Display *dpy,
   Colormap xidcmap,
   Bool contig,
   unsigned long *pixels_return,
   int ncolors,
   int nreds, int ngreens, int nblues,
   unsigned long *rmask, unsigned long *gmask, unsigned long *bmask)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC11(dpy, 25,  xidcmap,contig, pixels_return, ncolors,
        nreds, ngreens, nblues, rmask, gmask, bmask);
   UnlockDisplay(dpy);
   return(code);
}
int XStoreColors(
   Display *dpy,
   Colormap xidcmap,
   XColor *def,
   int    ncolors)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC4(dpy, 26,  xidcmap, def,  ncolors);
   UnlockDisplay(dpy);
   return(code);
}

/* pixmap calls */
Pixmap XCreatePixmap(
 Display*	dpy,
 Drawable	d,
 unsigned int	width,
 unsigned int	height,
 unsigned int	depth)
{
   XID code=0;
#ifndef XLIBRENDER
   if(depth !=1 && depth !=16)
     return((Pixmap)0);
#endif
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   if(depth ==1)  code |= 0x80000000;
   XCMD6(dpy, 27, d, width, height, depth, code);
   UnlockDisplay(dpy);
   return((Pixmap)code);
}
Pixmap XCreateBitmapFromData(
    Display*		dpy,
    Drawable		d,
    _Xconst char*	data,
    unsigned int	width,
    unsigned int	height
)
{
   int size, isize;
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy) | 0x80000000;
   size=((width +7 ) >> 3)*height;
   if(size > TRAPTHRESH)
      {
      XCMDSYNC7(dpy, 28, d, width, height, code,  0, data);
      }
   else
      { /* pipe the data into the channel */
      isize=(size + 3)/4;
      CHKSPACE(dpy, 6 + isize);      
      XCMDRAW6(dpy, 28 | ((6 + isize) << 16), d, width, height, code,  1);
      copydpy(dpy, data, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return((Pixmap)code);
}
Pixmap XCreatePixmapFromBitmapData(
    Display*		dpy,
    Drawable		d,
    char*		data,
    unsigned int	width,
    unsigned int	height,
    unsigned long	fg,
    unsigned long	bg,
    unsigned int	depth)
{
   XID code=0;
#ifndef XLIBRENDER
   if(depth !=1 && depth !=16)
     return((Pixmap)0);
#endif
   LockDisplay(dpy);
   code = XAllocID(dpy);
   if(depth ==1)  code |= 0x80000000;
   XCMDSYNC9(dpy, 29, d, data, width, height,
        fg, bg,	depth, code);
   UnlockDisplay(dpy);
   return((Pixmap)code);
}
int XFreePixmap(
    Display*		dpy,
    Pixmap		pixmap)
{
   int code=1;
   if(pixmap == 0)
      return(code);	/* 0 is not valid, probably failed creat pixmap */
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 30, pixmap);
   UnlockDisplay(dpy);
   return(code);
}
Pixmap __XShmCreatePixmap(
 Display	*dpy,
 Drawable	d,
 unsigned int	w,
 unsigned int	h,
 unsigned int	depth,
 char *		data)
{
   XID code=0;
   dpy->request++;
#ifndef XLIBRENDER
   if(depth !=1 && depth !=16)
     return((Pixmap)0);
#endif
   code = XAllocID(dpy);
   if(depth ==1)  code |= 0x80000000;
   XCMD7(dpy, 31, d, w, h, depth, data, code);
   return((Pixmap)code);
}

/* Cursor calls */
int
XFreeCursor(Display*	dpy, register Cursor   cursor)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 32, cursor);
   UnlockDisplay(dpy);
   return(code);
}
int
XRecolorCursor (Display*	dpy,
 Cursor		cursor,
 XColor*	foreground_color,
 XColor*	background_color)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC4(dpy, 33, cursor, foreground_color,background_color);
   UnlockDisplay(dpy);
   return(code);
}
Cursor
XCreateFontCursor(Display*  dpy, unsigned int	shape)
{
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   XCMD3(dpy, 34, shape, code);
   UnlockDisplay(dpy);
   return((Cursor)code);
}
Cursor XCreatePixmapCursor(
    Display*		dpy,
    Pixmap		source,
    Pixmap		umask,
    XColor*		foreground_color,
    XColor*		background_color,
    unsigned int	x,
    unsigned int	y)
{
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   XCMDSYNC8(dpy, 35, source, umask, foreground_color, background_color,
   	x, y, code);
   UnlockDisplay(dpy);
   return((Cursor)code);
}
Cursor XCreateGlyphCursor(Display*	dpy,
 Font		source_font,
 Font		mask_font,
 unsigned int	source_char,
 unsigned int	mask_char,
 XColor _Xconst *	foreground_color,
 XColor _Xconst *	background_color)
{
   XID code=0;
   LockDisplay(dpy);
   dpy->request++;
   code = XAllocID(dpy);
   XCMDSYNC8(dpy, 36, source_font, mask_font,
            source_char, mask_char, foreground_color, background_color, code);
   UnlockDisplay(dpy);
   return((Cursor)code);
}

/* Keyboard calls */
int
XGetKeyboardControl (register Display *dpy,
		     register XKeyboardState *state)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC2(dpy, 37, state);
   UnlockDisplay(dpy);
   return(code);
}
int
XChangeKeyboardControl(dpy, mask, value_list)
    register Display *dpy;
    unsigned long mask;
    XKeyboardControl *value_list;
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 38, mask, value_list);
   UnlockDisplay(dpy);
   return(code);
}

int __XReadModifierMapping(Display *dpy, char * map)
{
   dpy->request++;
   XCMDSYNC2(dpy, 39, map);
   return 1;
}

int __XReadKeyboardMapping(dpy, mapping, first_keycode, count)
    Display *dpy; 
    register char *mapping;
    unsigned int first_keycode;
    int count;
{
   dpy->request++;
   XCMDSYNC4(dpy, 40, mapping, first_keycode, count);
   return 1;
}
int __XWriteKeyboardMapping(dpy, mapping, first_keycode, count, keysyms_per_keycode)
    Display *dpy; 
    register char *mapping;
    unsigned int first_keycode;
    int count;
    int keysyms_per_keycode;
{
   dpy->request++;
   XCMDSYNC5(dpy, 41, mapping, first_keycode, count, keysyms_per_keycode);
   return 1;
}

int XGrabKeyboard(
    Display*		dpy,
    Window		grab_window,
    Bool		owner_events,
    int			pointer_mode,
    int			keyboard_mode,
    Time		time)
{
   int code=0;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC7(dpy, 42, grab_window, owner_events,
        pointer_mode, keyboard_mode, time, &code);
   UnlockDisplay(dpy);
   return(code);
}
int XUngrabKeyboard(
    Display*		dpy,
    Time		time)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 43, time);
   UnlockDisplay(dpy);
   return(code);
}
int
XSetInputFocus(dpy, focus, revert_to, time)
    register Display *dpy;
    Window focus;
    int revert_to;
    Time time;
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 44, focus, revert_to, time);
   UnlockDisplay(dpy);
   return(code);
}
XSetInputFocusSync(dpy, focus, revert_to, time)
    register Display *dpy;
    Window focus;
    int revert_to;
    Time time;
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC4(dpy, 44, focus, revert_to, time);
   UnlockDisplay(dpy);
   return(code);
}
int
XGetInputFocus(dpy, focus_return, revert_to_return)
    register Display *dpy;
    Window *focus_return;
    int *revert_to_return;    
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 45, focus_return, revert_to_return);
   UnlockDisplay(dpy);
   return(code);
}
int XGrabKey(
    Display*		dpy,
    int			keycode,
    unsigned int	modifiers,
    Window		grab_window,
    Bool		owner_events,
    int			pointer_mode,
    int			keyboard_mode)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD7(dpy, 46,  keycode, modifiers, grab_window,
        owner_events, pointer_mode, keyboard_mode);
   UnlockDisplay(dpy);
   return(code);
}
int XUngrabKey(
    Display*		dpy,
    int			keycode,
    unsigned int	modifiers,
    Window		grab_window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 47, keycode, modifiers, grab_window);
   UnlockDisplay(dpy);
   return(code);
}

/* pointer calls */
int XGrabPointer
(	Display *	dpy,
	Window		grab_window,
	Bool		owner_events,
	unsigned int	event_mask,
	int		pointer_mode,
	int		keyboard_mode,
	Window		confine_to,
	Cursor		cursor,
	Time		time)
{
   int code=0;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC10(dpy, 48, grab_window, owner_events, event_mask,
	pointer_mode, keyboard_mode, confine_to, cursor, time, &code);
   UnlockDisplay(dpy);
   return(code);
}
int XUngrabPointer (Display *dpy, Time time)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 49, time);
   UnlockDisplay(dpy);
   return(code);
}
int XChangeActivePointerGrab
(	Display *	dpy,
	unsigned int	event_mask,
	Cursor		cursor,
	Time		time)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 50, event_mask, cursor,time);
   UnlockDisplay(dpy);
   return(code);
}
int XWarpPointer
(	Display *	dpy,
	Window		src_w,
	Window		dest_w,
	int		src_x,
	int		src_y,
	unsigned int	src_width,
	unsigned int	src_height,
	int		dest_x,
	int		dest_y)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD9(dpy, 51, src_w, dest_w, src_x, src_y,
	src_width, src_height, dest_x, dest_y);
   UnlockDisplay(dpy);
   return(code);
}
Bool XQueryPointer
(	Display*	dpy,
	Window		w,
	Window*		root_return,
	Window*		child_return,
	int*		root_x_return,
	int*		root_y_return,
	int*		win_x_return,
	int*		win_y_return,
	unsigned int*	mask_return)
{
   Bool code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC9(dpy, 52,  w, root_return, child_return, root_x_return,
	root_y_return, win_x_return, win_y_return, mask_return);
   UnlockDisplay(dpy);
   return(code);
}
int XGrabButton(
    Display*		dpy,
    unsigned int	button,
    unsigned int	modifiers,
    Window		grab_window,
    Bool		owner_events,
    unsigned int	event_mask,
    int			pointer_mode,
    int			keyboard_mode,
    Window		confine_to,
    Cursor		cursor)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD10(dpy, 53,  button, modifiers, grab_window, owner_events,
        event_mask, pointer_mode, keyboard_mode, confine_to, cursor);
   UnlockDisplay(dpy);
   return(code);
}
int XUngrabButton(
    Display*		dpy,
    unsigned int	button,
    unsigned int	modifiers,
    Window		grab_window)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 54,  button, modifiers,grab_window);
   UnlockDisplay(dpy);
   return(code);
}

/* GC Calls */
int __XcreateGCid(Display *dpy, Drawable d, GC gc)
{
   dpy->request++;
   gc->gid = XAllocID(dpy);
   XCMD3(dpy, 55, d, gc->gid);
   return(1);
}
int __XfreeGCid(Display *dpy, GC gc)
{
   dpy->request++;
   XCMD2(dpy, 56, gc->gid);
   return 1;
}
/*
 * GenerateGCList looks at the GC dirty bits, and appends all the required
 * long words to the request being generated.
 */

static int
_XGenerateGCList (gc, values)
    GC gc;
    unsigned long *values;
    {
    register unsigned long *value = values;
    long nvalues;
    register XGCValues *gv = &gc->values;
    register unsigned long dirty = gc->dirty;

    /*
     * Note: The order of these tests are critical; the order must be the
     * same as the GC mask bits in the word.
     */
    if (dirty & GCFunction)          *value++ = gv->function;
    if (dirty & GCPlaneMask)         *value++ = gv->plane_mask;
    if (dirty & GCForeground)        *value++ = gv->foreground;
    if (dirty & GCBackground)        *value++ = gv->background;
    if (dirty & GCLineWidth)         *value++ = gv->line_width;
    if (dirty & GCLineStyle)         *value++ = gv->line_style;
    if (dirty & GCCapStyle)          *value++ = gv->cap_style;
    if (dirty & GCJoinStyle)         *value++ = gv->join_style;
    if (dirty & GCFillStyle)         *value++ = gv->fill_style;
    if (dirty & GCFillRule)          *value++ = gv->fill_rule;
    if (dirty & GCTile)              *value++ = gv->tile;
    if (dirty & GCStipple)           *value++ = gv->stipple;
    if (dirty & GCTileStipXOrigin)   *value++ = gv->ts_x_origin;
    if (dirty & GCTileStipYOrigin)   *value++ = gv->ts_y_origin;
    if (dirty & GCFont)              *value++ = gv->font;
    if (dirty & GCSubwindowMode)     *value++ = gv->subwindow_mode;
    if (dirty & GCGraphicsExposures) *value++ = gv->graphics_exposures;
    if (dirty & GCClipXOrigin)       *value++ = gv->clip_x_origin;
    if (dirty & GCClipYOrigin)       *value++ = gv->clip_y_origin;
    if (dirty & GCClipMask)          *value++ = gv->clip_mask;
    if (dirty & GCDashOffset)        *value++ = gv->dash_offset;
    if (dirty & GCDashList)          *value++ = gv->dashes;
    if (dirty & GCArcMode)           *value++ = gv->arc_mode;

    nvalues = (value - values);

    return (nvalues);
    }
void _XFlushGCCache(Display *dpy, GC gc)
{
   int size;
   unsigned long values[32];
   if(!gc->dirty)
      return;	/* nothing to do */
   dpy->request++;
   size=_XGenerateGCList (gc, values);
   CHKSPACE(dpy, 3 + size);
   XCMDRAW3(dpy, 57 | ((size + 3) << 16), gc->gid, gc->dirty);
   copydpy(dpy, values, size*sizeof(unsigned long));
   RAWFLUSH(dpy);
   gc->dirty=0;
   return;
}

int __XSetClipRectangles (dpy, gc, clip_x_origin, clip_y_origin, rectangles, n,
                    ordering)
    register Display *dpy;
    GC gc;
    int clip_x_origin, clip_y_origin;
    XRectangle *rectangles;
    int n;
    int ordering;
{
   int size;
   dpy->request++;
   if(n ==0)
      {
      XCMD5(dpy, 58 | ( 5 << 16), gc->gid, clip_x_origin, clip_y_origin,  n);
      return 1;
      }
   size=(n*sizeof(XRectangle) + 3)/4;
   CHKSPACE(dpy, 5 + size);
   XCMDRAW5(dpy, 58 | ((size + 5) << 16), gc->gid, clip_x_origin, clip_y_origin,  n);
   copydpy(dpy, rectangles, n*(sizeof(XRectangle)));
   RAWFLUSH(dpy);
   return 1;
}
int XSetDashes(dpy, gc, dash_offset, dash_list, n)
    register Display *dpy;
    GC gc;
    int dash_offset;
    _Xconst char  *dash_list;
    int n;
{
   int size;
   int code=1;
   LockDisplay(dpy);
   gc->dirty &= ~(GCDashList | GCDashOffset); /* clear these bits */
   dpy->request++;
   size=(n + 3)/4;
   CHKSPACE(dpy, 4 + size);
   XCMDRAW4(dpy, 59 | ((size + 4) << 16),  gc->gid, dash_offset, n);
   copydpy(dpy, dash_list, n);
   RAWFLUSH(dpy);
   UnlockDisplay(dpy);
   return(code);
}
int __XCopyGC(Display *dpy, GC sgc, GC dgc)
{
   unsigned long values[32];
   int size;
   dpy->request++;
   size=_XGenerateGCList (dgc, values);
   CHKSPACE(dpy, 4 + size);
   XCMDRAW4(dpy, 60 | ((size + 4) << 16), sgc->gid, dgc->gid, dgc->dirty);
   copydpy(dpy, values, size*sizeof(unsigned long));
   RAWFLUSH(dpy);
   return 1;
}

/* event calls */
int
XSendEvent
(	Display*	dpy,
	Window		winid,
	int		propagate,
	long		event_mask,
	XEvent*		event)
{
   int size;
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   size=(sizeof(XEvent) + 3)/4;
   CHKSPACE(dpy, 4 + size);
   XCMDRAW4(dpy, 61 | ((size + 4) << 16), winid, propagate, event_mask);
   copydpy(dpy, event, sizeof(XEvent));
   RAWFLUSH(dpy);
   UnlockDisplay(dpy);
   return(code);
}


int
XAllowEvents
(	Display*	dpy,
	int		event_mode,
	Time		time)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD3(dpy, 62, event_mode,time);
   UnlockDisplay(dpy);
   return(code);
}

int __XGetMotionEvents(
	Display		*dpy,
	Window		window,
	Time	start,
	Time	stop,
	int	*unevents,
	XTimeCoord *tc)
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC7(dpy, 63, window, start, stop, unevents, tc, &ret);
   return ret;
}

/* atom and properties */
Atom
__XInternAtom(
    Display*		dpy,
    _Xconst char*	atom_name,
    Bool		only_if_exists)
{
   Atom code=0;
   dpy->request++;
   XCMDSYNC4(dpy, 64, atom_name, only_if_exists, &code);
   return(code);
}

int __XGetAtomName(Display *dpy,
 Atom		atom,
 int 		*length,
 char 		*data)
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC5(dpy, 65,  atom, length, data, &ret);
   return ret;
}
int __XGetWindowProperty(
    Display		*dpy,
    Window		w,
    Atom		property,
    long		long_offset,
    long		long_length,
    Bool		delete,
    Atom		req_type,
    Atom*		actual_type_return,
    int*		actual_format_return,
    unsigned long*	nitems_return,
    unsigned long*	bytes_after_return,
    unsigned char*	data)
{
   int ret=0;

   dpy->request++;
   XCMDSYNC13(dpy, 66, w, property, long_offset,
      long_length, delete, req_type, actual_type_return, actual_format_return,
      nitems_return, bytes_after_return, data, &ret);
   return ret;
}
int __XListProperties(
    Display		*dpy,
    Window		w,
    int*		num_prop_return,
    Atom 		*ap)
{
   int ret=0;

   dpy->request++;
   XCMDSYNC5(dpy, 67,  w, num_prop_return, ap, &ret);
   return ret;
}


int XChangeProperty(
    Display*		dpy,
    Window		w,
    Atom		property,
    Atom		type,
    int			format,
    int			mode,
    _Xconst unsigned char*	data,
    int			nelements)
{
   int size, isize;
   int code=1;

   size=nelements * (format/8);
   LockDisplay(dpy);
   dpy->request++;
   if(size > TRAPTHRESH)
      {
      XCMDSYNC9(dpy, 68, w, property, type, format,
           mode, nelements, 0, data);
      }
   else
      { /* pipe the data into the channel */
      isize=(size + 3)/4;
      CHKSPACE(dpy, 8 + isize);      
      XCMDRAW8(dpy, 68 | ((8+isize) << 16), w, property, type, format, mode, nelements, 1);
      copydpy(dpy, data, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}
int XDeleteProperty(
    Display*		dpy,
    Window		w,
    Atom		property)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD3(dpy, 69,w,property);
   UnlockDisplay(dpy);
   return(code);
}
int XRotateWindowProperties(
    Display*		dpy,
    Window		w,
    Atom*		properties,
    int			num_prop,
    int			npositions)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC5(dpy, 70,  w, properties, num_prop, npositions);
   UnlockDisplay(dpy);
   return(code);
}

/* selection */
int XSetSelectionOwner(dpy, selection, owner, time)
     Display *dpy;
     Atom selection;
     Window owner;
     Time time;
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 71, selection, owner, time);
   UnlockDisplay(dpy);
   return(code);
}
Window XGetSelectionOwner(dpy, selection)
   Display *dpy;
   Atom selection;
{
   Window code=0;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 72, selection, &code);
   UnlockDisplay(dpy);
   return(code);
}
int XConvertSelection(dpy, selection, target, property, requestor, time)
   Display *dpy;
   Atom selection, target;
   Atom property;
   Window requestor;
   Time time;
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD6(dpy, 73, selection, target, property, requestor, time);
   UnlockDisplay(dpy);
   return(code);
}

/* Font calls */
XID __XLoadFont(Display *dpy, _Xconst char *name, int index)
{
   XID fid=0;
   
   dpy->request++;
   fid = XAllocID(dpy);
   XCMDSYNC4(dpy, 74, name, index, fid);
   return fid;
}
int XUnloadFont(Display *dpy, XID fontid)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 75, fontid);
   UnlockDisplay(dpy);
   return(code);
}
int __XDrawString(
    Display		*dpy,
    Drawable		d,
    GC			gc,
    int			x1,
    int			y1,
    _Xconst char*	string,
    int			count,
    int			reqtype)
{
   dpy->request++;
   FlushGC(dpy, gc);
   XCMDSYNC8(dpy, 76, d, gc->gid, x1, y1, string, count, reqtype);
   return 1;
}
int __XQueryFont(
    Display 	*dpy,
    XID		fontid,
    XFontStruct *fs, 
    int 	*index,
    int		*ncharinfo)
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC6(dpy, 77,  fontid, fs, index, ncharinfo, &ret);
   return ret;
}

int XQueryTextExtents (
    Display *dpy,
    XID fid,
    _Xconst char *string,
    int count,
    int *dir,
    int *font_ascent,
    int *font_descent,
    XCharStruct *overall)
{
   register int i;
   register char *ptr;
   char *buf;
   int code=1;

   LockDisplay(dpy);
   buf=(char *)Xmalloc(count*2);
   for (ptr = buf, i = count; --i >= 0;) {
       *ptr++ = 0;
       *ptr++ = *string++;
   }
   dpy->request++;
   XCMDSYNC8(dpy, 78,  fid, buf, count,
         dir, font_ascent, font_descent, overall);
   Xfree(buf);
   UnlockDisplay(dpy);
   return(code);
}
XQueryTextExtents16 (
    register Display *dpy,
    Font fid,
    _Xconst XChar2b *string,
    int count,
    int *dir,
    int *font_ascent,
    int *font_descent,
    register XCharStruct *overall)
{
   register int i;
   register unsigned char *ptr;
   char *buf;
   int code=1;

   LockDisplay(dpy);
   buf=(char *)Xmalloc(count*2);
   for (ptr = (unsigned char *)buf, i = count; --i >= 0; string++) {
       *ptr++ = string->byte1;
       *ptr++ = string->byte2;
   }
   dpy->request++;
   XCMDSYNC8(dpy, 78,  fid, buf, count,
         dir, font_ascent, font_descent, overall);
   Xfree(buf);
   UnlockDisplay(dpy);
   return(code);
}
/* draw calls */
int
XCopyArea(
    Display*		dpy,
    Drawable		src,
    Drawable		dest,
    GC			gc,
    int			src_x,
    int			src_y,
    unsigned int	width,
    unsigned int	height,
    int			dest_x,
    int			dest_y)
{
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   XCMD10(dpy, 79, src, dest, gc->gid, src_x, src_y, width,
        height, dest_x, dest_y);
   UnlockDisplay(dpy);
   return(code);
}
int
XCopyPlane(
    Display*		dpy,
    Drawable		src,
    Drawable		dest,
    GC			gc,
    int			src_x,
    int			src_y,
    unsigned int	width,
    unsigned int	height,
    int			dest_x,
    int			dest_y,
    unsigned long	plane)
{
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   XCMD11(dpy, 80, src, dest, gc->gid, src_x, src_y, width,
         height, dest_x, dest_y, plane);
   UnlockDisplay(dpy);
   return(code);
}
int
XFillRectangles(
    Display*		dpy,
    Drawable		d,
    GC			gc,
    XRectangle*		rectangles,
    int			nrectangles)
{
   int size, isize;
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   size=nrectangles*sizeof(XRectangle);
   dpy->request++;
   if(size > TRAPTHRESH)
      {
      XCMDSYNC6(dpy, 81, d, gc->gid, nrectangles, 0, rectangles);
      }
   else
      {
      isize=(size + 3)/4;
      CHKSPACE(dpy, 5 + isize);
      XCMDRAW5(dpy, 81 | ((5+isize) << 16), d, gc->gid, nrectangles, 1);
      copydpy(dpy, rectangles, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}

int
XDrawPoints(
    Display*		dpy,
    Drawable		d,
    GC			gc,
    XPoint*		points,
    int			npoints,
    int			mode)
{
   int code=1;
   int size, isize;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   size=npoints*sizeof(XPoint);
   if(size > TRAPTHRESH)
      {
      XCMDSYNC7(dpy, 82, d, gc->gid, npoints, mode, 0, points);
     }
   else
     {
      isize=(size + 3)/4;
      CHKSPACE(dpy, 6 + isize);
      XCMDRAW6(dpy, 82 | ((6+isize) << 16), d, gc->gid, npoints, mode, 1);
      copydpy(dpy, points, size);
      RAWFLUSH(dpy);
     }
   UnlockDisplay(dpy);
   return(code);
}

int
XDrawLines
(Display*	dpy,
 Drawable	d,
 GC		gc,
 XPoint*	points,
 int		npoints,
 int		mode)
{
   int code=1;
   int size, isize;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   size=npoints*sizeof(XPoint);
   if(size > TRAPTHRESH)
      {
      XCMDSYNC7(dpy, 83, d, gc->gid, npoints, mode, 0, points);
      }
   else
      { /* pipe data to the kernel */
      isize=(size + 3)/4;
      CHKSPACE(dpy, 6 + isize);
      XCMDRAW6(dpy, 83 | ((6+isize) << 16), d, gc->gid, npoints, mode, 1);
      copydpy(dpy, points, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}
int __XPutRectangle(
    Display		*dpy,
    Drawable		d,
    GC			gc,
    XRectangle*		rectangle,
    int			awidth,
    register pixel_t	*pptr)
{
   dpy->request++;
   FlushGC(dpy, gc);
   XCMDSYNC6(dpy, 84,  d, gc->gid, rectangle, awidth, pptr);
   return 1;
}

int
XFillPolygon(
    Display*		dpy,
    Drawable		d,
    GC			gc,
    XPoint		*points,
    int			count,              /* number of points        */
    int			shape,
    int			mode)
{
   int size, isize;
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   size=count*sizeof(XPoint);
   if(size > TRAPTHRESH)
      {
      XCMDSYNC8(dpy, 85, d, gc->gid, count, shape, mode, 0, points);
      }
   else
      {
      isize=(size + 3)/4;
      CHKSPACE(dpy, 7 + isize);
      XCMDRAW7(dpy, 85 | ((7+isize) << 16), d, gc->gid, count, shape, mode,  1);
      copydpy(dpy, points, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}

int
XDrawSegments
(Display*	dpy,
 Drawable	d,
 GC		gc,
 XSegment*	segment,
 int		nsegments)
{
   int size, isize;
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   dpy->request++;
   size=nsegments*sizeof(XSegment);
   if(size > TRAPTHRESH)
      {
      XCMDSYNC6(dpy, 86,  d, gc->gid, nsegments, 0, segment);
      }
   else
      {
      isize=(size + 3)/4;
      CHKSPACE(dpy, 5 + isize);
      XCMDRAW5(dpy, 86 | ((5+isize) << 16),  d, gc->gid, nsegments, 1);
      copydpy(dpy, segment, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}

int __XGetRectangle(
    Display		*dpy,
    Drawable		d,
    unsigned long	plane_mask,
    XRectangle*		urect,
    pixel_t		*pptr)
{
   int depth= -1;
   dpy->request++;
   XCMDSYNC6(dpy, 87, d, plane_mask, urect, pptr, &depth);
   return depth;
}

int
XDrawRectangles(Display*	dpy,
 Drawable	d,
 GC		gc,
 XRectangle*	rect,
 int		nrectangles)
{
   int size, isize;
   int code=1;
   LockDisplay(dpy);
   FlushGC(dpy, gc);
   size=nrectangles*sizeof(XRectangle);
   dpy->request++;
   if(size > TRAPTHRESH)
      {
      XCMDSYNC6(dpy, 88, d, gc->gid, nrectangles, 0, rect);
      }
   else
      {
      isize=(size + 3)/4;
      CHKSPACE(dpy, 5 + isize);
      XCMDRAW5(dpy, 88 | ((5+isize) << 16),  d, gc->gid, nrectangles, 1);
      copydpy(dpy, rect, size);
      RAWFLUSH(dpy);
      }
   UnlockDisplay(dpy);
   return(code);
}

/* misc calls */
int XSetCloseDownMode(dpy, mode)
    register Display *dpy; 
    int mode;

{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 89, mode);
   UnlockDisplay(dpy);
   return(code);
}

int XChangeSaveSet(Display *dpy, Window window, int change_mode)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD3(dpy, 90,  window, change_mode);
   UnlockDisplay(dpy);
   return(code);
}

int XGrabServer(Display *dpy)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD1(dpy, 91);
   UnlockDisplay(dpy);
   return(code);
}
int XUngrabServer(Display *dpy)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD1(dpy, 92);
   UnlockDisplay(dpy);
   return(code);
}
int XKillClient(Display *dpy, XID id)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 93, id);
   UnlockDisplay(dpy);
   return(code);
}
int
XBell (Display *dpy, int percent)
{
   int code=1;
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 94, percent);
   UnlockDisplay(dpy);
   return(code);
}

/* XShape calls */

void XShapeCombineRectangles (dpy, dest, destKind, xOff, yOff,
			      rects, n_rects, op, ordering)
register Display *dpy;
XID dest;
int destKind, op, xOff, yOff, ordering;
XRectangle  *rects;
int n_rects;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC9(dpy, 95, dest, destKind, xOff, yOff, rects, n_rects, op, ordering);
   UnlockDisplay(dpy);
   return;
}


void XShapeCombineMask (dpy, dest, destKind, xOff, yOff, src, op)
register Display *dpy;
int destKind;
XID dest;
Pixmap	src;
int op, xOff, yOff;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD7(dpy, 96, dest, destKind, xOff, yOff, src, op);
   UnlockDisplay(dpy);
   return;
}

void XShapeCombineShape (dpy, dest, destKind, xOff, yOff, src, srcKind, op)
register Display *dpy;
int destKind;
XID dest;
int srcKind;
XID src;
int op, xOff, yOff;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD8(dpy, 97, dest, destKind, xOff, yOff, src, srcKind, op);
   UnlockDisplay(dpy);
   return;
}

void XShapeOffsetShape (dpy, dest, destKind, xOff, yOff)
register Display *dpy;
int destKind;
XID dest;
int xOff, yOff;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD5(dpy, 98, dest, destKind, xOff, yOff);
   UnlockDisplay(dpy);
   return;
}

Status XShapeQueryExtents (dpy, window,
			   bShaped, xbs, ybs, wbs, hbs,
			   cShaped, xcs, ycs, wcs, hcs)    
    register Display    *dpy;
    Window		    window;
    int			    *bShaped, *cShaped;	    /* RETURN */
    int			    *xbs, *ybs, *xcs, *ycs; /* RETURN */
    unsigned int	    *wbs, *hbs, *wcs, *hcs; /* RETURN */
{
   int code=0;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC13(dpy, 99, window, bShaped, xbs, ybs, wbs, hbs,
			   cShaped, xcs, ycs, wcs, hcs, &code);    
   UnlockDisplay(dpy);
   return(code);
}


void XShapeSelectInput (dpy, window, mask)
    register Display	*dpy;
    Window		window;
    unsigned long	mask;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD3(dpy, 100, window, mask);
   UnlockDisplay(dpy);
   return;
}

unsigned long XShapeInputSelected (dpy, window)
    register Display	*dpy;
    Window		window;
{
   unsigned long code=0L;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC3(dpy, 101, window, &code);
   UnlockDisplay(dpy);
   return code;
}
int __XShapeGetRectangles (dpy, window, kind, xr, count)
    register Display	*dpy;
    Window		window;
    int			kind;
    XRectangle		*xr;
    int			*count;	/* RETURN */
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC6(dpy, 102,  window, kind, xr, count, &ret);
   return ret;
}

/* XTest calls */

int XTestGrabControl(dpy, impervious)
    Display *dpy;
    Bool impervious;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD2(dpy, 103, impervious);
   UnlockDisplay(dpy);
   return;
}
Bool XTestCompareCursorWithWindow(dpy, window, cursor)
    Display *dpy;
    Window window;
    Cursor cursor;
{
   unsigned int code=0;
   LockDisplay(dpy);
   dpy->request++;
   XCMDSYNC4(dpy, 104, window, cursor, &code);
   UnlockDisplay(dpy);
   return ((Bool)code);
}
int XTestFakeKeyEvent(dpy, keycode, is_press, delay)
    Display *dpy;
    unsigned int keycode;
    Bool is_press;
    unsigned long delay;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 105, keycode, is_press, delay);
   UnlockDisplay(dpy);
   return 1;
}
int XTestFakeButtonEvent(dpy, button, is_press, delay)
    Display *dpy;
    unsigned int button;
    Bool is_press;
    unsigned long delay;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 106, button, is_press, delay);
   UnlockDisplay(dpy);
   return 1;
}
int XTestFakeMotionEvent(dpy, screen, x, y, delay)
    Display *dpy;
    int screen;
    int x, y;
    unsigned long delay;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD5(dpy, 107, screen, x, y, delay);
   UnlockDisplay(dpy);
   return 1;
}

int XTestFakeRelativeMotionEvent(dpy, dx, dy, delay)
    Display *dpy;
    int dx, dy;
    unsigned long delay;
{
   LockDisplay(dpy);
   dpy->request++;
   XCMD4(dpy, 108, dx, dy, delay);
   UnlockDisplay(dpy);
   return 1;
}
/* Scan fill from user space */
int __XFillSpans(Display *dpy, Drawable d, GC gc, int n, XPoint *points, int *widths)
{
   dpy->request++;
   FlushGC(dpy, gc);
   XCMDSYNC6(dpy, 109,  d, gc->gid, n, points, widths);
   return 0;
}

/* some asynchronus drawings */
int
XDrawPoint(
    Display*		dpy,
    Drawable		d,
    GC			gc,
    int			x,
    int			y)
{
   LockDisplay(dpy);
   dpy->request++;
   FlushGC(dpy, gc);
   XCMD5(dpy, 110, d, gc->gid, x, y);
   UnlockDisplay(dpy);
   return 1;
}

XDrawLine (dpy, d, gc, x1, y1, x2, y2)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x1, y1, x2, y2;
{
   LockDisplay(dpy);
   dpy->request++;
   FlushGC(dpy, gc);
   XCMD7(dpy, 111, d, gc->gid, x1, y1, x2, y2);
   UnlockDisplay(dpy);
   return 1;
}

XFillRectangle(dpy, d, gc, x, y, width, height)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    unsigned int width, height;
{
   LockDisplay(dpy);
   dpy->request++;
   FlushGC(dpy, gc);
   XCMD7(dpy, 112, d, gc->gid, x, y, width, height);
   UnlockDisplay(dpy);
   return 1;
}
/*
 * Draw a single rectangle
 */
int
XDrawRectangle(Display*	dpy,
 Drawable	d,
 GC		gc,
 int		x,
 int            y,
 unsigned  int  width,
 unsigned  int  height)
{
   LockDisplay(dpy);
   dpy->request++;
   FlushGC(dpy, gc);
   XCMD7(dpy, 113, d, gc->gid, x, y, width, height);
   UnlockDisplay(dpy);
   return 1;
}
/* 114 is _XFillPolygon3 */

/* flushes the buffer and return number of events */
int __XSync(dpy)
    register Display	*dpy;
{
   int ret=0;
   
   dpy->request++;
   XCMDSYNC2(dpy, 115, &ret);
   return ret;
}
/*
 * _XFlush - Flush the X request buffer.  Might be useful for the kernel to know about.
 */
void _XFlush (dpy)
	register Display *dpy;
{
   if(_wrindex(dpy))
      {
      XCMDSYNC1(dpy, 116);
      }
    return;
}
