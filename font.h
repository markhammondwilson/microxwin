/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
 * Machine dependent values:
 * GLYPHPADBYTES should be chosen with consideration for the space-time
 * trade-off.  Padding to 0 bytes means that there is no wasted space
 * in the font bitmaps (both on disk and in memory), but that access of
 * the bitmaps will cause odd-address memory references.  Padding to
 * 2 bytes would ensure even address memory references and would
 * be suitable for a 68010-class machine, but at the expense of wasted
 * space in the font bitmaps.  Padding to 4 bytes would be good
 * for real 32 bit machines, etc.  Be sure that you tell the font
 * compiler what kind of padding you want because its defines are
 * kept separate from this.  See server/include/fonts.h for how
 * GLYPHPADBYTES is used.
 *
 * Along with this, you should choose an appropriate value for
 * GETLEFTBITS_ALIGNMENT, which is used in ddx/mfb/maskbits.h.  This
 * constant choses what kind of memory references are guarenteed during
 * font access; either 1, 2 or 4, for byte, word or longword access,
 * respectively.  For instance, if you have decided to to have
 * GLYPHPADBYTES == 4, then it is pointless for you to have a
 * GETLEFTBITS_ALIGNMENT > 1, because the padding of the fonts has already
 * guarenteed you that your fonts are longword aligned.  On the other
 * hand, even if you have chosen GLYPHPADBYTES == 1 to save space, you may
 * also decide that the computing involved in aligning the pointer is more
 * costly than an odd-address access; you choose GETLEFTBITS_ALIGNMENT == 1.
 * 
 * XXX: this code has changed since beta test and only GLYPHPADBYTES == 4
 * has been tested, hence all machines have this same value.
 *
 */
#define	GLYPHPADBYTES		1
#define GETLEFTBITS_ALIGNMENT	1
#define MAXSHORT 32767
#define MINSHORT -MAXSHORT 
#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	FALSE
#define	FALSE	0
#endif
#define NullCharInfo ((CharInfoPtr)0)
#define NullFontInfo ((FontInfoPtr)0)
#define LeftToRight 0
#define RightToLeft 1
/*
 * for linear char sets
 */
#define n1dChars(pfi) ((pfi)->lastCol - (pfi)->firstCol + 1)
#define chFirst firstCol	/* usage:  pfi->chFirst */
#define chLast lastCol		/* usage:  pfi->chLast */

/*
 * for 2D char sets
 */
#define n2dChars(pfi)	(((pfi)->lastCol - (pfi)->firstCol + 1) * \
			 ((pfi)->lastRow - (pfi)->firstRow + 1))

#define ADDRXTHISCHARINFO( pf, ch ) \
        ((CharInfoRec *) &((pf)->pCI[(ch) - (pf)->pFI->chFirst]))

#define	GLWIDTHPIXELS(pci) \
	((pci)->metrics.rightSideBearing - (pci)->metrics.leftSideBearing)
#define	GLHEIGHTPIXELS(pci) \
 	((pci)->metrics.ascent + (pci)->metrics.descent)


#define	GLYPHWIDTHBYTES(pci)	(((GLYPHWIDTHPIXELS(pci))+7) >> 3)
#define	GLYPHHEIGHTPIXELS(pci)	(pci->metrics.ascent + pci->metrics.descent)
#define	GLYPHWIDTHPIXELS(pci)	(pci->metrics.rightSideBearing \
				    - pci->metrics.leftSideBearing)
#define GLWIDTHPADDED( bc)	((bc+7) & ~0x7)

#if GLYPHPADBYTES == 0 || GLYPHPADBYTES == 1
#define	GLYPHWIDTHBYTESPADDED(pci)	(GLYPHWIDTHBYTES(pci))
#define	PADGLYPHWIDTHBYTES(w)		(((w)+7)>>3)
#endif

#if GLYPHPADBYTES == 2
#define	GLYPHWIDTHBYTESPADDED(pci)	((GLYPHWIDTHBYTES(pci)+1) & ~0x1)
#define	PADGLYPHWIDTHBYTES(w)		(((((w)+7)>>3)+1) & ~0x1)
#endif

#if GLYPHPADBYTES == 4
#define	GLYPHWIDTHBYTESPADDED(pci)	((GLYPHWIDTHBYTES(pci)+3) & ~0x3)
#define	PADGLYPHWIDTHBYTES(w)		(((((w)+7)>>3)+3) & ~0x3)
#endif

#if GLYPHPADBYTES == 8 /* for a cray? */
#define	GLYPHWIDTHBYTESPADDED(pci)	((GLYPHWIDTHBYTES(pci)+7) & ~0x7)
#define	PADGLYPHWIDTHBYTES(w)		(((((w)+7)>>3)+7) & ~0x7)
#endif

typedef struct _FontProp *FontPropPtr;
typedef struct _CharInfo *CharInfoPtr;
typedef struct _FontInfo *FontInfoPtr;
typedef unsigned int DrawDirection;
typedef struct _ExtentInfo *ExtentInfoPtr;
typedef int Bool;

typedef struct _CharInfo {
    xCharInfo	metrics;	/* info preformatted for Queries */
    unsigned	byteOffset:24;	/* byte offset of the raster from pGlyphs */
    Bool	exists:1;	/* true iff glyph exists for this char */
    unsigned	pad:7;		/* must be zero for now */
} CharInfoRec;

typedef struct _FontInfo {
    unsigned int	version1;   /* version stamp */
    unsigned int	allExist;
    unsigned int	drawDirection;
    unsigned int	noOverlap;	/* true if:
					 * max(rightSideBearing-characterWidth)
					 * <= minbounds->metrics.leftSideBearing
					 */
    unsigned int	constantMetrics;
    unsigned int	terminalFont;	/* Should be deprecated!  true if:
					   constant metrics &&
					   leftSideBearing == 0 &&
					   rightSideBearing == characterWidth &&
					   ascent == fontAscent &&
					   descent == fontDescent
					*/
    unsigned int	linear:1;	/* true if firstRow == lastRow */
    unsigned int	constantWidth:1;  /* true if minbounds->metrics.characterWidth
					   *      == maxbounds->metrics.characterWidth
					   */
    unsigned int	inkInside:1;    /* true if for all defined glyphs:
					 * leftSideBearing >= 0 &&
					 * rightSideBearing <= characterWidth &&
					 * -fontDescent <= ascent <= fontAscent &&
					 * -fontAscent <= descent <= fontDescent
					 */
    unsigned int	inkMetrics:1;	/* ink metrics != bitmap metrics */
					/* used with terminalFont */
					/* see font's pInk{CI,Min,Max} */
    unsigned int	padding:28;
    unsigned int	firstCol;
    unsigned int	lastCol;
    unsigned int	firstRow;
    unsigned int	lastRow;
    unsigned int	nProps;
    unsigned int	lenStrings; /* length in bytes of string table */
    unsigned int	chDefault;  /* default character */ 
    int			fontDescent; /* minimum for quality typography */
    int			fontAscent;  /* minimum for quality typography */
    CharInfoRec		minbounds;  /* MIN of glyph metrics over all chars */
    CharInfoRec		maxbounds;  /* MAX of glyph metrics over all chars */
    unsigned int	pixDepth;   /* intensity bits per pixel */
    unsigned int	glyphSets;  /* number of sets of glyphs, for
					    sub-pixel positioning */
    unsigned int	version2;   /* version stamp double-check */
} FontInfoRec;

typedef struct _ExtentInfo {
    DrawDirection	drawDirection;
    int			fontAscent;
    int			fontDescent;
    int			overallAscent;
    int			overallDescent;
    int			overallWidth;
    int			overallLeft;
    int			overallRight;
} ExtentInfoRec;

/*
 * This file describes the Server Natural Font format.
 * SNF fonts are both CPU-dependent and frame buffer bit order dependent.
 * This file is used by:
 *	1)  the server, to hold font information read out of font files.
 *	2)  font converters
 *
 * Each font file contains the following
 * data structures, with no padding in-between.
 *
 *	1)  The XFONTINFO structure
 *		hand-padded to a two-short boundary.
 *		maxbounds.byteoffset is the total number of bytes in the
 *			glpyh array
 *		maxbounds.bitOffset is thetotal width of the unpadded font
 *
 *	2)  The XCHARINFO array
 *		indexed directly with character codes, both on disk
 *		and in memory.
 *
 *	3)  Character glyphs
 *		padded in the server-natural way, and
 *		ordered in the device-natural way.
 *		End of glyphs padded to 32-bit boundary.
 *
 *	4)  nProps font properties
 *
 *	5)  a sequence of null-terminated strings, for font properties
 */

#define FONT_FILE_VERSION	4

typedef struct _FontProp { 
	CARD32	name;		/* offset of string */
	INT32	value;		/* number or offset of string */
	Bool	indirect;	/* value is a string offset */
} FontPropRec;

/*
 * the following macro definitions describe a font file image in memory
 */
#define ADDRCharInfoRec( pfi)	\
	((CharInfoRec *) &(pfi)[1])

#define ADDRCHARGLYPHS( pfi)	\
	(((char *) &(pfi)[1]) + BYTESOFCHARINFO(pfi))

/*
 * pad out glyphs to a CARD32 boundary
 */
#define ADDRXFONTPROPS( pfi)  \
	((DIXFontProp *) ((char *)ADDRCHARGLYPHS( pfi) + BYTESOFGLYPHINFO(pfi)))

#define ADDRSTRINGTAB( pfi)  \
	((char *)ADDRXFONTPROPS( pfi) + BYTESOFPROPINFO(pfi))

#define	BYTESOFFONTINFO(pfi)	(sizeof(FontInfoRec))
#define BYTESOFCHARINFO(pfi)	(sizeof(CharInfoRec) * n2dChars(pfi))
#define	BYTESOFPROPINFO(pfi)	(sizeof(FontPropRec) * (pfi)->nProps)
#define	BYTESOFSTRINGINFO(pfi)	((pfi)->lenStrings)
#define	BYTESOFGLYPHINFO(pfi)	(((pfi)->maxbounds.byteOffset+3) & ~0x3)
#define BYTESOFINKINFO(pfi)	(sizeof(CharInfoRec) * (2 + n2dChars(pfi)))

