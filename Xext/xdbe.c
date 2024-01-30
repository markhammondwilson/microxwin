#include <stdio.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xdbe.h>

#if NeedFunctionPrototypes
Status XdbeQueryExtension (
    Display *dpy,
    int *major_version_return,
    int *minor_version_return)
#else
Status XdbeQueryExtension (dpy, major_version_return, minor_version_return)
    Display *dpy;
    int *major_version_return;
    int *minor_version_return;
#endif
{
    return (Status)0; /* failure */
}
#if NeedFunctionPrototypes
XdbeBackBuffer XdbeAllocateBackBufferName(
    Display *dpy,
    Window window,
    XdbeSwapAction swap_action)
#else
XdbeBackBuffer XdbeAllocateBackBufferName (dpy, window, swap_action)
    Display *dpy;
    Window window;
    XdbeSwapAction swap_action;
#endif
{
   return (XdbeBackBuffer)0;
}
#if NeedFunctionPrototypes
Status XdbeDeallocateBackBufferName (
    Display *dpy,
    XdbeBackBuffer buffer)
#else
Status XdbeDeallocateBackBufferName (dpy, buffer)
    Display *dpy;
    XdbeBackBuffer buffer;
#endif
{
   return (Status) 0;
}
#if NeedFunctionPrototypes
Status XdbeSwapBuffers (
    Display *dpy,
    XdbeSwapInfo *swap_info,
    int num_windows)
#else
Status XdbeSwapBuffers (dpy, swap_info, num_windows)
    Display *dpy;
    XdbeSwapInfo *swap_info;
    int num_windows;
#endif
{
   return (Status) 0;
}
#if NeedFunctionPrototypes
Status XdbeBeginIdiom (
    Display *dpy)
#else
Status XdbeBeginIdiom (dpy)
    Display *dpy;
#endif
{
   return (Status) 0;
}
#if NeedFunctionPrototypes
Status XdbeEndIdiom (
    Display *dpy)
#else
Status XdbeEndIdiom (dpy)
    Display *dpy;
#endif
{
   return (Status) 0;
}
#if NeedFunctionPrototypes
XdbeScreenVisualInfo *XdbeGetVisualInfo (
    Display        *dpy,
    Drawable       *screen_specifiers,
    int            *num_screens)  /* SEND and RETURN */
#else
XdbeScreenVisualInfo *XdbeGetVisualInfo (dpy, screen_specifiers, num_screens)
    Display        *dpy;
    Drawable       *screen_specifiers;
    int            *num_screens;  /* SEND and RETURN */
#endif
{
    return (XdbeScreenVisualInfo *)NULL;
}
#if NeedFunctionPrototypes
void XdbeFreeVisualInfo(
    XdbeScreenVisualInfo *visual_info)
#else
void XdbeFreeVisualInfo(visual_info)
    XdbeScreenVisualInfo *visual_info;
#endif
{
    if (visual_info == NULL) {
        return;
    }

    if (visual_info->visinfo) {
        XFree(visual_info->visinfo);
    }

    XFree(visual_info);
}
#if NeedFunctionPrototypes
XdbeBackBufferAttributes *XdbeGetBackBufferAttributes(
    Display *dpy,
    XdbeBackBuffer buffer)
#else
XdbeBackBufferAttributes *XdbeGetBackBufferAttributes(dpy, buffer)
    Display *dpy;
    XdbeBackBuffer buffer;
#endif
{
   return (XdbeBackBufferAttributes *)NULL;
}


