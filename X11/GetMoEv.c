/* $XConsortium: GetMoEv.c,v 11.21 94/04/17 20:19:37 rws Exp $ */
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

#define NEED_REPLIES
#include "Xlibint.h"

XTimeCoord *XGetMotionEvents(dpy, w, start, stop, nEvents)
    register Display *dpy;
    Time start, stop;
    Window w;
    int *nEvents;  /* RETURN */
{       
   XTimeCoord *tc;

   LockDisplay(dpy);

   tc = (XTimeCoord *)Xmalloc(128*sizeof(XTimeCoord));
   if(!tc)
      {
      UnlockDisplay(dpy);
      return (NULL);
      }

   *nEvents=128;
   if(!__XGetMotionEvents(dpy, w, start, stop, nEvents, tc))
      {
      UnlockDisplay(dpy);
      *nEvents=0;
      Xfree(tc);
      return (NULL);
      }
   UnlockDisplay(dpy);
   return (tc);	
}

