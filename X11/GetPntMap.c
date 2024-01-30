/* $XConsortium: GetPntMap.c,v 1.16 94/04/17 20:19:41 rws Exp $ */
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

#define NEED_REPLIES
#include "Xlibint.h"

#ifdef MIN		/* some systems define this in <sys/param.h> */
#undef MIN
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int XGetPointerMapping (dpy, map, nmaps)
    register Display *dpy;
    unsigned char *map;	/* RETURN */
    int nmaps;

{
  LockDisplay(dpy);
  if (--nmaps >= 0) map[0] = 1;
  if (--nmaps >= 0) map[1] = 2;
  if (--nmaps >= 0) map[2] = 3;
  UnlockDisplay(dpy);
  return ((int) 3);
}

KeySym *XGetKeyboardMapping (dpy, first_keycode, count, keysyms_per_keycode)
    register Display *dpy;
/*    unsigned int first_keycode; */
    KeyCode first_keycode; 
    int count;
    int *keysyms_per_keycode;		/* RETURN */
{
    register KeySym *mapping = NULL;

    if (first_keycode < dpy->min_keycode 
	|| (first_keycode + count - 1) > dpy->max_keycode)
      return (KeySym *) NULL;

    LockDisplay(dpy);
    if (count > 0) {
        /* 4 keysyms / keycode */
	mapping = (KeySym *) Xmalloc (4*count*sizeof (KeySym));
	if (! mapping) {
            UnlockDisplay(dpy);
	    return (KeySym *) NULL;
	}
        __XReadKeyboardMapping(dpy, mapping, first_keycode, count);
    }
    *keysyms_per_keycode = 4;
    UnlockDisplay(dpy);
    return (mapping);
}

int
XChangeKeyboardMapping (dpy, first_keycode, keysyms_per_keycode, 
		     keysyms, nkeycodes)
    register Display *dpy;
    int first_keycode;
    int keysyms_per_keycode;
    KeySym *keysyms;
    int nkeycodes;
    {

    if (first_keycode < dpy->min_keycode 
	|| (first_keycode + nkeycodes - 1) > dpy->max_keycode)
      return 0;
    if(keysyms_per_keycode > 4)
       return 0;	/* kernel table only supports max 4 */

    LockDisplay(dpy);
    __XWriteKeyboardMapping(dpy, keysyms, first_keycode, nkeycodes, keysyms_per_keycode);
    UnlockDisplay(dpy);
    SyncHandle();
    return 0;
    }

