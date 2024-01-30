This release consists of the whole user space libraries and utilities required to run MicroXwin.
All of the userspace code is released under a liberal license see COPYING for further information.
The userspace code depends on a kernel module which is proprietary. However we plan to provide modules
for popular x86 distribution that will be free for non-commercial use.
For details on MicroXwin please visit http://www.microxwin.com. Report bugs to bugs@microxwin.com and
for support issues email support@microxwin.com

DIRECTORY LAYOUT

X11 - This consist of the replacement libX11.so library. The feature set is simplified and doesn't support XKB extension.
      It also includes user space program xevent which reads keyboard and mouse events and writes to kernel module.

Xext- This consists of replacement libXext.so library. This currently only supports Xshm and Xtst extension.

Xtst- Xtst extension for touch sreen. Not used for now.

fonts- Some standard SNF fonts and xfonts.dir that has to be copied to /usr/fonts on the target system.

utils- Bdftosnf tools and tool to create xfonts.dir

modules- Binary kernel modules that talks with user space library.
 
gtkfixes- Patch to gtk+-2 series so that it doesn't  go below the Xlib layer. MicroXwin is only compatable at the Xlib layer.

QUICK BUILD STEPS

1. Go to X11 and type make -f makefile.linux. This will create libX11.so.6.2.0. This makefile assumes the new X11 layout
   as in Fedora6/7 and Slackware 12. Backup original libX11.so.6.2.0 and copy this libX11.so.6.2.0 to /usr/lib.

2. Do the same for Xext and copy libXext.so.6.4.0 to /usr/lib

3. If you run gtk+2 programs create libgdk-x11-2.0.so.0.xxxx as documented in gtkfixes/README

4. Go to fonts and copy *.snf and xfonts.dir to /usr/fonts.

RUN STEPS.

1. Please login as root.

2. Change /etc/inittab (id:3:initdefaults) so that on bootup the OS
   boots to multiuser mode. We have to run X-Windows manually.

3. Change /boot/grub.conf so that kernel boots with linux frame buffer
   enabled. Add vga=791 (1024x768x16) to the kernel command:
   kernel /boot/vmlinuz-XXXX ro root=LABEL=/ vga=791 rhgb quiet.
   On reboot you should see the penguin logo. Other vga modes will also work.

4. Go to modules directory copy required module to x11.ko for example on fedora6 copy fedora6-x11.ko -> x11.ko.

5. ./runblackbox is a script that inserts x11.ko module and starts blackbox window manager & xevent processing.
 

BUILDING FROM SCRATCH

Beyond linux from scratch has details on building Xorg protocols, libraries and fonts. For non x86 platform 
this can be done in scratchbox. Simple way to build for MicroXwin would be to do the Xorg build first and then replace
the libX11.so.6.2.0 and libXext.so.6.4.0 built from the MicroXwin distribution.

