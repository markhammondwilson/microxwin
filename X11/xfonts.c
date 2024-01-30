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
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define NCOMPS	14
/*#define FONTDEBUG 1 */
typedef struct 
   {
   char *components[NCOMPS+1]; /* the NCOMPS parts of a XLFD spec + NULL */
   char file_name[20];
   char *logical_name;
   Atom xafont;
   Atom xafamily;
   Atom wtname;
   int index; /* index into FontList array */
   } _WFontName;
_WFontName *_FontList = NULL;
int _FontListSize = 0;
static Atom wtname; /* Atom for string WEIGHT_NAME */
static void fnametocomponents(const char *p, _WFontName *fname);
static void freecomponents(_WFontName *source);
static int matchcomponents(_WFontName *source, _WFontName *target);
static int catstr(char *dest, int count, char *str1, char *str2)
{
   int i;
   i=0;
   /* copy string 1 */
   while(*str1 && i < count)
      {
      *dest++ = *str1++;
      i++;
      }
   if(i >= (count-1))
      return -1;	/* out of room */
   *dest++ = '/';
   i++;
   /* copy string 2 */
   while(*str2 && i < count)
      {
      *dest++ = *str2++;
      i++;
      }
   if(i >= (count-1))
      return -1;	/* out of room */
   *dest++ = '\0';
   return 0;
}
/*
 * Free all the components of a font name.
 */
static void freecomponents(_WFontName *source)
{
   char *s;
   int sindex;

   sindex=0;
   while(sindex < NCOMPS)
      {
      if(s=source->components[sindex++])
         free(s);
      }
   return;
}
/*
 * Match components of source with target, source can have '*' or '?'
 */
static int matchcomponents(_WFontName *source, _WFontName *target)
{
   char *s, *t;
   int index;

   index=0;
   while(index < NCOMPS)
      {
      s=source->components[index];
      t=target->components[index];
      if( *s == '*' || *t == '*')
         {
         index++;
	 continue;
	 }
      if(strcmp(s,t))
	    return(0);
      index++;
      }	 
   return(1);
}
/*
 * convert from font name to individual components.
 */
static void fnametocomponents(const char *p, _WFontName *fname)
{
   char buffer[200], *s;
   int index=0, startup=1;

   while(index < NCOMPS)
      {
      /* get rid of leading '*' or '-' */
      while(1)
         {
         if(!(*p))
            goto fill;
         if(*p == '-')
	    {
            p++;
            startup=0;
            break;  
            }
         else if(*p == '*' && startup)	/* can start with *courier */
	    {
            fname->components[index++] = strdup("*");
            p++;
            startup=0;
            continue;  
            }
         else
            break;
         }	 
      /* copy the string until '-' into buffer */
      startup=0;
      s=buffer;
      while(1)
         {
         if(!(*p))
            break;
         if(*p == '-')
	    break;
         *s++ = tolower(*p++);
         }
      *s++='\0';
      if(buffer[0] == '\0')
         fname->components[index++] = strdup("*");
      else
         fname->components[index++] = strdup(buffer);
      }      
fill:
   while(index < NCOMPS)
      fname->components[index++] = strdup("*");
   return;
} 
/*
 * Read the font names from xfonts.dir and initilaize.
 */
void  _WInitFontList(Display* dpy)
{
   FILE *fp;
   int n, list_size, i;
   _WFontName *font_list;
   char dir_name[120];
   char buffer[200];
   char *p, c, *s;

   if (_FontList) return;

   /* scan xfonts.dir produced with \'grep "FONT.*$" *.fnt\' */

   catstr(dir_name, 120, "/usr/fonts","xfonts.dir");
   fp = fopen (dir_name, "rt");
   if (fp == NULL)
     return;
  
   wtname=XInternAtom(dpy, "WEIGHT_NAME", False);
   list_size = 32;
   font_list = (_WFontName *)Xmalloc(list_size * sizeof(_WFontName));
   n = 0;

   while(fgets (buffer, 200, fp))
      {
      if (n >= list_size)
         {
         list_size += list_size/2 + 1;
         font_list = (_WFontName *)
	                 Xrealloc (font_list, list_size * sizeof(_WFontName));
         }
      p = strchr (buffer, '\n');
      if (p) *p = '\0';
      p = strchr (buffer, ':');
      if (! p) 
         continue;
      *p = 0;
      strcpy (font_list[n].file_name, buffer);
      p++;
      while( *p != '-')
         p++;		/* skip ":FONT" */
      s=p;
      while( *s)
         {
         *s=tolower(*s); /* convert to lower case */
         s++;
         }
      font_list[n].logical_name=strdup(p);
      fnametocomponents(p, &font_list[n]); 
      /* get a atom for the font and fammily name */
      font_list[n].xafont=XInternAtom(dpy,font_list[n].logical_name, False);
      c= *(font_list[n].components[1]);
      font_list[n].xafamily=None;
      if(c != '\0' && c != '*')
         font_list[n].xafamily=XInternAtom(dpy,font_list[n].components[1], False);
      c= *(font_list[n].components[2]);
      font_list[n].wtname=None;
      if(c != '\0' && c != '*')
         font_list[n].wtname=XInternAtom(dpy,font_list[n].components[2], False);
      font_list[n].index=n;
      n++;
      }
   fclose (fp);
   _FontList = font_list;
   _FontListSize = n;
   return;
}

_WFontName **_WListFonts (const char *pattern, int max_count, int *count)
{
   _WFontName fname, **flist;
   
   int i, j;
   *count=0;
   if(!pattern) 
      return(NULL);
   /* limit list of names to  _FontListSize */
   max_count= (max_count > _FontListSize) ? _FontListSize : max_count; 
   flist=(_WFontName **)Xmalloc((max_count+1)*sizeof(_WFontName *));
   if(!flist)
      return(NULL);
   /* get all the components for the name */
   fnametocomponents(pattern, &fname); 
   j=0;
   for(i=0; i < _FontListSize; ++i)
      {
      if(matchcomponents(&fname, &_FontList[i]))
         {
	 flist[j++]=&_FontList[i];	/* save the matched font */
	 if(j == max_count)
            break;
	 }
      }
   freecomponents(&fname);
   if(!j)
      {
      Xfree(flist);
      *count=0;
      return(NULL);
      }
   flist[j]=NULL;
   *count=j;
   return(flist);
}
void
_WFreeFontNames (char **name_table)
{
  char **p = name_table;

  if (! p) return;
  while (*p) { free (*p); p++; }
  free (name_table);
}

Font
XLoadFont(
    Display*		display,
    _Xconst char*	name)
{
   Font font=None;
   _WFontName **flist;
   int width, height, count;
   char pattern[200];
   char file_name[100];
  
   LockDisplay(display);
   if (_FontListSize == 0)
      _WInitFontList (display);
   if (isdigit (name[0])
         && sscanf ((char *) name, "%dx%d", &width, &height) == 2)
      {
      sprintf (pattern, "-*-fixed-medium-r-*--%d-*-*-*-c-%d-*-1", height, 10 * width);
      name = pattern;
      }
   else if (! strcmp (name, "fixed"))
      {
      width = 7;
      height = 13;
      sprintf (pattern, "-*-fixed-medium-r-normal--%d-*-*-*-c-%d-*-1", height, 10 * width);
      name = pattern;
      }
   else if (! strcmp (name, "nil2"))
      {
      catstr(file_name, 100, "/usr/fonts","nil2.snf");
      font = __XLoadFont(display, file_name, _FontListSize);
      UnlockDisplay(display);
      return font;
      }
#if 0  /* kde 3.2.1 crashes, actually  in _XcursorCreateGlyphCursor() */
   else if (! strcmp (name, "cursor")) /* tcl/TK needs CURSORFONT */
      {
      catstr(file_name, 100, "/usr/fonts","cursor.snf");
      font = __XLoadFont(display, file_name, _FontListSize);
      UnlockDisplay(display);
      return font;
      }
#endif
   flist = _WListFonts (name, 1, &count);
#ifdef FONTDEBUG
   printf("XLoadFont: %s, %s\n", name, ((flist) ? flist[0]->file_name : "NOFONT")); 
#endif
   if(flist)
      {
      catstr(file_name, 100, "/usr/fonts", flist[0]->file_name);
      font = __XLoadFont(display, file_name, flist[0]->index);
      UnlockDisplay(display);
      Xfree(flist);
      if (font)
	return font;
      }
    else 
      UnlockDisplay(display);
    return 0;
}


XFontStruct *
XLoadQueryFont(
    Display*		display,
    _Xconst char*	name)
{
  Font font;

#ifdef FONTDEBUG
  printf("XLoadQueryFont %s \n",  name); 
#endif
  font = XLoadFont (display, name);
  if (font == None)
    return NULL;

  return XQueryFont (display, font);
}

XFontStruct *
XQueryFont(
    Display*		display,
    Font		font)
{
  XFontStruct *fs;
  int ncharinfo, index;
  _WFontName *fname;
  
  if (font == None)
    return NULL;

  fs = (XFontStruct *) Xmalloc (sizeof(XFontStruct));
  if (fs == NULL) 
    return NULL;
  LockDisplay(display);
  ncharinfo=0;
  /* get the size of ncharinfo & index */
  if(!__XQueryFont(display, font, fs, &index, &ncharinfo))
     {
     UnlockDisplay(display);
     Xfree(fs);
     return NULL;
     }
  if(ncharinfo)
     fs->per_char=Xmalloc(ncharinfo*sizeof(XCharStruct));
  else
     fs->per_char=NULL;
  fs->properties=NULL;
  fs->n_properties=0;
  if(index !=  _FontListSize)
     {
     fs->properties=Xmalloc(3*sizeof(XFontProp));
     fs->n_properties=3;
     fname=&_FontList[index];/* gives access to atoms */
     fs->properties[0].name=wtname;
     fs->properties[0].card32=fname->wtname;
     fs->properties[1].name=XA_FONT;
     fs->properties[1].card32=fname->xafont;
     fs->properties[2].name=XA_FAMILY_NAME;
     fs->properties[2].card32=fname->xafamily;
     }      
  __XQueryFont(display, font, fs, &index, &ncharinfo);
  UnlockDisplay(display);
  return fs;
}

#if 0
Bool XGetFontProperty (fs, name, valuePtr)
XFontStruct *fs;
register Atom name;
unsigned long *valuePtr;
{
   _WFontName *fname;
   
   if(!fs)
      return 0;
   /* cursor or nil2 fonts */
   if(fs->n_properties == _FontListSize)
      {
      *valuePtr=0;
      return 0;
      }
   fname=&_FontList[fs->n_properties]; /* gives access to atoms */
   if(name == wtname)
      {
      *valuePtr=fname->wtname;
      return 1;
      }
   switch (name)
      {
      case XA_FONT:
         *valuePtr=fname->xafont;
 	 return 1;
      case XA_FAMILY_NAME:
         *valuePtr=fname->xafamily;
	 return 1;
      default:
         *valuePtr=0;
	 return 0;
      }      	 
}
#endif
int
XFreeFont(
    Display*		display,
    XFontStruct*	fs)
{
  LockDisplay(display);
  XUnloadFont(display, fs->fid);
  if(fs->properties)
    Xfree(fs->properties);
  if(fs->per_char)
    Xfree(fs->per_char);
  Xfree(fs);
  UnlockDisplay(display);
  return 0;
}

int
XFreeFontInfo(
    char**		names,
    XFontStruct*	free_info,
    int			actual_count)
{
  int i;

  _WFreeFontNames (names);
  for (i = 0; i < actual_count; i++) {
      if(free_info[i].properties);
         Xfree(free_info[i].properties);
      if(free_info[i].per_char);
         Xfree(free_info[i].per_char);
    }
  Xfree(free_info);
  return 0;
}

char **XListFonts(
    Display*		display,
    _Xconst char*	pattern,
    int			maxnames,
    int*		actual_count_return)
{
   _WFontName **flist;
   char **list=NULL;
   int count, i;

#ifdef FONTDEBUG
  printf("XListFonts pattern %s\n",  pattern); 
#endif
   LockDisplay(display);
   if (_FontListSize == 0)
      _WInitFontList (display);
   flist = _WListFonts (pattern, maxnames, &count);
   if(flist)
      {
      list=(char **)Xmalloc((count+1)*sizeof(char *));     
      for(i=0; i < count; ++i)
         {
         list[i]=strdup(flist[i]->logical_name);
#ifdef FONTDEBUG
         printf("%s\n", flist[i]->file_name);
#endif
         }
      list[count]=NULL;
      Xfree(flist);
      *actual_count_return = count;
      UnlockDisplay(display);
      return list;
      }
   UnlockDisplay(display);
   *actual_count_return = 0;
   return NULL;
}


char **XListFontsWithInfo(
    Display*		display,
    _Xconst char*	pattern,
    int			maxnames,
    int*		count_return,
    XFontStruct**	info_return)
{
  int i, count;
  XFontStruct *font_array, *fs;
  char **name_array;

  LockDisplay(display);
  name_array = XListFonts (display, pattern, maxnames, count_return);
  count = *count_return;
  if (count > 0) {

    font_array = (XFontStruct *) Xmalloc (count * sizeof(XFontStruct));
    for (i = 0; i < count; i++) {
      fs = XLoadQueryFont (display, name_array[i]);
      font_array[i] = *fs;
      XUnloadFont(display, fs->fid);
      Xfree(fs);	/* keep properties and charinfo arrays */
    }
    *info_return = font_array;
  }
  UnlockDisplay(display);
  return name_array;
}

int XFreeFontNames(
    char**		list)
{
  _WFreeFontNames (list);
}
int
XDrawString(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    _Xconst char*	string,
    int			length)
{
   int code;
   LockDisplay(display);
   code=__XDrawString(display, d, gc, x, y, string, length, X_PolyText8);
   UnlockDisplay(display);
   return(code);
}
int
XDrawString16(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    _Xconst XChar2b	*string,
    int			length)
{
   int code;
   LockDisplay(display);
   code=__XDrawString(display, d, gc, x, y, string, length, X_PolyText16);
   UnlockDisplay(display);
   return(code);
}

int
XDrawImageString(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    _Xconst char*	string,
    int			length)
{
   int code;
   LockDisplay(display);
   code=__XDrawString(display, d, gc, x, y, string, length, X_ImageText8);
   UnlockDisplay(display);
   return(code);
}
int
XDrawImageString16(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    _Xconst XChar2b	*string,
    int			length)
{
   int code;
   LockDisplay(display);
   code=__XDrawString(display, d, gc, x, y, string, length, X_ImageText16);
   UnlockDisplay(display);
   return(code);
}

int
XDrawText(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    XTextItem*		items,
    int			nitems)
{
  int i;
  XFontStruct *fs;

  LockDisplay(display);
  fs = XQueryFont (display, (XID) gc->values.font);

  for (i = 0; i < nitems; i++) {

    if (items->nchars > 0 && items->chars) {
      if (items->font != None) {
	XSetFont (display, gc, items->font);
	fs = XQueryFont (display, (XID) gc->values.font);
      }
      x += items->delta;
      XDrawString (display, d, gc, x, y,
		   items->chars, items->nchars);
      x += XTextWidth (fs, items->chars, items->nchars);
    }

    items++;
  }
  UnlockDisplay(display);
  return 1;
}
int
XDrawText16(
    Display*		display,
    Drawable		d,
    GC			gc,
    int			x,
    int			y,
    XTextItem16*	items,
    int			nitems)
{
  int i;
  XFontStruct *fs;

  LockDisplay(display);
  fs = XQueryFont (display, (XID) gc->values.font);

  for (i = 0; i < nitems; i++) {

    if (items->nchars > 0 && items->chars) {
      if (items->font != None) {
	XSetFont (display, gc, items->font);
	fs = XQueryFont (display, (XID) gc->values.font);
      }
      x += items->delta;
      XDrawString16 (display, d, gc, x, y,
		   items->chars, items->nchars);
      x += XTextWidth16 (fs, items->chars, items->nchars);
    }

    items++;
  }
  UnlockDisplay(display);
  return 1;
}

char **XGetFontPath(dpy, npaths)
register Display *dpy;
int *npaths;	/* RETURN */
{
   *npaths=0;
   return(NULL);
}
XFreeFontPath (list)
char **list;
{
   return 1;
}
XSetFontPath (dpy, directories, ndirs)
register Display *dpy;
char **directories;
int ndirs;
{
   return 1;
}

