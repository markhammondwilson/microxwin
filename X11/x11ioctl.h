/*								 
 * Module Name - x11ioctl.h
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

#ifndef _SYS_X11IOCTL_H_
#define	_SYS_X11IOCTL_H_

#ifndef __KERNEL__
#include <sys/cdefs.h>
#endif /* !__KERNEL__ */

#ifdef XLIB
#include <sys/types.h>
#endif

typedef struct _X11InfoRec {
   int width;		/* width in pixels */
   int height;		/* height in pixels */
   int bitsperpixel;	/* bits per pixel (8, 15, 16 or 24) */
   int class;		/* true color or pseudo color */
   int	min_keycode, max_keycode;
   unsigned int red_mask; 
   unsigned int green_mask; 
   unsigned int blue_mask;	/* bit masks for RGB */
   int bits_per_rgb;
   int map_entries;
   unsigned long cmapid;	/* XID of colormap */
   unsigned long rootid;	/* id of root window */
   unsigned long white;		/* white and black pixel */
   unsigned long black;
   long		root_mask;	/* events on root window */
   int		motionbufsize;	/* size of motion buffer */
   int		res_base;	/* the LSB's have connection info */
   int		res_shift;	
   } X11Info;
/*
 * Touch screen event format written by xevent.
 */
struct tsevent
   {
   int	x;
   int	y;
   unsigned int	pressure;
   };

/*
 * Format of mouse and kbd data written to the kernel module with Xevent user psace program.
 * Future kernel module will use kevent feature.
 */
#define MOUSEPS2	1	/* Standard ps2 mouse protocol */
#define MOUSEMS3	2	/* Microsoft 3 byte protocol   */
#define MOUSEPC5	3	/* PC 5 byte protocol          */
#define MOUSETS		4	/* touch screen mouse          */
#define KBDEVENT	5	/* keyboard data               */

#ifndef XLIB   
#ifdef __NetBSD__
#include <sys/types.h>
#include <sys/ioccom.h>
#else
#include <linux/ioctl.h>
#endif
#define X11_IOC_MAGIC	'R'
#define	X11GETPARAM	 _IOR(X11_IOC_MAGIC, 101, X11Info) /* get dev params */
#define	X11GETEVENTCOUNT _IOR(X11_IOC_MAGIC, 102, int) /* get number of events */
#endif
#endif
