#include <X11/Xlibint.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/dpmsstr.h>

Bool
DPMSQueryExtension (Display *dpy, int *event_basep, int *error_basep)
{
   *event_basep = 0;
   *error_basep = 0;
   return False;
}
Bool
DPMSCapable(Display *dpy)
{
    return False;
}
