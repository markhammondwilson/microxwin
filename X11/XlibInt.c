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
/* $Xorg: XlibInt.c,v 1.8 2001/02/09 02:03:38 xorgcvs Exp $ */
/*

Copyright 1985, 1986, 1987, 1998  The Open Group

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
#include "Xlibint.h"
#include <X11/Xpoll.h>
#include <stdio.h>

#ifdef XTHREADS
#include "locking.h"

/* these pointers get initialized by XInitThreads */
LockInfoPtr _Xglobal_lock = NULL;
void (*_XCreateMutex_fn)() = NULL;
struct _XCVList *(*_XCreateCVL_fn)() = NULL;
void (*_XFreeMutex_fn)() = NULL;
void (*_XLockMutex_fn)() = NULL;
void (*_XUnlockMutex_fn)() = NULL;
xthread_t (*_Xthread_self_fn)() = NULL;

#define XThread_Self()	((*_Xthread_self_fn)())

#define UnlockNextReplyReader(d) if ((d)->lock) \
    (*(d)->lock->pop_reader)((d),&(d)->lock->reply_awaiters,&(d)->lock->reply_awaiters_tail)

#define QueueReplyReaderLock(d) ((d)->lock ? \
    (*(d)->lock->push_reader)(d,&(d)->lock->reply_awaiters_tail) : NULL)
#define QueueEventReaderLock(d) ((d)->lock ? \
    (*(d)->lock->push_reader)(d,&(d)->lock->event_awaiters_tail) : NULL)

#if defined(XTHREADS_WARN) || defined(XTHREADS_FILE_LINE)
#define InternalLockDisplay(d,wskip) if ((d)->lock) \
    (*(d)->lock->internal_lock_display)(d,wskip,__FILE__,__LINE__)
#else
#define InternalLockDisplay(d,wskip) if ((d)->lock) \
    (*(d)->lock->internal_lock_display)(d,wskip)
#endif

#else /* XTHREADS else */

#define UnlockNextReplyReader(d)   
#define UnlockNextEventReader(d)
#define InternalLockDisplay(d,wskip)

#endif /* XTHREADS else */ 

int _XDefaultError(Display *, XErrorEvent *);
int _XDefaultIOError(Display *);

XErrorHandler _XErrorFunction = _XDefaultError;
XIOErrorHandler _XIOErrorFunction = _XDefaultIOError;
_XQEvent *_qfree = NULL;
XErrorHandler _WErrorFunctions[256] = { 0 };	/* XXX depends on kernel */

/*
 * _XEnq - Place event packets on the display's queue.
 * note that no squishing of move events in V11, since there
 * is pointer motion hints....
 */
void _XEnq (dpy, event)
	register Display *dpy;
	register xEvent *event;
{
	register _XQEvent *qelt;

	if ((qelt = dpy->qfree)) {
		/* If dpy->qfree is non-NULL do this, else malloc a new one. */
		dpy->qfree = qelt->next;
	}
	else if ((qelt = 
	    (_XQEvent *) Xmalloc((unsigned)sizeof(_XQEvent))) == NULL) {
		/* Malloc call failed! */
                /* errno=ENOMEM; */
		_XIOError(dpy);
	}
	qelt->next = NULL;
        qelt->event= *(XEvent *)event;
        qelt->qserial_num = dpy->next_event_serial_num++;
        if (dpy->tail)	dpy->tail->next = qelt;
        else 		dpy->head = qelt;
        dpy->tail = qelt;
        dpy->qlen++;
}

/*
 * _XDeq - Remove event packet from the display's queue.
 */
void _XDeq (dpy, prev, qelt)
    register Display *dpy;
    register _XQEvent *prev;	/* element before qelt */
    register _XQEvent *qelt;	/* element to be unlinked */
{
    if (prev) {
	if ((prev->next = qelt->next) == NULL)
	    dpy->tail = prev;
    } else {
	/* no prev, so removing first elt */
	if ((dpy->head = qelt->next) == NULL)
	    dpy->tail = NULL;
    }
    qelt->qserial_num = 0;
    qelt->next = dpy->qfree;
    dpy->qfree = qelt;
    dpy->qlen--;
}

/*
 * EventToWire in separate file in that often not needed.
 */

/*ARGSUSED*/
Bool
_XUnknownWireEvent(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XEvent *re;	/* pointer to where event should be reformatted */
register xEvent *event;	/* wire protocol event */
{
#ifdef notdef
	(void) fprintf(stderr, 
	    "Xlib: unhandled wire event! event number = %d, display = %x\n.",
			event->u.u.type, dpy);
#endif
	return(False);
}

/*ARGSUSED*/
Status
_XUnknownNativeEvent(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XEvent *re;	/* pointer to where event should be reformatted */
register xEvent *event;	/* wire protocol event */
{
#ifdef notdef
	(void) fprintf(stderr, 
 	   "Xlib: unhandled native event! event number = %d, display = %x\n.",
			re->type, dpy);
#endif
	return(0);
}
/*
 * The following routines are internal routines used by Xlib for protocol
 * packet transmission and reception.
 *
 * _XIOError(Display *) will be called if any sort of system call error occurs.
 * This is assumed to be a fatal condition, i.e., XIOError should not return.
 *
 * _XError(Display *, xError *) will be called whenever an X_Error event is
 * received.  This is not assumed to be a fatal condition, i.e., it is
 * acceptable for this procedure to return.  However, XError should NOT
 * perform any operations (directly or indirectly) on the DISPLAY.
 *
 * Routines declared with a return type of 'Status' return 0 on failure,
 * and non 0 on success.  Routines with no declared return type don't 
 * return anything.  Whenever possible routines that create objects return
 * the object they have created.
 */
int
_WXError (Display *dpy, XErrorEvent *error)
{
  if (_WErrorFunctions[ConnectionNumber(dpy)])
    return (*_WErrorFunctions[ConnectionNumber(dpy)]) (dpy, error);

  return (*_XErrorFunction) (dpy, error);
}

int _XDefaultError (Display *dpy, XErrorEvent *event)
{
  fprintf (stderr, "X Error:\n");
  fprintf (stderr, "Resource ID: %08x\n", event->resourceid);
  fprintf (stderr, "Serial:\t%d\n", event->serial);
  if (event->error_code <= BadImplementation)
    fprintf (stderr, "Error code:\t%d\n", event->error_code);
  else
    fprintf (stderr, "Error code:\t%d\n", event->error_code);
  fprintf (stderr, "Request code:\t%d\n", event->request_code);
  fprintf (stderr, "Minor code:\t%d\n", event->minor_code);
  exit (1);
}

/*
 * _XDefaultIOError - Default fatal system error reporting routine.  Called 
 * when an X internal system error is encountered.
 */
_XDefaultIOError (dpy)
	Display *dpy;
{
/*	(void) fprintf (stderr, 
	 "XIO:  fatal IO error %d (%s) on X server \"%s\"\r\n",
			errno, strerror (errno), DisplayString (dpy)); */
	(void) fprintf (stderr, 
	 "      after %lu requests (%lu known processed) with %d events remaining.\r\n",
			NextRequest(dpy) - 1, LastKnownRequestProcessed(dpy),
			QLength(dpy));

/*	if (errno == EPIPE) {
	    (void) fprintf (stderr,
	 "      The connection was probably broken by a server shutdown or KillClient.\r\n");
	} */
	exit(1);
}

/*
 * _XIOError - call user connection error handler and exit
 */
int _XIOError (dpy)
    Display *dpy;
{
    dpy->flags |= XlibDisplayIOError;
    if (_XIOErrorFunction != NULL)
	(*_XIOErrorFunction)(dpy);
    else
	_XDefaultIOError(dpy);
    exit (1);
}
/*
 * _XFlush - Flush the X request buffer.  If the buffer is empty, no
 * action is taken.  This routine correctly handles incremental writes.
 * This routine may have to be reworked if int < long.
 */
int
XFlush (dpy)
	register Display *dpy;
{
    LockDisplay(dpy);
    _XFlush (dpy);
    UnlockDisplay(dpy);
    return 1;
}
#define EVENTBUFSIZE	512
void __XReadEvents(dpy)
register Display *dpy;
{
   int count, i;
   XEvent event[EVENTBUFSIZE];

   __XFlush (dpy);
   while(1)
      {   
      /* this is a blocking read */
      if( (count=__XReadKernel(dpy, event, EVENTBUFSIZE)) <= 0)
         return;
      for(i=0; i < count; ++i)
         {
         event[i].xany.display=dpy;
         dpy->last_request_read=event[i].xany.serial;
         if(event[i].type == X_Error)
            _WXError (dpy, (XErrorEvent *)&event[i]);
         else   /* must be an event packet */
            _XEnq (dpy, (xEvent *)&event[i]);
         }
      if(count < EVENTBUFSIZE)
         return;
      if(!__XPendingEvent(dpy))	
         return;	/* nothing more to read */
      }
}

int
_XEventsQueued (dpy, mode)
    register Display *dpy;
    int mode;
{
#ifdef XTHREADS
    struct _XCVList *cvl = NULL;
    xthread_t self;

#ifdef XTHREADS_DEBUG
    printf("_XEventsQueued called in thread %x\n", XThread_Self());
#endif
#endif /* XTHREADS*/

#ifdef XTHREADS
	/* create our condition variable and append to list,
	 * unless we were called from within XProcessInternalConnection
	 * or XLockDisplay
	 */
	xthread_clear_id(self);
	if (dpy->lock && (xthread_have_id (dpy->lock->conni_thread)
			  || xthread_have_id (dpy->lock->locking_thread)))
	    /* some thread is in XProcessInternalConnection or XLockDisplay
	       so we have to see if we are it */
	    self = XThread_Self();
	if (!xthread_have_id(self)
	    || (!xthread_equal(self, dpy->lock->conni_thread)
		&& !xthread_equal(self, dpy->lock->locking_thread))) {
	    /* In the multi-threaded case, if there is someone else
	       reading events, then there aren't any available, so
	       we just return.  If we waited we would block.
	       */
	    if (dpy->lock && dpy->lock->event_awaiters)
		return dpy->qlen;
	    /* nobody here but us, so lock out any newcomers */
	    cvl = QueueEventReaderLock(dpy);
	}

#endif /* XTHREADS*/
   if(/*!dpy->qlen && */(mode ==  QueuedAfterReading || mode ==  QueuedAfterFlush) )
      {
      if(mode ==  QueuedAfterFlush)
         __XFlush (dpy);
      if (__XPendingEvent(dpy))	/* get number of queued events from kernel */
         __XReadEvents (dpy);
      }
#ifdef XTHREADS
	if (cvl)
#endif
	{
	    UnlockNextEventReader(dpy);
	}
   return(dpy->qlen);
}

/* _XReadEvents - Flush the output queue,
 * then read as many events as possible (but at least 1) and enqueue them
 */
void _XReadEvents(dpy)
	register Display *dpy;
{
#ifdef XTHREADS
	struct _XCVList *cvl = NULL;
	xthread_t self;

#ifdef XTHREADS_DEBUG
	printf("_XReadEvents called in thread %x\n",
	       XThread_Self());
#endif
	/* create our condition variable and append to list,
	 * unless we were called from within XProcessInternalConnection
	 * or XLockDisplay
	 */
	xthread_clear_id(self);
	if (dpy->lock && (xthread_have_id (dpy->lock->conni_thread)
			  || xthread_have_id (dpy->lock->locking_thread)))
	    /* some thread is in XProcessInternalConnection or XLockDisplay
	       so we have to see if we are it */
	    self = XThread_Self();
	if (!xthread_have_id(self)
	    || (!xthread_equal(self, dpy->lock->conni_thread)
		&& !xthread_equal(self, dpy->lock->locking_thread)))
	    cvl = QueueEventReaderLock(dpy);
    /* if it is not our turn to read an event off the wire,
	       wait til we're at head of list */
    while(dpy->lock && cvl && dpy->lock->event_awaiters != cvl)
	ConditionWait(dpy, cvl->cv);
#endif /* XTHREADS */
   __XReadEvents(dpy);
   UnlockNextEventReader(dpy);
   return;
}
#define POLLFD_CACHE_SIZE 5

/* initialize the struct array passed to poll() below */
Bool _XPollfdCacheInit(dpy)		   
    Display *dpy;
{
#ifdef USE_POLL
    struct pollfd *pfp;

    pfp = (struct pollfd *)Xmalloc(POLLFD_CACHE_SIZE * sizeof(struct pollfd));
    if (!pfp)
	return False;
    pfp[0].fd = dpy->fd;
    pfp[0].events = POLLIN;

    dpy->filedes = (XPointer)pfp;
#endif
    return True;
}

void _XPollfdCacheAdd(dpy, fd)
    Display *dpy;
    int fd;
{
#ifdef USE_POLL
    struct pollfd *pfp = (struct pollfd *)dpy->filedes;

    if (dpy->im_fd_length <= POLLFD_CACHE_SIZE) {
	pfp[dpy->im_fd_length].fd = fd;
	pfp[dpy->im_fd_length].events = POLLIN;
    }
#endif
}

/* ARGSUSED */
void _XPollfdCacheDel(dpy, fd)
    Display *dpy;
    int fd;			/* not used */
{
#ifdef USE_POLL
    struct pollfd *pfp = (struct pollfd *)dpy->filedes;
    struct _XConnectionInfo *conni;

    /* just recalculate whole list */
    if (dpy->im_fd_length <= POLLFD_CACHE_SIZE) {
	int loc = 1;
	for (conni = dpy->im_fd_info; conni; conni=conni->next) {
	    pfp[loc].fd = conni->fd;
	    pfp[loc].events = POLLIN;
	    loc++;
	}
    }
#endif
}
/*
 * Support for internal connections, such as an IM might use.
 * By Stephen Gildea, X Consortium, September 1993
 */

/* _XRegisterInternalConnection
 * Each IM (or Xlib extension) that opens a file descriptor that Xlib should
 * include in its select/poll mask must call this function to register the
 * fd with Xlib.  Any XConnectionWatchProc registered by XAddConnectionWatch
 * will also be called.
 *
 * Whenever Xlib detects input available on fd, it will call callback
 * with call_data to process it.  If non-Xlib code calls select/poll
 * and detects input available, it must call XProcessInternalConnection,
 * which will call the associated callback.
 *
 * Non-Xlib code can learn about these additional fds by calling
 * XInternalConnectionNumbers or, more typically, by registering
 * a XConnectionWatchProc with XAddConnectionWatch
 * to be called when fds are registered or unregistered.
 *
 * Returns True if registration succeeded, False if not, typically
 * because could not allocate memory.
 * Assumes Display locked when called.
 */
#if NeedFunctionPrototypes
Status _XRegisterInternalConnection(
    Display* dpy,
    int fd,
    _XInternalConnectionProc callback,
    XPointer call_data
)
#else
Status
_XRegisterInternalConnection(dpy, fd, callback, call_data)
    Display *dpy;
    int fd;
    _XInternalConnectionProc callback;
    XPointer call_data;
#endif
{
    struct _XConnectionInfo *new_conni, **iptr;
    struct _XConnWatchInfo *watchers;
    XPointer *wd;

    new_conni = (struct _XConnectionInfo*)Xmalloc(sizeof(struct _XConnectionInfo));
    if (!new_conni)
	return 0;
    new_conni->watch_data = (XPointer *)Xmalloc(dpy->watcher_count * sizeof(XPointer));
    if (!new_conni->watch_data) {
	Xfree(new_conni);
	return 0;
    }
    new_conni->fd = fd;
    new_conni->read_callback = callback;
    new_conni->call_data = call_data;
    new_conni->next = NULL;
    /* link new structure onto end of list */
    for (iptr = &dpy->im_fd_info; *iptr; iptr = &(*iptr)->next)
	;
    *iptr = new_conni;
    dpy->im_fd_length++;
    _XPollfdCacheAdd(dpy, fd);

    for (watchers=dpy->conn_watchers, wd=new_conni->watch_data;
	 watchers;
	 watchers=watchers->next, wd++) {
	*wd = NULL;		/* for cleanliness */
	(*watchers->fn) (dpy, watchers->client_data, fd, True, wd);
    }

    return 1;
}

/* _XUnregisterInternalConnection
 * Each IM (or Xlib extension) that closes a file descriptor previously
 * registered with _XRegisterInternalConnection must call this function.
 * Any XConnectionWatchProc registered by XAddConnectionWatch
 * will also be called.
 *
 * Assumes Display locked when called.
 */
#if NeedFunctionPrototypes
void _XUnregisterInternalConnection(
    Display* dpy,
    int fd
)
#else
void
_XUnregisterInternalConnection(dpy, fd)
    Display *dpy;
    int fd;
#endif
{
    struct _XConnectionInfo *info_list, **prev;
    struct _XConnWatchInfo *watch;
    XPointer *wd;

    for (prev = &dpy->im_fd_info; info_list = *prev; prev = &info_list->next) {
	if (info_list->fd == fd) {
	    *prev = info_list->next;
	    dpy->im_fd_length--;
	    for (watch=dpy->conn_watchers, wd=info_list->watch_data;
		 watch;
		 watch=watch->next, wd++) {
		(*watch->fn) (dpy, watch->client_data, fd, False, wd);
	    }
	    if (info_list->watch_data)
		Xfree (info_list->watch_data);
	    Xfree (info_list);
	    break;
	}
    }
    _XPollfdCacheDel(dpy, fd);
}

/* XInternalConnectionNumbers
 * Returns an array of fds and an array of corresponding call data.
 * Typically a XConnectionWatchProc registered with XAddConnectionWatch
 * will be used instead of this function to discover
 * additional fds to include in the select/poll mask.
 *
 * The list is allocated with Xmalloc and should be freed by the caller
 * with Xfree;
 */
#if NeedFunctionPrototypes
Status XInternalConnectionNumbers(
    Display *dpy,
    int **fd_return,
    int *count_return
)
#else
Status
XInternalConnectionNumbers(dpy, fd_return, count_return)
    Display *dpy;
    int **fd_return;
    int *count_return;
#endif
{
    int count;
    struct _XConnectionInfo *info_list;
    int *fd_list;

    LockDisplay(dpy);
    count = 0;
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next)
	count++;
    fd_list = (int*) Xmalloc (count * sizeof(int));
    if (!fd_list) {
	UnlockDisplay(dpy);
	return 0;
    }
    count = 0;
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	fd_list[count] = info_list->fd;
	count++;
    }
    UnlockDisplay(dpy);

    *fd_return = fd_list;
    *count_return = count;
    return 1;
}

void _XProcessInternalConnection(dpy, conn_info)
    Display *dpy;
    struct _XConnectionInfo *conn_info;
{
    dpy->flags |= XlibDisplayProcConni;
#ifdef XTHREADS
    if (dpy->lock) {
	/* check cache to avoid call to thread_self */
	if (xthread_have_id(dpy->lock->reading_thread))
	    dpy->lock->conni_thread = dpy->lock->reading_thread;
	else
	    dpy->lock->conni_thread = XThread_Self();
    }
#endif /* XTHREADS */
    UnlockDisplay(dpy);
    (*conn_info->read_callback) (dpy, conn_info->fd, conn_info->call_data);
    LockDisplay(dpy);
#ifdef XTHREADS
    if (dpy->lock)
	xthread_clear_id(dpy->lock->conni_thread);
#endif /* XTHREADS */
    dpy->flags &= ~XlibDisplayProcConni;
}

/* XProcessInternalConnection
 * Call the _XInternalConnectionProc registered by _XRegisterInternalConnection
 * for this fd.
 * The Display is NOT locked during the call.
 */
#if NeedFunctionPrototypes
void XProcessInternalConnection(
    Display* dpy,
    int fd
)
#else
void
XProcessInternalConnection(dpy, fd)
    Display *dpy;
    int fd;
#endif
{
    struct _XConnectionInfo *info_list;

    LockDisplay(dpy);
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	if (info_list->fd == fd) {
	    _XProcessInternalConnection(dpy, info_list);
	    break;
	}
    }
    UnlockDisplay(dpy);
}

/* XAddConnectionWatch
 * Register a callback to be called whenever _XRegisterInternalConnection
 * or _XUnregisterInternalConnection is called.
 * Callbacks are called with the Display locked.
 * If any connections are already registered, the callback is immediately
 * called for each of them.
 */
#if NeedFunctionPrototypes
Status XAddConnectionWatch(
    Display* dpy,
    XConnectionWatchProc callback,
    XPointer client_data
)
#else
Status
XAddConnectionWatch(dpy, callback, client_data)
    Display *dpy;
    XConnectionWatchProc callback;
    XPointer client_data;
#endif
{
    struct _XConnWatchInfo *new_watcher, **wptr;
    struct _XConnectionInfo *info_list;
    XPointer *wd_array;

    LockDisplay(dpy);

    /* allocate new watch data */
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	wd_array = (XPointer *)Xrealloc((char *)info_list->watch_data,
					(dpy->watcher_count + 1) *
					sizeof(XPointer));
	if (!wd_array) {
	    UnlockDisplay(dpy);
	    return 0;
	}
	wd_array[dpy->watcher_count] = NULL;	/* for cleanliness */
    }

    new_watcher = (struct _XConnWatchInfo*)Xmalloc(sizeof(struct _XConnWatchInfo));
    if (!new_watcher) {
	UnlockDisplay(dpy);
	return 0;
    }
    new_watcher->fn = callback;
    new_watcher->client_data = client_data;
    new_watcher->next = NULL;

    /* link new structure onto end of list */
    for (wptr = &dpy->conn_watchers; *wptr; wptr = &(*wptr)->next)
	;
    *wptr = new_watcher;
    dpy->watcher_count++;

    /* call new watcher on all currently registered fds */
    for (info_list=dpy->im_fd_info; info_list; info_list=info_list->next) {
	(*callback) (dpy, client_data, info_list->fd, True,
		     info_list->watch_data + dpy->watcher_count - 1);
    }

    UnlockDisplay(dpy);
    return 1;
}

/* XRemoveConnectionWatch
 * Unregister a callback registered by XAddConnectionWatch.
 * Both callback and client_data must match what was passed to
 * XAddConnectionWatch.
 */ 
#if NeedFunctionPrototypes
void XRemoveConnectionWatch(
    Display* dpy,
    XConnectionWatchProc callback,
    XPointer client_data
)
#else
void
XRemoveConnectionWatch(dpy, callback, client_data)
    Display *dpy;
    XConnectionWatchProc callback;
    XPointer client_data;
#endif
{
    struct _XConnWatchInfo *watch;
    struct _XConnWatchInfo *previous = NULL;
    struct _XConnectionInfo *conni;
    int counter = 0;

    LockDisplay(dpy);
    for (watch=dpy->conn_watchers; watch; watch=watch->next) {
	if (watch->fn == callback  &&  watch->client_data == client_data) {
	    if (previous)
		previous->next = watch->next;
	    else
		dpy->conn_watchers = watch->next;
	    Xfree (watch);
	    dpy->watcher_count--;
	    /* remove our watch_data for each connection */
	    for (conni=dpy->im_fd_info; conni; conni=conni->next) {
		/* don't bother realloc'ing; these arrays are small anyway */
		/* overlapping */
		memmove(conni->watch_data+counter,
			conni->watch_data+counter+1,
			dpy->watcher_count - counter);
	    }
	    break;
	}
	previous = watch;
	counter++;
    }
    UnlockDisplay(dpy);
}

/* end of internal connections support */
#if 0
static int _XPrintDefaultError (dpy, event, fp)
    Display *dpy;
    XErrorEvent *event;
    FILE *fp;
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";
    register _XExtension *ext = (_XExtension *)NULL;
    _XExtension *bext = (_XExtension *)NULL;
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    (void) fprintf(fp, "%s:  %s\n  ", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", 
	mesg, BUFSIZ);
    (void) fprintf(fp, mesg, event->request_code);
    if (event->request_code < 128) {
	sprintf(number, "%d", event->request_code);
	XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    }
    (void) fprintf(fp, " (%s)\n", buffer);
    if ((event->error_code == BadWindow) ||
	       (event->error_code == BadPixmap) ||
	       (event->error_code == BadCursor) ||
	       (event->error_code == BadFont) ||
	       (event->error_code == BadDrawable) ||
	       (event->error_code == BadColor) ||
	       (event->error_code == BadGC) ||
	       (event->error_code == BadIDChoice) ||
	       (event->error_code == BadValue) ||
	       (event->error_code == BadAtom)) {
	if (event->error_code == BadValue)
	    XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
				  mesg, BUFSIZ);
	else if (event->error_code == BadAtom)
	    XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
				  mesg, BUFSIZ);
	else
	    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
				  mesg, BUFSIZ);
	fputs("  ", fp);
	(void) fprintf(fp, mesg, event->resourceid);
	fputs("\n", fp);
    }
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
			  mesg, BUFSIZ);
    fputs("  ", fp);
    (void) fprintf(fp, mesg, event->serial);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
			  mesg, BUFSIZ);
    fputs("\n  ", fp);
    (void) fprintf(fp, mesg, dpy->request);
    fputs("\n", fp);
    if (event->error_code == BadImplementation) return 0;
    return 1;
}
int _XDefaultError(dpy, event)
	Display *dpy;
	XErrorEvent *event;
{
    if (_XPrintDefaultError (dpy, event, stderr) == 0) return 0;
    exit(1);
    /*NOTREACHED*/
}
#endif
/*ARGSUSED*/
Bool _XDefaultWireError(display, he, we)
    Display     *display;
    XErrorEvent *he;
    xError      *we;
{
    return True;
}


/*
 * This routine can be used to (cheaply) get some memory within a single
 * Xlib routine for scratch space.  A single buffer is reused each time
 * if possible.  To be MT safe, you can only call this between a call to
 * GetReq* and a call to Data* or _XSend*, or in a context when the thread
 * is guaranteed to not unlock the display.
 */
char *_XAllocScratch (dpy, nbytes)
	register Display *dpy;
	unsigned long nbytes;
{
	if (nbytes > dpy->scratch_length) {
	    if (dpy->scratch_buffer) Xfree (dpy->scratch_buffer);
	    if (dpy->scratch_buffer = Xmalloc((unsigned) nbytes))
		dpy->scratch_length = nbytes;
	    else dpy->scratch_length = 0;
	}
	return (dpy->scratch_buffer);
}

/*
 * Scratch space allocator you can call any time, multiple times, and be
 * MT safe, but you must hand the buffer back with _XFreeTemp.
 */
char *_XAllocTemp (dpy, nbytes)
    register Display *dpy;
    unsigned long nbytes;
{
    char *buf;

    buf = _XAllocScratch(dpy, nbytes);
    dpy->scratch_buffer = NULL;
    dpy->scratch_length = 0;
    return buf;
}

void _XFreeTemp (dpy, buf, nbytes)
    register Display *dpy;
    char *buf;
    unsigned long nbytes;
{
    if (dpy->scratch_buffer)
	Xfree(dpy->scratch_buffer);
    dpy->scratch_buffer = buf;
    dpy->scratch_length = nbytes;
}

/*
 * Given a visual id, find the visual structure for this id on this display.
 */
Visual *_XVIDtoVisual (dpy, id)
	Display *dpy;
	VisualID id;
{
	register int i, j, k;
	register Screen *sp;
	register Depth *dp;
	register Visual *vp;
	for (i = 0; i < dpy->nscreens; i++) {
		sp = &dpy->screens[i];
		for (j = 0; j < sp->ndepths; j++) {
			dp = &sp->depths[j];
			/* if nvisuals == 0 then visuals will be NULL */
			for (k = 0; k < dp->nvisuals; k++) {
				vp = &dp->visuals[k];
				if (vp->visualid == id) return (vp);
			}
		}
	}
	return (NULL);
}

#if NeedFunctionPrototypes
XFree (void *data)
#else
XFree (data)
	char *data;
#endif
{
	Xfree (data);
	return 1;
}

#ifdef _XNEEDBCOPYFUNC
void _Xbcopy(b1, b2, length)
    register char *b1, *b2;
    register length;
{
    if (b1 < b2) {
	b2 += length;
	b1 += length;
	while (length--)
	    *--b2 = *--b1;
    } else {
	while (length--)
	    *b2++ = *b1++;
    }
}
#endif

/* Make sure this produces the same string as DefineLocal/DefineSelf in xdm.
 * Otherwise, Xau will not be able to find your cookies in the Xauthority file.
 *
 * Note: POSIX says that the ``nodename'' member of utsname does _not_ have
 *       to have sufficient information for interfacing to the network,
 *       and so, you may be better off using gethostname (if it exists).
 */

#if (defined(_POSIX_SOURCE) && !defined(AIXV3)) || defined(hpux) || defined(USG) || defined(SVR4)
#define NEED_UTSNAME
#include <sys/utsname.h>
#endif

/*
 * _XGetHostname - similar to gethostname but allows special processing.
 */
int _XGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
{
    int len;

#ifdef NEED_UTSNAME
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* NEED_UTSNAME */
    return len;
}


/*
 * _XScreenOfWindow - get the Screen of a given window
 */

Screen *_XScreenOfWindow (dpy, w)
    Display *dpy;
    Window w;
{
    register int i;
    Window root;
    int x, y;				/* dummy variables */
    unsigned int width, height, bw, depth;  /* dummy variables */

    if (XGetGeometry (dpy, w, &root, &x, &y, &width, &height,
		      &bw, &depth) == False) {
	return None;
    }
    for (i = 0; i < ScreenCount (dpy); i++) {	/* find root from list */
	if (root == RootWindow (dpy, i)) {
	    return ScreenOfDisplay (dpy, i);
	}
    }
    return NULL;
}

/*
 * _XReply - Wait for a reply packet and copy its contents into the
 * specified rep.  Meanwhile we must handle error and event packets that
 * we may encounter.
 */
Status
_XReply (dpy, rep, extra, discard)
    register Display *dpy;
    register xReply *rep;
    int extra;		/* number of 32-bit words expected after the reply */
    Bool discard;	/* should I discard data following "extra" words? */
{
	return 0;
}
/*
 * _XReadPad - Read bytes from the socket taking into account incomplete
 * reads.  If the number of bytes is not 0 mod 4, read additional pad
 * bytes. This routine may have to be reworked if int < long.
 */
void _XReadPad (dpy, data, size)
    	register Display *dpy;	
	register char *data;
	register long size;
{
	_XIOError(dpy);
	return;
}
/* 
 * _XRead - Read bytes from the socket taking into account incomplete
 * reads.  This routine may have to be reworked if int < long.
 */
int _XRead (dpy, data, size)
	register Display *dpy;
	register char *data;
	register long size;
{
    return 0;
}
/*
 * _XSend - Flush the buffer and send the client data. 32 bit word aligned
 * transmission is used, if size is not 0 mod 4, extra bytes are transmitted.
 * This routine may have to be reworked if int < long;
 */
void
#if NeedFunctionPrototypes
_XSend (
	register Display *dpy,
	_Xconst char *data,
	register long size)
#else
_XSend (dpy, data, size)
	register Display *dpy;
	char *data;
	register long size;
#endif
{
	return;
}
void _XEatData (dpy, n)
    Display *dpy;
    register unsigned long n;
{
	return;
}
/*
 * The hard part about this is that we only get 16 bits from a reply.
 * We have three values that will march along, with the following invariant:
 *	dpy->last_request_read <= rep->sequenceNumber <= dpy->request
 * We have to keep
 *	dpy->request - dpy->last_request_read < 2^16
 * or else we won't know for sure what value to use in events.  We do this
 * by forcing syncs when we get close.
 */

unsigned long
_XSetLastRequestRead(dpy, rep)
    register Display *dpy;
    register xGenericReply *rep;
{
    register unsigned long	newseq, lastseq;

    lastseq = dpy->last_request_read;
    /*
     * KeymapNotify has no sequence number, but is always guaranteed
     * to immediately follow another event, except when generated via
     * SendEvent (hmmm).
     */
    if ((rep->type & 0x7f) == KeymapNotify)
	return(lastseq);

    newseq = (lastseq & ~((unsigned long)0xffff)) | rep->sequenceNumber;

    if (newseq < lastseq) {
	newseq += 0x10000;
	if (newseq > dpy->request) {
	    (void) fprintf (stderr, 
	    "Xlib: sequence lost (0x%lx > 0x%lx) in reply type 0x%x!\n",
			    newseq, dpy->request, 
			    (unsigned int) rep->type);
	    newseq -= 0x10000;
	}
    }

    dpy->last_request_read = newseq;
    return(newseq);
}
char *
_XGetAsyncReply(dpy, replbuf, rep, buf, len, extra, discard)
    register Display *dpy;
    register char *replbuf;	/* data is read into this buffer */
    register xReply *rep;	/* value passed to calling handler */
    char *buf;			/* value passed to calling handler */
    int len;			/* value passed to calling handler */
    int extra;			/* extra words to read, ala _XReply */
    Bool discard;		/* discard after extra?, ala _XReply */
{
	return (char *)0;
}
void
_XGetAsyncData(dpy, data, buf, len, skip, datalen, discardtotal)
    Display *dpy;
    char *data;			/* data is read into this buffer */
    char *buf;			/* value passed to calling handler */
    int len;			/* value passed to calling handler */
    int skip;			/* number of bytes already read in previous
				   _XGetAsyncReply or _XGetAsyncData calls */
    int datalen;		/* size of data buffer in bytes */
    int discardtotal;		/* min. bytes to consume (after skip) */
{
}
void _XDeqAsyncHandler(dpy, handler)
    Display *dpy;
    register _XAsyncHandler *handler;
{
}
/*
 * _XAllocID - resource ID allocation routine.
 */
XID _XAllocID(dpy)
    register Display *dpy;
{
   XID id;

   id = dpy->resource_id << dpy->resource_shift;
   if (id < 0x20000000) {
       dpy->resource_id++;
       return (dpy->resource_base + id);
   }
   (void) fprintf(stderr,  "Xlib: resource ID allocation space exhausted!\n");
   dpy->resource_id = 1;
   id = dpy->resource_id << dpy->resource_shift;
   return (dpy->resource_base + id);
}
