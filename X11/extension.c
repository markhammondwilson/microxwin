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
/* $XConsortium: QuExt.c,v 11.19 94/04/17 20:20:36 rws Exp $ */
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
#include "XShm.h"
#include "shmstr.h"
//#include <X11/extensions/Xvlib.h>
#include <X11/extensions/dpmsstr.h>
#include <X11/extensions/render.h>
#define SHAPENAME "SHAPE"
#define XTestExtensionName	"XTEST"
int ShmReqCode=200;
int ShmEventBase=LASTEvent;
int DpmsReqCode=300;
int XTestReqCode=400;
int ShapeReqCode=500;
int ShapeEventBase=500;
int XvReqCode=600;
int XvEventBase=600;
int XrenderReqCode=700;
int XrenderEventBase=700;
int XrenderErrorBase=700;
static int maxextensions=6;
static XExtCodes Shm, Xv, Shape, Dpms, Xtst, Xrender;

#if NeedFunctionPrototypes
Bool XQueryExtension(
    register Display *dpy,
    _Xconst char *name,
    int *major_opcode,  /* RETURN */
    int *first_event,   /* RETURN */
    int *first_error)	/* RETURN */
#else
Bool XQueryExtension(dpy, name, major_opcode, first_event, first_error)
    register Display *dpy;
    char *name;
    int *major_opcode;  /* RETURN */
    int *first_event;   /* RETURN */
    int *first_error;	/* RETURN */
#endif
{       
   LockDisplay (dpy);
   if(strcmp(name, DPMSExtensionName) == 0)
      {
      *major_opcode=DpmsReqCode;
      *first_event=0;  
      *first_error=0;
      UnlockDisplay (dpy);
      return True;
      }
#ifdef XTESTEXT
   else  if(strcmp(name, XTestExtensionName) == 0)
      {
      *major_opcode=XTestReqCode;
      *first_event=0;  
      *first_error=0;
      UnlockDisplay (dpy);
      return True;
      }
#endif
#ifdef MITSHM
   else if(strcmp(name, SHMNAME) == 0)
      {
      *major_opcode=ShmReqCode;
      *first_event=ShmEventBase;  
      *first_error=0;
      UnlockDisplay (dpy);
      return True;
      }
#endif
#ifdef XLIBRENDER
   else  if(strcmp(name, RENDER_NAME) == 0)
      {
      *major_opcode=XrenderReqCode;
      *first_event=XrenderEventBase;  
      *first_error=XrenderErrorBase;
      UnlockDisplay (dpy);
      return True;
      }
#endif
#ifdef SHAPE
   else  if(strcmp(name, SHAPENAME) == 0)
      {
      *major_opcode=ShapeReqCode;
      *first_event=ShapeEventBase;  
      *first_error=0;
      UnlockDisplay (dpy);
      return True;
      }
#endif
#ifdef XV
   else  if(strcmp(name, XvName) == 0)
      {
      *major_opcode=XvReqCode;
      *first_event=XvEventBase;  
      *first_error=0;
      UnlockDisplay (dpy);
      return True;
      }
#endif
   UnlockDisplay (dpy);
   return False;	/* Extensions not supported */
}
char **XListExtensions(dpy, nextensions)
register Display *dpy;
int *nextensions;	/* RETURN */
{
   int i;
   char **list;
   i=0;
   LockDisplay (dpy);

   list=(char **)Xmalloc((maxextensions+1)*sizeof(char *));
   list[i++]=strdup(DPMSExtensionName);
#ifdef XTESTEXT
   list[i++]=strdup(XTestExtensionName);
#endif
#ifdef MITSHM
   list[i++]=strdup(SHMNAME);
#endif
#ifdef XLIBRENDER
   list[i++]=strdup(RENDER_NAME);
#endif
#ifdef SHAPE
   list[i++]=strdup(SHAPENAME);
#endif
#ifdef XV
   list[i++]=strdup(XvName);
#endif
   *nextensions=i;
   /* clear the rest */
   while(i <= maxextensions)
      list[i++]=NULL;
   UnlockDisplay (dpy);
   return(list);
}

XFreeExtensionList (list)
char **list;
{
   int i;
   
   if(list)
      {
      for(i=0; i < maxextensions; ++i)
         {
         if(list[i])
            free(list[i]);
         }
      Xfree(list);
      }	 
   return;	    
}
/*
 * This routine is used to link a extension in so it will be called
 * at appropriate times.
 */

#if NeedFunctionPrototypes
XExtCodes *XInitExtension (
	Display *dpy,
	_Xconst char *name)
#else
XExtCodes *XInitExtension (dpy, name)
	Display *dpy;
	char *name;
#endif
{
   LockDisplay (dpy);
   if(strcmp(name, DPMSExtensionName) == 0)
      {
      Dpms.major_opcode=DpmsReqCode;
      Dpms.first_event=0;  
      Dpms.extension=1;
      Dpms.first_error=0;
      UnlockDisplay (dpy);
      return (&Dpms);
      }
#ifdef XTESTEXT
   else if(strcmp(name, XTestExtensionName) == 0)
      {
      Xtst.major_opcode=XTestReqCode;
      Xtst.first_event=0;  
      Xtst.extension=1;
      Xtst.first_error=0;
      UnlockDisplay (dpy);
      return (&Xtst);
      }
#endif
#ifdef MITSHM
   else if(strcmp(name, SHMNAME) == 0)
      {
      Shm.major_opcode=ShmReqCode;
      Shm.first_event=ShmEventBase;  
      Shm.extension=1;
      Shm.first_error=0;
      UnlockDisplay (dpy);
      return (&Shm);
      }
#endif
#ifdef XLIBRENDER
   else if(strcmp(name, RENDER_NAME) == 0)
      {
      Xrender.major_opcode=XrenderReqCode;
      Xrender.first_event=XrenderEventBase;  
      Xrender.extension=1;
      Xrender.first_error=XrenderErrorBase;
      UnlockDisplay (dpy);
      return (&Xrender);
      }
#endif
#ifdef SHAPE
   else if(strcmp(name, SHAPENAME) == 0)
      {
      Shape.major_opcode=ShapeReqCode;
      Shape.first_event=ShapeEventBase;  
      Shape.extension=1;
      Shape.first_error=0;
      UnlockDisplay (dpy);
      return (&Shape);
      }
#endif
#ifdef XV
   else if(strcmp(name, XvName) == 0)
      {
      Xv.major_opcode=XvReqCode;
      Xv.first_event=XvEventBase;  
      Xv.extension=1;
      Xv.first_error=0;
      UnlockDisplay (dpy);
      return (&Xv);
      }
#endif
   UnlockDisplay (dpy);
   return (XExtCodes *) NULL;
}

XExtCodes *XAddExtension (dpy)
    Display *dpy;
{
    register _XExtension *ext;

    LockDisplay (dpy);
    if (! (ext = (_XExtension *) Xcalloc (1, sizeof (_XExtension)))) {
	UnlockDisplay(dpy);
	return (XExtCodes *) NULL;
    }
    ext->codes.extension = dpy->ext_number++;

    /* chain it onto the display list */
    ext->next = dpy->ext_procs;
    dpy->ext_procs = ext;
    UnlockDisplay (dpy);

    return (&ext->codes);		/* tell him which extension */
}

CloseDisplayType XESetCloseDisplay(dpy, extension, proc)
	Display *dpy;		/* display */
	int extension;		/* extension number */
	CloseDisplayType proc;	/* routine to call when display closed */
{
	return (CloseDisplayType)NULL;
}

XExtData **XEHeadOfExtensionList(object)
    XEDataObject object;
{
    return *(XExtData ***)&object;
}

XExtData *XFindOnExtensionList(structure, number)
    XExtData **structure;
    int number;
{
    XExtData *ext;

    ext = *structure;
    while (ext && (ext->number != number))
	ext = ext->next;
    return ext;
}


int
XAddToExtensionList(structure, ext_data)
    XExtData **structure;
    XExtData *ext_data;
{
    ext_data->next = *structure;
    *structure = ext_data;
    return 1;
}

ErrorType XESetError(dpy, extension, proc)
	Display *dpy;		/* display */
	int extension;		/* extension number */
	ErrorType proc;		/* routine to call when X error happens */
{
	return (NULL);
}

typedef Bool (*WireToEventType) (
#if NeedFunctionPrototypes
    Display*	/* display */,
    XEvent*	/* re */,
    xEvent*	/* event */
#endif
);

WireToEventType XESetWireToEvent(dpy, event_number, proc)
	Display *dpy;		/* display */
	WireToEventType proc;	/* routine to call when converting event */
	int event_number;	/* event routine to replace */
{
	return (NULL);
}
typedef Status (*EventToWireType) (
#if NeedFunctionPrototypes
    Display*	/* display */,
    XEvent*	/* re */,
    xEvent*	/* event */
#endif
);

EventToWireType XESetEventToWire(dpy, event_number, proc)
	Display *dpy;		/* display */
	EventToWireType proc;	/* routine to call when converting event */
	int event_number;	/* event routine to replace */
{
	return (NULL);
}
