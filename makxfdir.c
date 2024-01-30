/* $Id: makxfdir.c 1.1 1994/02/01 14:35:01 ulrich Exp $ */
/*
 * Create xfonts.dir.
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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
  char *outname = NULL;
  FILE *outfile = NULL;
  char *fntname;
  FILE *fntfile;
  char *program;
  int c, i;
  char buf[512];
  char *p;
  
  program = argv[0];

  if (argc <= 1 || strcmp (argv[1],"-?") == 0) {
    fprintf (stderr, "usage: %s [-o file] font1.fnt ...\n", program);
    exit (1);
  }
  if (strcmp (argv[1], "-o") == 0 && argc >= 3) {
    outname = argv[2];
    outfile = fopen (outname, "w+");
    argc -= 2;
    argv += 2;
  }
  if (outfile == NULL)
    outfile = stdout;

/*  setmode (fileno (outfile), O_BINARY); */

  while (argc > 1)
    {
      --argc;
      ++argv;

      fntname = *argv;
      fntfile = fopen (fntname, "rb");
      if (fntfile == NULL) {
	fprintf (stderr, "%s: warning: cannot open \"%s\"\n",
		 program, fntname);
	continue;
      }
      while ((c = fgetc (fntfile)) != EOF)
	{
	  p = buf;
          *p++ = c;
	  if ( c != 'F') continue;
	  if ((*p++ = fgetc(fntfile)) != 'O') continue;
	  if ((*p++ = fgetc(fntfile)) != 'N') continue;
	  if ((*p++ = fgetc(fntfile)) != 'T') continue;
	  if ((fgetc(fntfile)) != '\0') continue;
          *p++ = ' ';
	  for (i = 5; i < sizeof(buf)-1; i++)
	    if ((*p++ = fgetc(fntfile)) == '\0')
	      break;
          p--;
          *p++ = '\n';
	  *p++ = 0;

	  for (i = strlen(fntname) - 1; i >= 0; --i)
	    if (fntname[i] == '/'
		|| fntname[i] == '\\'
		|| fntname[i] == ':')
	      break;
	  
	  fputs (fntname + i + 1, outfile);
	  fputc (':', outfile);
	  fputs (buf, outfile);
	  break;
	}
      if (c == EOF) {
	fprintf (stderr, "%s: warning: FONT not found in \"%s\"\n",
		 program, fntname);
      }
      fclose (fntfile);
    }
  if (outname)	/* do not close stdout */
    fclose (outfile);

  return (0);
}

