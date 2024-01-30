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
#include "Xlibint.h"
#include "XShm.h"
#include "shmstr.h"
extern int ShmReqCode;
extern int ShmEventBase;
Bool XShmQueryExtension (dpy /* event_basep, error_basep */)
    Display *dpy;
/*  int *event_basep, *error_basep; */
{
   int opcode, fevent, ferror;
   if(XQueryExtension(dpy, SHMNAME, &opcode, &fevent, &ferror))
      return True;
   return False;
}

int XShmGetEventBase(dpy)
    Display *dpy;
{
  return (ShmEventBase);
}


Bool XShmQueryVersion(dpy, majorVersion, minorVersion, sharedPixmaps)
    Display *dpy;
    int	    *majorVersion, *minorVersion;
    Bool    *sharedPixmaps;
{
   int opcode, fevent, ferror;
   if(XQueryExtension(dpy, SHMNAME, &opcode, &fevent, &ferror))
      {
      *majorVersion = 1;
      *minorVersion = 0;
      *sharedPixmaps = True;
      return True;
      }
   return False;
}


int XShmPixmapFormat(dpy)
    Display *dpy;
{
    return(ZPixmap);
}


Status XShmAttach(dpy, shminfo)
    Display *dpy;
    XShmSegmentInfo *shminfo;
{
    return 1;
}


Status XShmDetach(dpy, shminfo)
    Display *dpy;
    XShmSegmentInfo *shminfo;
{
    return 1;
}

static int _XShmDestroyImage (ximage)
    XImage *ximage;

{
	Xfree((char *)ximage);
	return 1;
}
XImage *XShmCreateImage (dpy, visual, depth, format, data, shminfo,
			 width, height)
    register Display *dpy;
    register Visual *visual;
    unsigned int depth;
    int format;
    char *data;
    XShmSegmentInfo *shminfo;
    unsigned int width;
    unsigned int height;
{
    register XImage *image;
    image=XCreateImage (dpy, visual, depth, format, 0 /* offset */, data, width, height,
    ((depth == 1) ? 8 : 32 ) /* pad */, 0 /* image_bytes_per_line */);
    image->f.destroy_image = _XShmDestroyImage;
    return image;
}

Status XShmPutImage (dpy, d, gc, image, src_x, src_y, dst_x, dst_y,
		     src_width, src_height, send_event)
    register Display *dpy;
    Drawable d;
    GC gc;
    register XImage *image;
    int src_x, src_y, dst_x, dst_y;
    unsigned int src_width, src_height;
    Bool send_event;
{
   XShmCompletionEvent event;
   
   XPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height);
   if(send_event)
      {
      LockDisplay(dpy);
      event.type=XShmGetEventBase(dpy) + ShmCompletion;
      event.serial=dpy->request;
      event.send_event=False;
      event.display=dpy;
      event.drawable=d;
      event.major_code=ShmReqCode;
      event.minor_code=X_ShmPutImage;
      event.shmseg=(ShmSeg)0;
      event.offset=0;
      _XEnq (dpy, (xEvent *)&event);
      UnlockDisplay(dpy);
      }
   return(1);
}


Status XShmGetImage(dpy, d, image, x, y, plane_mask)
    register Display *dpy;
    Drawable d;
    XImage *image;
    int x, y;
    unsigned long plane_mask;
{
   printf("XShmGetImage not supported\n");
/*   image=XGetImage(dpy, d, x, y, width, height, plane_mask, ZPixmap); */
   return(0);
}

Pixmap XShmCreatePixmap (dpy, d, data, shminfo, width, height, depth)
    register Display *dpy;
    Drawable d;
    char *data;
    XShmSegmentInfo *shminfo;
    unsigned int width, height, depth;
{
   int code;
   LockDisplay(dpy);
   code=__XShmCreatePixmap(dpy, d, width, height, depth, data);
   UnlockDisplay(dpy);
   return(code);
}
