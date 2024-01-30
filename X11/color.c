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
/*
 * Xlibemu
 *
 * Copyright 1993, 1994
 * Ulrich Leodolter, Austria 3051 St.Christophen, Ludmerfeld 33.
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
static const char	_XcmsRGBi_prefix[] = "rgbi";
static const char	_XcmsRGB_prefix[] = "rgb";

static struct _ColorEntry {
  unsigned int r:8;
  unsigned int g:8;
  unsigned int b:8;
  char *name;
} _ColorTable[] = {
# include "rgb.h"
};

static int _ColorTableSorted = 0;

static int CompareColorEntry (const void *c1, const void *c2)
{
  return strcasecmp (((struct _ColorEntry *) c1)->name,
		     ((struct _ColorEntry *) c2)->name);
}

static void SortColorTable ()
{
  if (_ColorTableSorted == 0) {
    qsort (_ColorTable,
	   sizeof(_ColorTable) / sizeof(_ColorTable[0]),
	   sizeof(_ColorTable[0]),
	   CompareColorEntry);
    _ColorTableSorted = 1;
  }
}


/*
 *	NAME
 *		XcmsLRGB_RGB_ParseString
 *
 *	SYNOPSIS
 *              Modified from X11/LRGB.c
 */
static int
XcmsLRGB_RGB_ParseString(spec, pColor)
    register char *spec;
    XColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XcmsRGBFormat.
 *
 *	RETURNS
 *		0 if failed, non-zero otherwise.
 */
{
    register int n, i;
    unsigned short r, g, b;
    char c;
    char *pchar;
    unsigned short *pShort;

    /*
     * Check for old # format
     */
    if (*spec == '#') {
	/*
	 * Attempt to parse the value portion.
	 */
	spec++;
	n = strlen(spec);
	if (n != 3 && n != 6 && n != 9 && n != 12) {
	    return(0);
	}

	n /= 3;
	g = b = 0;
	do {
	    r = g;
	    g = b;
	    b = 0;
	    for (i = n; --i >= 0; ) {
		c = *spec++;
		b <<= 4;
		if (c >= '0' && c <= '9')
		    b |= c - '0';
		else if (c >= 'A' && c <= 'F')
		    b |= c - ('A' - 10);
		else if (c >= 'a' && c <= 'f')
		    b |= c - ('a' - 10);
		else return (0);
	    }
	} while (*spec != '\0');

	/*
	 * Succeeded !
	 */
	n <<= 2;
	n = 16 - n;
	/* shift instead of scale, to match old broken semantics */
	pColor->red = r << n;
	pColor->green = g << n;
	pColor->blue =  b << n;
    } else {
	if ((pchar = strchr(spec, ':')) == NULL) {
	    return(0);
	}
	n = (int)(pchar - spec);

	/*
	 * Check for proper prefix.
	 */
	if (strncmp(spec, _XcmsRGB_prefix, n) != 0) {
	    return(0);
	}

	/*
	 * Attempt to parse the value portion.
	 */
	spec += (n + 1);
	pShort = &pColor->red;
	for (i = 0; i < 3; i++, pShort++, spec++) {
	    n = 0;
	    *pShort = 0;
	    while (*spec != '/' && *spec != '\0') {
		if (++n > 4) {
		    return(0);
		}
		c = *spec++;
		*pShort <<= 4;
		if (c >= '0' && c <= '9')
		    *pShort |= c - '0';
		else if (c >= 'A' && c <= 'F')
		    *pShort |= c - ('A' - 10);
		else if (c >= 'a' && c <= 'f')
		    *pShort |= c - ('a' - 10);
		else return (0);
	    }
	    if (n == 0)
		return (0);
	    if (n < 4) {
		*pShort = ((unsigned long)*pShort * 0xFFFF) / ((1 << n*4) - 1); 
	    } 
	}
    }
    return (1);
}
static int
_WParseColor (char *name, XColor *color)
{
  struct _ColorEntry *entry, key;

  if (!name || !color)
    return 0; /* not found */;

  if(XcmsLRGB_RGB_ParseString(name, color))
     return(1); /* found a match */

  SortColorTable ();
  key.name = name;
  entry = bsearch (&key, _ColorTable,
		   sizeof(_ColorTable) / sizeof(_ColorTable[0]),
		   sizeof(_ColorTable[0]),
		   CompareColorEntry);
  if (entry == NULL)
    return 0;

  color->red   = entry->r << 8;
  color->green = entry->g << 8;
  color->blue  = entry->b << 8;
  return 1;
}


Status XParseColor(
    Display*		display,
    Colormap		colormap,
    _Xconst char*	spec,
    XColor*		col)
{
  LockDisplay(display);
  if (_WParseColor ((char *) spec, col)) {
    col->flags = DoRed|DoGreen|DoBlue;
    UnlockDisplay(display);
    return 1;
  }
  UnlockDisplay(display);
  printf("Cannot find XParseColor %s\n", spec);
  return 0;
}

int
XQueryColor(
    Display*		display,
    Colormap		colormap,
    XColor*		def_in_out)
{
  return XQueryColors (display, colormap, def_in_out, 1);
}
Status XAllocNamedColor(
    Display*		display,
    Colormap		colormap,
    _Xconst char*	color_name,
    XColor*		screen_def_return,
    XColor*		exact_def_return)
{
  if (XParseColor (display, colormap, color_name, exact_def_return))
    {
      int status;
      *screen_def_return = *exact_def_return;
      status = XAllocColor (display, colormap, screen_def_return);
      exact_def_return->pixel = screen_def_return->pixel;
      return status;
    }
  return 0;
}

Status XLookupColor(
    Display*		display,
    Colormap		colormap,
    _Xconst char*	color_name,
    XColor*		exact_def_return,
    XColor*		screen_def_return)
{
  if (XParseColor (display, colormap, color_name, exact_def_return))
    {
    *screen_def_return = *exact_def_return;
    screen_def_return->red   &= 0xff00;
    screen_def_return->green &= 0xff00;
    screen_def_return->blue  &= 0xff00;	/* only 8 MSBs on hardware */
    return 1;
    }
  return 0;
}
XStoreColor(dpy, cmap, def)
register Display *dpy;
Colormap cmap;
XColor *def;
{
   return (XStoreColors(dpy, cmap, def, 1));
}
Colormap *XListInstalledColormaps(dpy, win, n)
register Display *dpy;
Window win;
int *n;  /* RETURN */
{
   *n=0;
   return(NULL);	/* no colormap for true color */
}
