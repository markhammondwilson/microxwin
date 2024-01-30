/*								 
 * Module Name - xproperty.c
 *
 * Description -
 *     Xlib property.
 *
 * Copyright (C) 2000-2007  Vasant Kanchan, http://www.microxwin.com
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
#include "Xlibint.h"

/*
 * Obtain the atom type and property format for a window.
 */
int XGetWindowProperty(
    Display*		display,
    Window		w,
    Atom		property,
    long		long_offset,
    long		long_length,
    Bool		delete,
    Atom		req_type,
    Atom*		actual_type_return,
    int*		actual_format_return,
    unsigned long*	nitems_return,
    unsigned long*	bytes_after_return,
    unsigned char**	prop_return)
{
    int err;
    char *data;
    
    LockDisplay(display);
    /* limit request to max_request_size */
    if(long_length > display->max_request_size)
       long_length = display->max_request_size;
    data = (char *) Xmalloc (4*long_length+1);
    *nitems_return=0;
    *bytes_after_return=0; /* caller may check nitems, clear it */
    err=__XGetWindowProperty(display, w, property, long_offset, long_length,
          delete, req_type, actual_type_return, actual_format_return, 
          nitems_return, bytes_after_return, data);
   if(err==0)
      {
      UnlockDisplay(display);
      Xfree(data);
      *prop_return=NULL;
      return(Success); 	/* always return Success */
      }
   *prop_return=data;
   UnlockDisplay(display);
   return(Success);
}
/*
 * Get the property list for a window.
 */
Atom *XListProperties(
    Display*		dpy,
    Window		w,
    int*		num_prop_return)
{
  int length, size;
  Atom *aptr;
  size=128;	/* assume this size is ok */

  LockDisplay(dpy);
retry:  
  length=size;
  aptr = (Atom *) Xmalloc(size*sizeof(Atom));
  if(!__XListProperties(dpy, w, &length, aptr))
     {
     UnlockDisplay(dpy);
     Xfree(aptr);
     return(NULL);
     }
  if(length < size)
     {
      UnlockDisplay(dpy);
     *num_prop_return=length;
     return(aptr);
     }
  Xfree(aptr);
  size =length+1;
  goto	retry;
}


