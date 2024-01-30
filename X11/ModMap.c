/* $XConsortium: ModMap.c /main/15 1996/10/22 14:20:24 kaleb $ */
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

XModifierKeymap *
XGetModifierMapping(dpy)
     register Display *dpy;
{       
    unsigned long nbytes;
    XModifierKeymap *res;

    LockDisplay(dpy);
    nbytes = 16*sizeof(KeyCode);	/* keyspermod is always 2 */
    res = (XModifierKeymap *) Xmalloc(sizeof (XModifierKeymap));
    if (res) res->modifiermap = (KeyCode *) Xmalloc ((unsigned) nbytes);
    if ((! res) || (! res->modifiermap)) {
      if (res) Xfree((char *) res);
      res = (XModifierKeymap *) NULL;
    } else {
      __XReadModifierMapping(dpy, res->modifiermap);
      res->max_keypermod = 2;
    }
    UnlockDisplay(dpy);
    return (res);
}
/*
 *	Returns:
 *	0	Success
 *	1	Busy - one or more old or new modifiers are down
 *	2	Failed - one or more new modifiers unacceptable
 */
int
XSetModifierMapping(dpy, modifier_map)
    register Display *dpy;
    register XModifierKeymap *modifier_map;
{
  return (2);
}

XModifierKeymap *
XNewModifiermap(keyspermodifier)
    int keyspermodifier;
{
    XModifierKeymap *res = (XModifierKeymap *) Xmalloc((sizeof (XModifierKeymap)));
    if (res) {
	res->max_keypermod = keyspermodifier;
	res->modifiermap = (keyspermodifier > 0 ?
			    (KeyCode *) Xmalloc((unsigned) (8 * keyspermodifier))
			    : (KeyCode *) NULL);
	if (keyspermodifier && (res->modifiermap == NULL)) {
	    Xfree((char *) res);
	    return (XModifierKeymap *) NULL;
	}
    }
    return (res);
}


XFreeModifiermap(map)
    XModifierKeymap *map;
{
    if (map) {
	if (map->modifiermap)
	    Xfree((char *) map->modifiermap);
	Xfree((char *) map);
    }
    return 1;
}

#if NeedFunctionPrototypes
XModifierKeymap *
XInsertModifiermapEntry(XModifierKeymap *map,
#if NeedWidePrototypes
			unsigned int keycode,
#else
			KeyCode keycode,
#endif
			int modifier)
#else
XModifierKeymap *
XInsertModifiermapEntry(map, keycode, modifier)
    XModifierKeymap *map;
    KeyCode keycode;
    int modifier;
#endif
{
    XModifierKeymap *newmap;
    int i,
	row = modifier * map->max_keypermod,
	newrow,
	lastrow;

    for (i=0; i<map->max_keypermod; i++) {
        if (map->modifiermap[ row+i ] == keycode)
	    return(map); /* already in the map */
        if (map->modifiermap[ row+i ] == 0) {
            map->modifiermap[ row+i ] = keycode;
	    return(map); /* we added it without stretching the map */
	}
    }   

    /* stretch the map */
    if ((newmap = XNewModifiermap(map->max_keypermod+1)) == NULL)
	return (XModifierKeymap *) NULL;
    newrow = row = 0;
    lastrow = newmap->max_keypermod * 8;
    while (newrow < lastrow) {
	for (i=0; i<map->max_keypermod; i++)
	    newmap->modifiermap[ newrow+i ] = map->modifiermap[ row+i ];
	newmap->modifiermap[ newrow+i ] = 0;
	row += map->max_keypermod;
	newrow += newmap->max_keypermod;
    }
    (void) XFreeModifiermap(map);
    newrow = newmap->max_keypermod * modifier + newmap->max_keypermod - 1;
    newmap->modifiermap[ newrow ] = keycode;
    return(newmap);
}

#if NeedFunctionPrototypes
XModifierKeymap *
XDeleteModifiermapEntry(XModifierKeymap *map,
#if NeedWidePrototypes
			unsigned int keycode,
#else
			KeyCode keycode,
#endif
			int modifier)
#else
XModifierKeymap *
XDeleteModifiermapEntry(map, keycode, modifier)
    XModifierKeymap *map;
    KeyCode keycode;
    int modifier;
#endif
{
    int i,
	row = modifier * map->max_keypermod;

    for (i=0; i<map->max_keypermod; i++) {
        if (map->modifiermap[ row+i ] == keycode)
            map->modifiermap[ row+i ] = 0;
    }
    /* should we shrink the map?? */
    return (map);
}
