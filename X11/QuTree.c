/* $XConsortium: QuTree.c,v 11.20 94/04/17 20:20:40 rws Exp $ */
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

int
XQueryTree(Display *	dpy,
	   Window	w,
	   Window *	root_return,
	   Window *	parent_return,
	   Window **	children_return,
	   unsigned int * nchildren_return)
{
  Window child, *children;
  int n, size;

  LockDisplay(dpy);
  size=128;
retry:
  n=size;
  children = (Window *) Xmalloc ((n+1) * sizeof (Window));
  if(!__XQueryChildren(dpy, w, root_return, parent_return, children, &n))
     {
     UnlockDisplay(dpy);
     Xfree(children);
     *children_return = NULL;
     return(0);
     }
  if(n < size)
     {
     *children_return = children;
     *nchildren_return = n;
     UnlockDisplay(dpy);
     return 1;
     }
  Xfree(children);
  size=n+1;
  goto  retry;
}
