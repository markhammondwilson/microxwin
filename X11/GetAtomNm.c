/* $Xorg: GetAtomNm.c,v 1.5 2001/02/09 02:03:33 xorgcvs Exp $ */
/*

Copyright 1986, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/X11/GetAtomNm.c,v 3.5 2001/12/14 19:54:00 dawes Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"
#include "Xintatom.h"

static
char *_XGetAtomName(
    Display *dpy,
    Atom atom)
{
    char *name;
    register Entry *table;
    register int idx;
    register Entry e;
    int length, size;

    if (dpy->atoms) {
	table = dpy->atoms->table;
	for (idx = TABLESIZE; --idx >= 0; ) {
	    if ((e = *table++) && (e->atom == atom)) {
		idx = strlen(EntryName(e)) + 1;
		if ((name = (char *)Xmalloc(idx)))
		    strcpy(name, EntryName(e));
		return name;
	    }		
	}
    }

    size=128;	/* assume this size is ok */
retry:  
    length=size;
    name = (char *) Xmalloc(size);
    if(!__XGetAtomName(dpy, atom, &length, name))
       {
       Xfree(name);
       return(NULL);
       }
    if(length < size)
      {
      _XUpdateAtomCache(dpy, name, atom, 0, -1, 0);
      return(name);
      }
    Xfree(name);
    size =length+1;
    goto retry;
}

/*
 * Get a string name for a property given in atom.
 */
char *XGetAtomName(dpy, atom)
    register Display *dpy;
    Atom atom;
{
    char *name;

    LockDisplay(dpy);
    name = _XGetAtomName(dpy, atom);
    UnlockDisplay(dpy);
    return (name);
}



Status
XGetAtomNames (dpy, atoms, count, names_return)
    Display *dpy;
    Atom *atoms;
    int count;
    char **names_return;
{
    int i;
    
    LockDisplay(dpy);
    for (i = 0; i < count; i++) {
	names_return[i] = _XGetAtomName(dpy, atoms[i]);
    }
    UnlockDisplay(dpy);
    return 1;
}
