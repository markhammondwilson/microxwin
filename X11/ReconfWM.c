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
/* $XConsortium: ReconfWM.c,v 1.9 94/04/17 20:20:45 rws Exp $ */
/*

Copyright (c) 1986  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"

#define AllMaskBits (CWX|CWY|CWWidth|CWHeight|\
                     CWBorderWidth|CWSibling|CWStackMode)

Status XReconfigureWMWindow (dpy, w, screen, mask, changes)
    register Display *dpy;
    Window w;
    int screen;
    unsigned int mask;
    XWindowChanges *changes;
{
    XConfigureRequestEvent ev;
    Window root = RootWindow (dpy, screen);
    XEvent event;
    int count;

    /*
     * Only need to go through the trouble if we are actually changing the
     * stacking mode.
     */
    LockDisplay(dpy);
    if (!(mask & CWStackMode)) {
	XConfigureWindow (dpy, w, mask, changes);
        UnlockDisplay(dpy);
	return True;
    }

    _XEventsQueued (dpy, QueuedAfterFlush);	/* read any pending events */

    XConfigureWindow (dpy, w, mask, changes);

   if(!__XPendingEvent(dpy))
       {
       UnlockDisplay(dpy);
       return True;	/* no event generated */
       }
repeat:   
    if( (count=__XReadKernel(dpy, &event, 1)) < 0)	/* read event from kernel or EINTR */
       {
       UnlockDisplay(dpy);
       return True;
       }
    if(event.type == X_Error)
       goto sendevent;
    else   /* must be an event packet */
      _XEnq (dpy, (xEvent *)&event);
    if(__XPendingEvent(dpy))
       goto repeat;

    /*
     * If the request succeeded, then everything is okay; otherwise, send event
     */
    UnlockDisplay(dpy);
    return True;

sendevent:
    ev.type		= ConfigureRequest;
    ev.window		= w;
    ev.parent		= root;
    ev.value_mask	= (mask & AllMaskBits);
    ev.x		= changes->x;
    ev.y		= changes->y;
    ev.width		= changes->width;
    ev.height		= changes->height;
    ev.border_width	= changes->border_width;
    ev.above		= changes->sibling;
    ev.detail		= changes->stack_mode;
    UnlockDisplay(dpy);
    return (XSendEvent (dpy, root, False,
			SubstructureRedirectMask|SubstructureNotifyMask,
			(XEvent *)&ev));
}
