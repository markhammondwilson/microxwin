#ifdef SYSV
#include <memory.h>
#define bzero(b,length) memset(b,0,length)
/* these are not strictly equivalent, but suffice for uses here */
#define bcopy(b1,b2,length) memcpy(b2,b1,length)
#endif /* SYSV */

#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct _GlyphMap {
    char	*bits;
    int		h;
    int		w;
    int		widthBytes;
} GlyphMap;

/*
 * a structure to hold all the pointers to make it easy to pass them all
 * around. Much like the FONT structure in the server.
 */

typedef struct _TempFont {
    FontInfoPtr pFI;
    CharInfoPtr pCI;
    unsigned char *pGlyphs;
    FontPropPtr pFP;
    CharInfoPtr pInkCI;
    CharInfoPtr pInkMin;
    CharInfoPtr pInkMax;
} TempFont; /* not called font since collides with type in X.h */

#define DEFAULTGLPAD 	1		/* default padding for glyphs */
#define DEFAULTBITORDER LSBFirst	/* default bitmap bit order */
#define DEFAULTBYTEORDER LSBFirst	/* default bitmap byte order */
#define DEFAULTSCANUNIT	1		/* default bitmap scan unit */

#define GLWIDTHBYTESPADDED(bits,nbytes) \
	((nbytes) == 1 ? (((bits)+7)>>3)	/* pad to 1 byte */ \
	:(nbytes) == 2 ? ((((bits)+15)>>3)&~1)	/* pad to 2 bytes */ \
	:(nbytes) == 4 ? ((((bits)+31)>>3)&~3)	/* pad to 4 bytes */ \
	:(nbytes) == 8 ? ((((bits)+63)>>3)&~7)	/* pad to 8 bytes */ \
	: 0)

