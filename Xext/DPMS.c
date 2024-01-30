/* $Xorg: DPMS.c,v 1.3 2000/08/17 19:45:50 cpqbld Exp $ */
/*****************************************************************

Copyright (c) 1996 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING, 
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation 
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital 
Equipment Corporation.

******************************************************************/
/* $XFree86: xc/lib/Xext/DPMS.c,v 3.6 2002/10/16 02:19:22 dawes Exp $ */

/*
 * HISTORY
 */

#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/dpmsstr.h>
#include <stdio.h>

/*****************************************************************************
 *                                                                           *
 *                  public routines                                          *
 *                                                                           *
 *****************************************************************************/

Bool
DPMSQueryExtension (Display *dpy, int *event_basep, int *error_basep)
{
   *event_basep = 0;
   *error_basep = 0;
   return True;
}

Status
DPMSGetVersion(Display *dpy, int *major_versionp, int *minor_versionp)
{

    LockDisplay (dpy);
    *major_versionp = DPMSMajorVersion;
    *minor_versionp = DPMSMinorVersion;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

Bool
DPMSCapable(Display *dpy)
{
    return True;
}

Status
DPMSSetTimeouts(Display *dpy, CARD16 standby, CARD16 suspend, CARD16 off)
{
    if ((off != 0)&&(off < suspend)) 
    {
	return BadValue;
    }
    if ((suspend != 0)&&(suspend < standby))
    {
	return BadValue;
    }  

    LockDisplay(dpy);
    _DPMSSetTimeouts(dpy, (int)standby, (int)suspend, (int)off);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Bool
DPMSGetTimeouts(Display *dpy, CARD16 *standby, CARD16 *suspend, CARD16 *off)
{
    int istandby, isuspend, ioff;
    LockDisplay(dpy);
    _DPMSGetTimeouts(dpy, &istandby, &isuspend, &ioff);
    UnlockDisplay(dpy);
    SyncHandle();
    *standby=(CARD16)istandby;
    *suspend=(CARD16)isuspend;
    *off=(CARD16)ioff;
    return 1;
}

Status
DPMSEnable(Display *dpy)
{
    LockDisplay(dpy);
    _DPMSEnable(dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
DPMSDisable(Display *dpy)
{
    LockDisplay(dpy);
    _DPMSDisable(dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}


Status
DPMSForceLevel(Display *dpy, CARD16 level)
{

    if ((level != DPMSModeOn) &&
	(level != DPMSModeStandby) &&
	(level != DPMSModeSuspend) &&
	(level != DPMSModeOff))
	return BadValue;

    LockDisplay(dpy);
    _DPMSForceLevel(dpy, (int)level);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
DPMSInfo(Display *dpy, CARD16 *power_level, BOOL *state)
{
    int ilevel, istate;
    
    LockDisplay(dpy);
    _DPMSInfo(dpy, &ilevel, &istate);
    UnlockDisplay(dpy);
    SyncHandle();
    *power_level=(CARD16)ilevel;
    *state=(BOOL)istate;
    return 1;
}



