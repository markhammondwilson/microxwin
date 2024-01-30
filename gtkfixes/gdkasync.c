/* GTK - The GIMP Toolkit
 * gdkasync.c: Utility functions using the Xlib asynchronous interfaces
 * Copyright (C) 2003, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* Portions of code in this file are based on code from Xlib
 */
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
#include <config.h>
#include <X11/Xlibint.h>
#include "gdkasync.h"
#include "gdkx.h"

typedef struct _ChildInfoChildState ChildInfoChildState;
typedef struct _ChildInfoState ChildInfoState;
typedef struct _ListChildrenState ListChildrenState;
typedef struct _SendEventState SendEventState;
typedef struct _SetInputFocusState SetInputFocusState;

typedef enum {
  CHILD_INFO_GET_PROPERTY,
  CHILD_INFO_GET_WA,
  CHILD_INFO_GET_GEOMETRY
} ChildInfoReq;

struct _ChildInfoChildState
{
  gulong seq[3];
};

struct _ChildInfoState
{
  gboolean get_wm_state;
  Window *children;
  guint nchildren;
  GdkChildInfoX11 *child_info;
  ChildInfoChildState *child_states;

  guint current_child;
  guint n_children_found;
  gint current_request;
  gboolean have_error;
  gboolean child_has_error;
};

struct _ListChildrenState
{
  Display *dpy;
  gulong get_property_req;
  gboolean have_error;
  gboolean has_wm_state;
};

struct _SendEventState
{
  Display *dpy;
  Window window;
  _XAsyncHandler async;
  gulong send_event_req;
  gulong get_input_focus_req;
  gboolean have_error;
  GdkSendXEventCallback callback;
  gpointer data;
};

struct _SetInputFocusState
{
  Display *dpy;
  _XAsyncHandler async;
  gulong set_input_focus_req;
  gulong get_input_focus_req;
};

void 
_gdk_x11_send_client_message_async (GdkDisplay           *display, 
				    Window                window, 
				    gboolean              propagate,
				    glong                 event_mask,
				    XClientMessageEvent  *event_send,
				    GdkSendXEventCallback callback,
				    gpointer              data)
{
  Display *dpy;
  SendEventState *state, se;
  int err;
    
  dpy = GDK_DISPLAY_XDISPLAY (display);

  state = &se;

  state->dpy = dpy;
  state->window = window;
  state->callback = callback;
  state->data = data;
  state->have_error = FALSE;
  if( (err=XSendEvent(dpy, window, propagate, event_mask, (XEvent *)event_send)) ==0)
     state->have_error=TRUE;
  XSync(dpy, 0);
  if (state->callback)
      state->callback (state->window, !state->have_error, state->data);
}

void
_gdk_x11_set_input_focus_safe (GdkDisplay             *display,
			       Window                  window,
			       int                     revert_to,
			       Time                    time)
{
  Display *dpy;
  dpy = GDK_DISPLAY_XDISPLAY (display);
  XSetInputFocus(dpy, window, revert_to, time);
  XSync(dpy, 0);
  return;
}

static gboolean
list_children_and_wm_state (Display      *dpy,
			    Window        w,
			    Atom          wm_state_atom,
			    gboolean     *has_wm_state,
			    Window      **children,
			    unsigned int *nchildren)
{
  ListChildrenState state;
  long nbytes, after;
  Atom actualtype;
  Window root, parent;
  int err, actualformat;
  unsigned char *data;
    
  *children = NULL;
  *nchildren = 0;
  *has_wm_state = FALSE;
  
  state.have_error = FALSE;
  state.has_wm_state = FALSE;

  if (wm_state_atom)
    {
    actualtype=None;
    err=XGetWindowProperty(dpy, w, wm_state_atom, 0, 0, False, AnyPropertyType,
        &actualtype, &actualformat, &nbytes, &after, &data);
    state.has_wm_state=(actualtype != None);
    }
  err=XQueryTree(dpy, w, &root, &parent, children, nchildren);
  if(err == 0)
     state.have_error = TRUE;
  *has_wm_state = state.has_wm_state;
  return !state.have_error;
}

gboolean
_gdk_x11_get_window_child_info (GdkDisplay       *display,
				Window            window,
				gboolean          get_wm_state,
				gboolean         *win_has_wm_state,
				GdkChildInfoX11 **children,
				guint            *nchildren)
{
  Display *dpy;
  ChildInfoState state;
  Atom wm_state_atom;
  gboolean has_wm_state;
  Bool result;
  guint i;
  long nbytes, after;
  Atom actualtype;
  int err, actualformat;
  unsigned char *data;
  XWindowAttributes wa;
  Window root;;
  unsigned int bw, depth;
    
  *children = NULL;
  *nchildren = 0;
  
  dpy = GDK_DISPLAY_XDISPLAY (display);
  if (get_wm_state)
    wm_state_atom = gdk_x11_get_xatom_by_name_for_display (display, "WM_STATE");
  else
    wm_state_atom = None;

  gdk_error_trap_push ();
  result = list_children_and_wm_state (dpy, window,
				       win_has_wm_state ? wm_state_atom : None,
				       &has_wm_state,
				       &state.children, &state.nchildren);
  gdk_error_trap_pop ();
  if (!result)
    return FALSE;

  if (has_wm_state)
    {
      if (win_has_wm_state)
	*win_has_wm_state = TRUE;
      return TRUE;
    }
  else
    {
      if (win_has_wm_state)
	*win_has_wm_state = FALSE;
    }

  state.get_wm_state = get_wm_state;
  state.child_info = g_new (GdkChildInfoX11, state.nchildren);
  state.current_child = 0;
  state.n_children_found = 0;
  if (get_wm_state)
    state.current_request = CHILD_INFO_GET_PROPERTY;
  else
    state.current_request = CHILD_INFO_GET_WA;
  state.have_error = FALSE;
  state.child_has_error = FALSE;

  for (i = 0; i < state.nchildren; i++)
    {
    Window window = state.children[i];
      
      state.child_info[i].window=window;
      if (get_wm_state)
	{
        actualtype=None;
        err=XGetWindowProperty(dpy, window, wm_state_atom, 0, 0, False, AnyPropertyType,
           &actualtype, &actualformat, &nbytes, &after, &data);
        state.child_info[i].has_wm_state=(actualtype != None);
	}
      err=XGetWindowAttributes(dpy, window, &wa);
      state.child_info[i].is_mapped=(wa.map_state != IsUnmapped);
      state.child_info[i].window_class=wa.class;
/*      if(err == 0)
         state.child_has_error = TRUE;	 */
      
      err=XGetGeometry(dpy, window, &root, &state.child_info[i].x,
               &state.child_info[i].y, &state.child_info[i].width,
	       &state.child_info[i].height, &bw, &depth);
/*      if(err == 0)
         state.child_has_error = TRUE; */
      
    }


  if (!state.have_error)
    {
      *children = state.child_info;
      *nchildren = state.nchildren;
    }
  else
     g_free (state.child_info);
  Xfree (state.children); /* allocated by XQueryTree */
  return !state.have_error;
}

