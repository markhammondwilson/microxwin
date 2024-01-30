/*								 
 * Module Name - xevent.c
 *
 * Description -
 *     Sending kbd/mouse events to kernel..
 *
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
/* Please note this software and associated kernel module only supports PS2 mouse */
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <linux/kd.h>
#include "x11ioctl.h"
#ifdef TSMOUSE
#include <tslib.h>
#endif
#define INBYTES  64
unsigned char msgbuf[INBYTES+2]; /* event, count, 64bytes of buffer */
static int kbdtrans;
static struct termios kbdtty;
static long kflag;
int  mousefd, kbdfd;
static struct tsdev *tsdev = NULL;

static void devclose(void )
{
#ifdef KBDENABLE
   /* restore old kbd values */
   ioctl(kbdfd, KDSETMODE, KD_TEXT);
   ioctl(kbdfd, KDSKBMODE, kbdtrans);
   tcsetattr(kbdfd, TCSAFLUSH, &kbdtty);
   fcntl(kbdfd, F_SETFL, kflag);
#endif

#ifdef TSMOUSE
   /* close mouse */
   ts_close(tsdev);
#else
   close(mousefd);
#endif

   return;
}
static void errorhandler(int signum)
{
   devclose();
   exit(0);
}

int main(int argc, char *argv[])
{
   struct termios newtty;
   int pfd, n;
   fd_set fdvar;
   struct sigaction sa;
         
   if(argc != 2)
      {
      fprintf(stderr, "Usage xevent inputdevice\n");
      exit(1);
      }
   
   /* open x11 input device */
   if( (pfd= open("/dev/x11input", O_WRONLY)) < 0)
      {
      printf("Cannot open /dev/x11input\n");
      exit (1);
      }
   
   mousefd=kbdfd=0;
#ifdef TSMOUSE
   if(!(tsdev = ts_open(argv[1], 1)))
      {
      fprintf(stderr, "Cannot open tsmouse input %s\n", argv[1]);
      exit(1);
      }
   if (ts_config(tsdev))
      {
      fprintf(stderr, "Cannot config tsmouse\n");
      exit(1);
      }
   if( (mousefd=ts_fd(tsdev)) < 0)
      {
      fprintf(stderr, "Cannot get ts_fd()\n");
      exit(1);
      }
#else
   mousefd = open(argv[1], O_NONBLOCK);
#endif

#ifdef KBDENABLE
   kbdfd = open("/dev/tty0", O_NONBLOCK);
   /* save old keyboard */
   ioctl(kbdfd, KDGKBMODE, &kbdtrans);
   tcgetattr(kbdfd, &kbdtty);
   kflag=fcntl(kbdfd, F_GETFL, 0);

   ioctl(kbdfd, KDSETMODE, KD_GRAPHICS);
   fcntl(kbdfd, F_SETFL, O_NONBLOCK);
   /* set new keyboard */
   ioctl(kbdfd, KDSKBMODE, K_RAW);

   newtty = kbdtty;
   newtty.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
   newtty.c_oflag = 0;
   newtty.c_cflag = CREAD | CS8;
   newtty.c_lflag = 0;
   newtty.c_cc[VMIN]=1;
   newtty.c_cc[VTIME]=0;
   newtty.c_cc[VSUSP]=0;
   cfsetispeed(&newtty, 9600);
   cfsetospeed(&newtty, 9600);
   tcsetattr(kbdfd, TCSANOW, &newtty);
#endif

/*   atexit(devclose); */

   /*
    * set up signal handler to restore state on signals.
    */
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = errorhandler;
   sigaction(SIGFPE, &sa, NULL);
   sigaction(SIGILL, &sa, NULL);
   sigaction(SIGBUS, &sa, NULL);
   sigaction(SIGSEGV, &sa, NULL);
   sigaction(SIGKILL, &sa, NULL);
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGHUP, &sa, NULL);
   sigaction(SIGTRAP, &sa, NULL);
   sigaction(SIGQUIT, &sa, NULL);
   sigaction(SIGINT, &sa, NULL);

   while(1)
      {      
      FD_ZERO(&fdvar);
      FD_SET(mousefd, &fdvar);
#ifdef KBDENABLE
      FD_SET(kbdfd, &fdvar); 
#endif
      select( ((kbdfd > mousefd) ? (kbdfd + 1) : (mousefd + 1)) , &fdvar, (fd_set *)0, (fd_set *)0, NULL);
#ifdef TSMOUSE
      if(FD_ISSET(mousefd, &fdvar))
         {
         struct ts_sample event;
         /* read and send mouse events, seems that I can read only 1 event */
         if( (n=ts_read(tsdev, &event, 1)) > 0)
            {
            msgbuf[0]=MOUSETS;
            msgbuf[1]=sizeof(struct tsevent);
            memcpy(&msgbuf[2], &event, sizeof(struct tsevent));
            write(pfd, msgbuf, (sizeof(struct tsevent)+2));
            }
         }
#else
      if(FD_ISSET(mousefd, &fdvar))
         {
         /* read and send mouse events */
         if( (n=read(mousefd, &msgbuf[2], INBYTES)) > 0)
            {
	    msgbuf[0]=MOUSEPS2;
	    msgbuf[1]=n;
            write(pfd, msgbuf, (n+2));
	    }
	 }
#endif
#ifdef KBDENABLE
      if(FD_ISSET(kbdfd, &fdvar))
         {
         /* read and send kbd events */
         if( (n=read(kbdfd, &msgbuf[2], INBYTES)) > 0)
            {
            msgbuf[0]=KBDEVENT;
            msgbuf[1]=n;
            write(pfd, msgbuf, (n+2));
            }
         }
#endif
      }
   return;
}
