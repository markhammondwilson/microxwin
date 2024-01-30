/*								 
 * Module Name - xlibglue.h
 *
 * Description -
 *     Xlib kernel interface.
 *
 * Copyright (C) 2000-2007  Vasant Kanchan http://www.microxwin.com
 *
 * This software may be freely copied and redistributed without
 * fee provided that this copyright notice is preserved intact on all
 * copies and modified copies. 
 * 
 * However this file cannot be modified without consent of Vasant Kanchan.
 * Please contact vkanchan@microxwin.com
 *
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage or
 * its effect upon hardware or computer systems.
 */
#ifndef _XLIBGLUE_H_
#define	_XLIBGLUE_H_

#define CMDTHRESH	16000	/* OpenDis.c should allocate enough buffer */
#define CMDBUFSIZE	16384
#define TRAPTHRESH 512	/* if larger than this trap to the kernel */

#ifndef __KERNEL__
#define client(dpy)	(((_XPrivDisplay)dpy)->private2)
#define _wrptr(dpy)	((unsigned int *)(((_XPrivDisplay)dpy)->private11))
#define _wrindex(dpy)	(((_XPrivDisplay)dpy)->private8)
#define _incrindex(dpy)	(((_XPrivDisplay)dpy)->private8++)
#define writedpy(dpy, data)   *(_wrptr(dpy) + _incrindex(dpy))= ((unsigned int)(data))
/* check for space in the command buffer */
#define CHKSPACE(dpy, d) 	if( (_wrindex(dpy) + (d)) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMDRAW1(dpy, cmd)	\
	writedpy(dpy, (cmd));	

#define XCMDRAW2(dpy, cmd, arg2)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	

#define XCMDRAW3(dpy, cmd, arg2, arg3)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	

#define XCMDRAW4(dpy, cmd, arg2, arg3, arg4)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	

#define XCMDRAW5(dpy, cmd, arg2, arg3, arg4, arg5)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	

#define XCMDRAW6(dpy, cmd, arg2, arg3, arg4, arg5, arg6)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	

#define XCMDRAW7(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	

#define XCMDRAW8(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	

#define XCMDRAW9(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	\
	writedpy(dpy, (arg9));	

#define XCMDRAW10(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	\
	writedpy(dpy, (arg9));	\
	writedpy(dpy, (arg10));	

#define XCMDRAW11(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	\
	writedpy(dpy, (arg9));	\
	writedpy(dpy, (arg10));	\
	writedpy(dpy, (arg11));	

#define XCMDRAW12(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	\
	writedpy(dpy, (arg9));	\
	writedpy(dpy, (arg10));	\
	writedpy(dpy, (arg11));	\
	writedpy(dpy, (arg12));	

#define XCMDRAW13(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)	\
	writedpy(dpy, (cmd));	\
	writedpy(dpy, (arg2));	\
	writedpy(dpy, (arg3));	\
	writedpy(dpy, (arg4));	\
	writedpy(dpy, (arg5));	\
	writedpy(dpy, (arg6));	\
	writedpy(dpy, (arg7));	\
	writedpy(dpy, (arg8));	\
	writedpy(dpy, (arg9));	\
	writedpy(dpy, (arg10));	\
	writedpy(dpy, (arg11));	\
	writedpy(dpy, (arg12));	\
	writedpy(dpy, (arg13));	

#define XCMDSYNC1(dpy, cmd)  \
        XCMDRAW1(dpy,cmd)   \
	__XFlush(dpy); 

#define XCMDSYNC2(dpy, cmd, arg2)	\
        XCMDRAW2(dpy, cmd, arg2)   \
	__XFlush(dpy); 

#define XCMDSYNC3(dpy, cmd, arg2, arg3)	\
        XCMDRAW3(dpy, cmd, arg2, arg3)   \
	__XFlush(dpy); 

#define XCMDSYNC4(dpy, cmd, arg2, arg3, arg4)	\
        XCMDRAW4(dpy, cmd, arg2, arg3, arg4)   \
	__XFlush(dpy); 

#define XCMDSYNC5(dpy, cmd, arg2, arg3, arg4, arg5)	\
        XCMDRAW5(dpy, cmd, arg2, arg3, arg4, arg5)   \
	__XFlush(dpy); 

#define XCMDSYNC6(dpy, cmd, arg2, arg3, arg4, arg5, arg6)	\
        XCMDRAW6(dpy, cmd, arg2, arg3, arg4, arg5, arg6)   \
	__XFlush(dpy); 

#define XCMDSYNC7(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7)	\
        XCMDRAW7(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7)   \
	__XFlush(dpy); 

#define XCMDSYNC8(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8)	\
        XCMDRAW8(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8)   \
	__XFlush(dpy); 

#define XCMDSYNC9(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)	\
        XCMDRAW9(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)   \
	__XFlush(dpy); 

#define XCMDSYNC10(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)	\
        XCMDRAW10(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)   \
	__XFlush(dpy); 

#define XCMDSYNC11(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)	\
        XCMDRAW11(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)   \
	__XFlush(dpy); 

#define XCMDSYNC12(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
        XCMDRAW12(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)   \
	__XFlush(dpy); 

#define XCMDSYNC13(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)	\
        XCMDRAW13(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)   \
	__XFlush(dpy); 

#ifdef XCMDDEBUG
/* in debug mode everything is sync */
#define XCMD1 XCMDSYNC1
#define XCMD2 XCMDSYNC2
#define XCMD3 XCMDSYNC3
#define XCMD4 XCMDSYNC4
#define XCMD5 XCMDSYNC5
#define XCMD6 XCMDSYNC6
#define XCMD7 XCMDSYNC7
#define XCMD8 XCMDSYNC8
#define XCMD9 XCMDSYNC9
#define XCMD10 XCMDSYNC10
#define XCMD11 XCMDSYNC11
#define XCMD12 XCMDSYNC12
#define XCMD13 XCMDSYNC13
#else
#define XCMD1(dpy, cmd)  \
        XCMDRAW1(dpy,cmd)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD2(dpy, cmd, arg2)	\
        XCMDRAW2(dpy, cmd, arg2)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD3(dpy, cmd, arg2, arg3)	\
        XCMDRAW3(dpy, cmd, arg2, arg3)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD4(dpy, cmd, arg2, arg3, arg4)	\
        XCMDRAW4(dpy, cmd, arg2, arg3, arg4)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD5(dpy, cmd, arg2, arg3, arg4, arg5)	\
        XCMDRAW5(dpy, cmd, arg2, arg3, arg4, arg5)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD6(dpy, cmd, arg2, arg3, arg4, arg5, arg6)	\
        XCMDRAW6(dpy, cmd, arg2, arg3, arg4, arg5, arg6)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD7(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7)	\
        XCMDRAW7(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD8(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8)	\
        XCMDRAW8(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD9(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)	\
        XCMDRAW9(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD10(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)	\
        XCMDRAW10(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD11(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)	\
        XCMDRAW11(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD12(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)	\
        XCMDRAW12(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 

#define XCMD13(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)	\
        XCMDRAW13(dpy, cmd, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13)   \
	if(_wrindex(dpy) >= CMDTHRESH)  __XFlush(dpy); 
#endif /* XCMDDEBUG */
#endif

#endif /* _XLIBGLUE_H_ */
