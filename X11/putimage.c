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
#include "Xlibint.h"
/*#include <sys/types.h> */	/* pixel_t */
typedef	unsigned short	pixel_t; 
/*typedef	unsigned long	u_long; */
/* begin from xputimage.c */

#if __STDC__
#define Const const
#else
#define Const /**/
#endif
/* aligns to a 4 byte boundary, RISC's work better */
#define	GRALIGN(a)	( ((u_long)(a) + 3) & ~0x3) 
/* assumes pad is a power of 2 */
#define ROUNDUP(nbytes, pad) (((nbytes) + ((pad) - 1)) & ~(long)((pad) - 1))

static unsigned char Const _reverse_byte[0x100] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static unsigned char Const _reverse_nibs[0x100] = {
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
	0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
	0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71,
	0x81, 0x91, 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1,
	0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72,
	0x82, 0x92, 0xa2, 0xb2, 0xc2, 0xd2, 0xe2, 0xf2,
	0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73,
	0x83, 0x93, 0xa3, 0xb3, 0xc3, 0xd3, 0xe3, 0xf3,
	0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74,
	0x84, 0x94, 0xa4, 0xb4, 0xc4, 0xd4, 0xe4, 0xf4,
	0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75,
	0x85, 0x95, 0xa5, 0xb5, 0xc5, 0xd5, 0xe5, 0xf5,
	0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76,
	0x86, 0x96, 0xa6, 0xb6, 0xc6, 0xd6, 0xe6, 0xf6,
	0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77,
	0x87, 0x97, 0xa7, 0xb7, 0xc7, 0xd7, 0xe7, 0xf7,
	0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78,
	0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0xe8, 0xf8,
	0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79,
	0x89, 0x99, 0xa9, 0xb9, 0xc9, 0xd9, 0xe9, 0xf9,
	0x0a, 0x1a, 0x2a, 0x3a, 0x4a, 0x5a, 0x6a, 0x7a,
	0x8a, 0x9a, 0xaa, 0xba, 0xca, 0xda, 0xea, 0xfa,
	0x0b, 0x1b, 0x2b, 0x3b, 0x4b, 0x5b, 0x6b, 0x7b,
	0x8b, 0x9b, 0xab, 0xbb, 0xcb, 0xdb, 0xeb, 0xfb,
	0x0c, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c,
	0x8c, 0x9c, 0xac, 0xbc, 0xcc, 0xdc, 0xec, 0xfc,
	0x0d, 0x1d, 0x2d, 0x3d, 0x4d, 0x5d, 0x6d, 0x7d,
	0x8d, 0x9d, 0xad, 0xbd, 0xcd, 0xdd, 0xed, 0xfd,
	0x0e, 0x1e, 0x2e, 0x3e, 0x4e, 0x5e, 0x6e, 0x7e,
	0x8e, 0x9e, 0xae, 0xbe, 0xce, 0xde, 0xee, 0xfe,
	0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f,
	0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff
};


_XReverse_Bytes (bpt, nb)
    register unsigned char *bpt;
    register int nb;
{
    do {
	*bpt = _reverse_byte[*bpt];
	bpt++;
    } while (--nb > 0);
    return;
}

/* end from xputimage.c */
XPutImage (
    Display*		display,
    Drawable		d,
    GC			gc,
    XImage*		image,
    int			src_x,
    int			src_y,
    int			dest_x,
    int			dest_y,
    unsigned int	width,
    unsigned int	height)	  
{
   int x, y, depth, awidth;
   register pixel_t *buf;
   XRectangle rect;
     
   /* this logic is from X11 putimage */
    if (src_x < 0) {
	width += src_x;
	src_x = 0;
    }
    if (src_y < 0) {
	height += src_y;
	src_y = 0;
    }
    if ((src_x + width) > image->width)
	width = image->width - src_x;
    if ((src_y + height) > image->height)
	height = image->height - src_y;
    if ((width <= 0) || (height <= 0))
	return 0;

   if(image->format == XYBitmap  && image->depth != 1)
      {
      printf("XPutImage BadMatch\n");
      return;
      }
   LockDisplay(display);
#if 1
   if(image->format == ZPixmap && image->bitmap_pad == 32   
     && image->xoffset == 0 && image->bits_per_pixel == 16 &&
          (image->depth == DefaultDepth(display, DefaultScreen(display))) )
      {
      /* we should be able to copy directly */
      rect.x=dest_x;
      rect.y=dest_y;
      rect.width=width;
      rect.height=height;  
      __XPutRectangle(display, d, gc, &rect, image->bytes_per_line, 
         (image->data + src_y*image->bytes_per_line + src_x*sizeof(pixel_t)));
      UnlockDisplay(display);
      return;      
      }
#endif
#ifdef XLIBRENDER
   if (image->depth == 8 || image->depth == 32)
      {
      /* QT/X11 request, this has to be a pixmap, since we support this depth only on pixmaps */
      if(image->format == ZPixmap && image->xoffset == 0 )
         {
         rect.x=dest_x;
         rect.y=dest_y;
         rect.width=width;
         rect.height=height;  
         __XPutPixmapRectangle(display, d, gc, &rect, image->bytes_per_line, 
            (image->data + src_y*image->bytes_per_line + (src_x*image->depth/8)) );
         }
      else
         printf("Xputimage depth %d format %d bitmap_pad %d xoffset %d\n", image->depth, image->format, image->bitmap_pad, image->xoffset);
      UnlockDisplay(display);
      return;      
      }
#endif
   /* make sure that XGetPixel(), function pointer is valid */
   if(!image->f.get_pixel)
       _XInitImageFuncPtrs(image);
   /* width of the buffer after allignment */
   awidth=GRALIGN(width*sizeof(pixel_t))/sizeof(pixel_t);
   buf=(pixel_t *)Xmalloc(awidth*height*sizeof(pixel_t));
   rect.x=dest_x;
   rect.y=dest_y;
   rect.width=width;
   rect.height=height;  
   if (image->depth == 1)
      {
      pixel_t fore, back;

      fore = (pixel_t)gc->values.foreground; 
      back = (pixel_t)gc->values.background; 
      if(image->format == XYBitmap)
         {
         for (y = 0; y < height; y++)
            {
            for (x = 0; x < width; x++)
               {
	       *(buf + y*awidth + x)=
                  (XGetPixel(image, src_x + x, src_y + y) ? fore : back);
               }
            }
	 }
      else
         {
         for (y = 0; y < height; y++)
            {
            for (x = 0; x < width; x++)
               {
	       *(buf + y*awidth + x)=
                  (XGetPixel(image, src_x + x, src_y + y) ? 1 : 0);
               }
            }
	 }
      }
   else if (image->depth == DefaultDepth(display, DefaultScreen(display)))
      {
      for (y = 0; y < height; y++)
         {
         for (x = 0; x < width; x++)
            {
	    *(buf + y*awidth + x)=
               XGetPixel(image, src_x + x, src_y + y);
            }
	 }
      }
   __XPutRectangle(display, d, gc, &rect, (awidth*sizeof(pixel_t)), buf);
   Xfree(buf);
   UnlockDisplay(display);
   return;
}

XImage *XGetImage (dpy, d, src_x, src_y, width, height, plane_mask, format)
     register Display *dpy;
     Drawable d;
     int src_x, src_y;
     unsigned int width, height;
     unsigned long plane_mask;
     int format;	/* either XYPixmap or ZPixmap */
{
   register XImage *image;
   int x, y, depth, awidth;
   register pixel_t *buf;
   char *data;  
   XRectangle rect;
     
   LockDisplay(dpy);
   /* width of the buffer after allignment */
   awidth=GRALIGN(width*sizeof(pixel_t))/sizeof(pixel_t);
   buf=(pixel_t *)Xmalloc(awidth*height*sizeof(pixel_t));
   rect.x=src_x;
   rect.y=src_y;
   rect.width=width;	/* kernel always expects aligned */
   rect.height=height;  
   /* get the actual depth of drawable */
   if( (depth=__XGetRectangle(dpy, d, plane_mask, &rect,  buf)) < 0)
      {
      Xfree(buf);
      UnlockDisplay(dpy);
      return(NULL);
      }
   /* ZPixmap at the right depth doesn't need any processing */
   if(depth == DefaultDepth(dpy, DefaultScreen(dpy)) &&
      format == ZPixmap)
      {
      if ((image = (XImage *) Xcalloc(1, (unsigned) sizeof(XImage))) == NULL)
	    return (XImage *) NULL;
      image->width = width;
      image->height = height;
      image->depth = depth;
      image->format = ZPixmap;
      image->byte_order = dpy->byte_order;
      image->bitmap_unit = dpy->bitmap_unit;
      image->bitmap_bit_order = dpy->bitmap_bit_order;
      image->xoffset = 0;
      image->bitmap_pad = 32;
      image->data = (char *)buf;
      image->bytes_per_line = awidth*sizeof(pixel_t);
      image->bits_per_pixel = 16;
      image->obdata = NULL;
      _XInitImageFuncPtrs (image);
#if 0
      image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
              depth, format, 0, (char *)buf, width, height, 
              32, awidth*sizeof(pixel_t));
#endif
      UnlockDisplay(dpy);
      return(image);
      }
   if(depth == 1)
      data=Xmalloc(((width + 7) >> 3)*height);
   else
      data=Xmalloc(awidth*height*sizeof(pixel_t));
   image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
              depth, format, 0, data, width, height, 
              (depth == 1) ? 8 : 32, /* padding different for bit map */
              /* so too, bytes per line */
              (depth == 1) ? ((width + 7) >> 3) : (awidth*sizeof(pixel_t)) );
   for (y = 0; y < height; y++)
      {
      for (x = 0; x < width; x++)
          XPutPixel(image, x, y, (unsigned long)(*(buf + y*awidth + x)) );
      }
   Xfree(buf);
   UnlockDisplay(dpy);
   return(image);
}

XImage *XGetSubImage(dpy, d, x, y, width, height, plane_mask, format,
		     dest_image, dest_x, dest_y)
     register Display *dpy;
     Drawable d;
     int x, y;
     unsigned int width, height;
     unsigned long plane_mask;
     int format;	/* either XYFormat or ZFormat */
     XImage *dest_image;
     int dest_x, dest_y;
{
	XImage *temp_image;
	temp_image = XGetImage(dpy, d, x, y, width, height, 
				plane_mask, format);
	if (!temp_image)
	    return (XImage *)NULL;
	_XSetImage(temp_image, dest_image, dest_x, dest_y);
	XDestroyImage(temp_image);
	return (dest_image);
}
