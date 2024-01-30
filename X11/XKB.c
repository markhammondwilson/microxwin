/* $Xorg: XKB.c,v 1.3 2000/08/17 19:44:59 cpqbld Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XFree86: xc/lib/X11/XKB.c,v 1.6 2001/10/28 03:32:33 tsi Exp $ */

#include <stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xlibint.h"
#include <X11/extensions/XKBproto.h>
#include "XKBlibint.h"

Bool 
#if NeedFunctionPrototypes
XkbQueryExtension(	Display *dpy,
			int *	opcodeReturn,
			int *	eventBaseReturn,
			int *	errorBaseReturn,
			int *	majorReturn,
			int *	minorReturn)
#else
XkbQueryExtension(dpy,opcodeReturn,eventBaseReturn,errorBaseReturn,
						majorReturn,minorReturn)
    Display *dpy;
    int *opcodeReturn;
    int *eventBaseReturn;
    int *errorBaseReturn;
    int *majorReturn;
    int *minorReturn;
#endif
{
  return False;
}

Bool 
#if NeedFunctionPrototypes
XkbLibraryVersion(int *libMajorRtrn,int *libMinorRtrn)
#else
XkbLibraryVersion(libMajorRtrn,libMinorRtrn)
    int *libMajorRtrn;
    int *libMinorRtrn;
#endif
{
    *libMajorRtrn = 0;
    *libMinorRtrn = 0;
    return False;
}

Bool
#if NeedFunctionPrototypes
XkbSelectEvents(	Display *	dpy,
			unsigned int 	deviceSpec,
			unsigned int 	affect,
			unsigned int 	selectAll)
#else
XkbSelectEvents(dpy,deviceSpec,affect,selectAll)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int selectAll;
#endif
{
   return False;
}

Bool
#if NeedFunctionPrototypes
XkbSelectEventDetails(	Display *		dpy,
			unsigned 		deviceSpec,
			unsigned 		eventType,
			unsigned long int 	affect,
			unsigned long int 	details)
#else
XkbSelectEventDetails(dpy,deviceSpec,eventType,affect,details)
    Display *dpy;
    unsigned deviceSpec;
    unsigned eventType;
    unsigned long int affect;
    unsigned long int details;
#endif
{
    return False;
}

Bool
#if NeedFunctionPrototypes
XkbLockModifiers(	Display *	dpy,
			unsigned int 	deviceSpec,
			unsigned int 	affect,
			unsigned int 	values)
#else
XkbLockModifiers(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int values;
#endif
{
    return False;
}

Bool
#if NeedFunctionPrototypes
XkbLatchModifiers(	Display *	dpy,
			unsigned int	deviceSpec,
			unsigned int	affect,
			unsigned int	values)
#else
XkbLatchModifiers(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int values;
#endif
{
    return False;
}

Bool
#if NeedFunctionPrototypes
XkbLockGroup(Display *dpy,unsigned int deviceSpec,unsigned int group)
#else
XkbLockGroup(dpy,deviceSpec,group)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int group;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbLatchGroup(Display *dpy,unsigned int deviceSpec,unsigned int group)
#else
XkbLatchGroup(dpy,deviceSpec,group)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int group;
#endif
{
	return False;
}

unsigned
#if NeedFunctionPrototypes
XkbSetXlibControls(Display *dpy,unsigned affect,unsigned values)
#else
XkbSetXlibControls(dpy,affect,values)
    Display *	dpy;
    unsigned	affect;
    unsigned	values;
#endif
{
	return False;
}

unsigned
#if NeedFunctionPrototypes
XkbGetXlibControls(Display *dpy)
#else
XkbGetXlibControls(dpy)
    Display *	dpy;
#endif
{
	return 0;
}

unsigned int
#if NeedFunctionPrototypes
XkbXlibControlsImplemented(void)
#else
XkbXlibControlsImplemented()
#endif
{
    return 0;
}

Bool
#if NeedFunctionPrototypes
XkbSetDebuggingFlags(	Display *	dpy,
			unsigned int 	mask,
			unsigned int 	flags,
			char *		msg,
			unsigned int	ctrls_mask,
			unsigned int	ctrls,
			unsigned int *	rtrn_flags,
			unsigned int *	rtrn_ctrls)
#else
XkbSetDebuggingFlags(dpy,mask,flags,msg,ctrls_mask,ctrls,rtrn_flags,rtrn_ctrls)
    Display *		dpy;
    unsigned int 	mask;
    unsigned int 	flags;
    char *		msg;
    unsigned int 	ctrls_mask;
    unsigned int 	ctrls;
    unsigned int *	rtrn_flags;
    unsigned int *	rtrn_ctrls;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbComputeEffectiveMap(	XkbDescPtr 	xkb,
			XkbKeyTypePtr 	type,
			unsigned char *	map_rtrn)
#else
XkbComputeEffectiveMap(xkb,type,map_rtrn)
    XkbDescPtr		xkb;
    XkbKeyTypePtr	type;
    unsigned char *	map_rtrn;
#endif
{
	return False;
}

Status
#if NeedFunctionPrototypes
XkbGetState(Display *dpy,unsigned deviceSpec,XkbStatePtr rtrn)
#else
XkbGetState(dpy,deviceSpec,rtrn)
    Display *	dpy;
    unsigned 	deviceSpec;
    XkbStatePtr	rtrn;
#endif
{
	return BadAccess;
}

Bool
#if NeedFunctionPrototypes
XkbSetDetectableAutoRepeat(Display *dpy,Bool detectable,Bool *supported)
#else
XkbSetDetectableAutoRepeat(dpy,detectable,supported)
    Display *		dpy;
    Bool		detectable;
    Bool *		supported;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbGetDetectableAutoRepeat(Display *dpy,Bool *supported)
#else
XkbGetDetectableAutoRepeat(dpy,supported)
    Display *		dpy;
    Bool *		supported;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbSetAutoResetControls(	Display *	dpy,
				unsigned 	changes,
				unsigned *	auto_ctrls,
				unsigned *	auto_values)
#else
XkbSetAutoResetControls(dpy,changes,auto_ctrls,auto_values)
    Display *		dpy;
    unsigned 		changes;
    unsigned *		auto_ctrls;
    unsigned *		auto_values;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbGetAutoResetControls(	Display *	dpy,
				unsigned *	auto_ctrls,
				unsigned *	auto_ctrl_values)
#else
XkbGetAutoResetControls(dpy,auto_ctrls,auto_ctrl_values)
    Display *		dpy;
    unsigned *		auto_ctrls;
    unsigned *		auto_ctrl_values;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbSetPerClientControls(	Display *	dpy,
				unsigned 	change,
				unsigned *	values)
#else
XkbSetPerClientControls(dpy,change,values)
    Display *		dpy;
    unsigned 		change;
    unsigned *		values;
#endif
{
	return False;
}

Bool
#if NeedFunctionPrototypes
XkbGetPerClientControls(	Display *	dpy,
				unsigned *	ctrls)
#else
XkbGetPerClientControls(dpy,ctrls)
    Display *		dpy;
    unsigned *		ctrls;
#endif
{
	return False;
}

Display *
#if NeedFunctionPrototypes
XkbOpenDisplay(	char *	name,
		int *	ev_rtrn,
		int *	err_rtrn,
		int *	major_rtrn,
		int *	minor_rtrn,
		int *	reason)
#else
XkbOpenDisplay(name,ev_rtrn,err_rtrn,major_rtrn,minor_rtrn,reason)
    char *	name;
    int *	ev_rtrn;
    int *	err_rtrn;
    int *	major_rtrn;
    int *	minor_rtrn;
    int *	reason;
#endif
{
	    *reason= XkbOD_NonXkbServer;
	    return NULL;
}

void
#if NeedFunctionPrototypes
XkbSetAtomFuncs(XkbInternAtomFunc getAtom,XkbGetAtomNameFunc getName)
#else
XkbSetAtomFuncs(getAtom,getName)
    XkbInternAtomFunc	getAtom;
    XkbGetAtomNameFunc	getName;
#endif
{
    return;
}

XkbDescPtr
#if NeedFunctionPrototypes
XkbGetMap(Display *dpy,unsigned which,unsigned deviceSpec)
#else
XkbGetMap(dpy,which,deviceSpec)
    Display *dpy;
    unsigned which;
    unsigned deviceSpec;
#endif
{
    return NULL;
}
Status
#if NeedFunctionPrototypes
XkbGetUpdatedMap(Display *dpy,unsigned which,XkbDescPtr xkb)
#else
XkbGetUpdatedMap(dpy,which,xkb)
    Display *	dpy;
    unsigned 	which;
    XkbDescPtr	xkb;
#endif
{
   return BadAccess;
}
Status
#if NeedFunctionPrototypes
XkbGetNames(Display *dpy,unsigned which,XkbDescPtr xkb)
#else
XkbGetNames(dpy,which,xkb)
    Display *	dpy;
    unsigned	which;
    XkbDescPtr	xkb;
#endif
{
	return BadAccess;
}
Bool
#if NeedFunctionPrototypes
XkbIgnoreExtension(Bool ignore)
#else
XkbIgnoreExtension(ignore)
    Bool ignore;
#endif
{
    return True;
}

void
#if NeedFunctionPrototypes
XkbFreeClientMap(XkbDescPtr xkb,unsigned what,Bool freeMap)
#else
XkbFreeClientMap(xkb,what,freeMap)
    XkbDescPtr	xkb;
    unsigned	what;
    Bool	freeMap;
#endif
{
    return;
}
unsigned
#if NeedFunctionPrototypes
XkbKeysymToModifiers(Display *dpy,KeySym ks)
#else
XkbKeysymToModifiers(dpy,ks)
    Display *dpy;
    KeySym ks;
#endif
{
   return 0;
}
Bool
#if NeedFunctionPrototypes
XkbChangeEnabledControls(	Display *	dpy,
				unsigned	deviceSpec,
				unsigned	affect,
				unsigned	values)
#else
XkbChangeEnabledControls(dpy,deviceSpec,affect,values)
    Display *	dpy;
    unsigned	deviceSpec;
    unsigned	affect;
    unsigned	values;
#endif
{
   return False;
}

Status 
#if NeedFunctionPrototypes
XkbGetControls(Display *dpy, unsigned long which, XkbDescPtr xkb)
#else
XkbGetControls(dpy, which, xkb)
    Display *dpy;
    unsigned long which;
    XkbDescPtr xkb;
#endif
{
    return BadAccess;
}

Bool 
#if NeedFunctionPrototypes
XkbSetControls(Display *dpy, unsigned long which, XkbDescPtr xkb)
#else
XkbSetControls(dpy, which, xkb)
    Display *		dpy;
    unsigned long 	which;
    XkbDescPtr		xkb;
#endif
{
    return False;
}
void
#if NeedFunctionPrototypes
XkbFreeKeyboard(XkbDescPtr xkb,unsigned which,Bool freeAll)
#else
XkbFreeKeyboard(xkb,which,freeAll)
    XkbDescPtr	xkb;
    unsigned	which;
    Bool	freeAll;
#endif
{
    return;
}
Bool
#if NeedFunctionPrototypes
XkbLookupKeySym(	register Display *	dpy,
			KeyCode 		key,
			register unsigned int 	mods,
			unsigned int *		mods_rtrn,
			KeySym *		keysym_rtrn)
#else
XkbLookupKeySym(dpy, key, mods, mods_rtrn, keysym_rtrn)
    register Display *dpy;
    KeyCode key;
    register unsigned int mods;
    unsigned int *mods_rtrn;
    KeySym *keysym_rtrn;
#endif
{
	return _XTranslateKey(dpy, key, mods, mods_rtrn, keysym_rtrn);
}

XkbDescPtr
#if NeedFunctionPrototypes
XkbGetKeyboard(Display *dpy,unsigned which,unsigned deviceSpec)
#else
XkbGetKeyboard(dpy,which,deviceSpec)
    Display *		dpy;
    unsigned		which;
    unsigned		deviceSpec;
#endif
{
	return NULL;
}
Bool
#if NeedFunctionPrototypes
XkbVirtualModsToReal(XkbDescPtr xkb,unsigned virtual_mask,unsigned *mask_rtrn)
#else
XkbVirtualModsToReal(xkb,virtual_mask,mask_rtrn)
    XkbDescPtr	xkb;
    unsigned	virtual_mask;
    unsigned *	mask_rtrn;
#endif
{
	return False;
}
