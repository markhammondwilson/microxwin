/* $Xorg: IntAtom.c,v 1.5 2001/02/09 02:03:34 xorgcvs Exp $ */
/*

Copyright 1986, 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/X11/IntAtom.c,v 1.6 2001/12/14 19:54:02 dawes Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"
#include "Xintatom.h"

#define HASH(sig) ((sig) & (TABLESIZE-1))
#define REHASHVAL(sig) ((((sig) % (TABLESIZE-3)) + 2) | 1)
#define REHASH(idx,rehash) ((idx + rehash) & (TABLESIZE-1))

void
_XFreeAtomTable(dpy)
    Display *dpy;
{
    register Entry *table;
    register int i;
    register Entry e;

    if (dpy->atoms) {
	table = dpy->atoms->table;
	for (i = TABLESIZE; --i >= 0; ) {
	    if ((e = *table++) && (e != RESERVED))
		Xfree((char *)e);
	}
	Xfree((char *)dpy->atoms);
    }
}

static
Atom _XInternAtom(
    Display *dpy,
    _Xconst char *name,
    Bool onlyIfExists)
{
    register AtomTable *atoms;
    register char *s1, c, *s2;
    register unsigned long sig;
    register int idx = 0, i;
    Entry e;
    int n, firstidx, rehash = 0;
    Atom atom;

    /* look in the cache first */
    if (!(atoms = dpy->atoms)) {
	dpy->atoms = atoms = (AtomTable *)Xcalloc(1, sizeof(AtomTable));
	dpy->free_funcs->atoms = _XFreeAtomTable;
    }
    sig = 0;
    for (s1 = (char *)name; (c = *s1++); )
	sig += c;
    n = s1 - (char *)name - 1;
    if (atoms) {
	firstidx = idx = HASH(sig);
	while ((e = atoms->table[idx])) {
	    if (e != RESERVED && e->sig == sig) {
	    	for (i = n, s1 = (char *)name, s2 = EntryName(e); --i >= 0; ) {
		    if (*s1++ != *s2++)
		    	goto nomatch;
	    	}
	    	if (!*s2)
		    return e->atom;
	    }
nomatch:    if (idx == firstidx)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    if (idx == firstidx)
		break;
	}
    }
    /* call the kernel & update cache */
    if(atom= __XInternAtom(dpy, name, onlyIfExists))
       _XUpdateAtomCache(dpy, name, atom, sig, idx, n);
    return (atom);
}

void
_XUpdateAtomCache(dpy, name, atom, sig, idx, n)
    Display *dpy;
    const char *name;
    Atom atom;
    unsigned long sig;
    int idx;
    int n;
{
    Entry e, oe;
    register char *s1;
    register char c;
    int firstidx, rehash;

    if (!dpy->atoms) {
	if (idx < 0) {
	    dpy->atoms = (AtomTable *)Xcalloc(1, sizeof(AtomTable));
	    dpy->free_funcs->atoms = _XFreeAtomTable;
	}
	if (!dpy->atoms)
	    return;
    }
    if (!sig) {
	for (s1 = (char *)name; (c = *s1++); )
	    sig += c;
	n = s1 - (char *)name - 1;
	if (idx < 0) {
	    firstidx = idx = HASH(sig);
	    if (dpy->atoms->table[idx]) {
		rehash = REHASHVAL(sig);
		do
		    idx = REHASH(idx, rehash);
		while (idx != firstidx && dpy->atoms->table[idx]);
	    }
	}
    }
    e = (Entry)Xmalloc(sizeof(EntryRec) + n + 1);
    if (e) {
	e->sig = sig;
	e->atom = atom;
	strcpy(EntryName(e), name);
	if ((oe = dpy->atoms->table[idx]) && (oe != RESERVED))
	    Xfree((char *)oe);
	dpy->atoms->table[idx] = e;
    }
}

#if NeedFunctionPrototypes
Atom XInternAtom (
    Display *dpy,
    const char *name,
    Bool onlyIfExists)
#else
Atom XInternAtom (dpy, name, onlyIfExists)
    Display *dpy;
    char *name;
    Bool onlyIfExists;
#endif
{
    Atom atom;

    if (!name)
	name = "";
    LockDisplay(dpy);
    atom = _XInternAtom(dpy, name, onlyIfExists);
    UnlockDisplay(dpy);
    return atom;
}


Status
XInternAtoms (dpy, names, count, onlyIfExists, atoms_return)
    Display *dpy;
    char **names;
    int count;
    Bool onlyIfExists;
    Atom *atoms_return;
{
    int i;

    LockDisplay(dpy);
    for (i = 0; i < count; i++) {
	atoms_return[i] = _XInternAtom(dpy, names[i], onlyIfExists);
    }
    UnlockDisplay(dpy);
    return 0;
}

