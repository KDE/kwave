####### This section is automatically generated from
#######    /root/debian/qt/qt1-1.31/debian/tmp/usr/doc/qt-doc/examples/Makefile

INCDIR	=	/usr/X11R6/include/qt -I/opt/kde/include
CFLAGS	=	-Wall -O6 -m486 -I/usr/X11R6/include
LFLAGS	=	-L/usr/X11R6/lib -L/opt/kde/lib -lXext -lqt -LX11 -lkdecore -lkdeui
CC	=	gcc
MOC	=	/usr/bin/moc

####### Files

HEADERS =       classes.h sample.h main.h
SOURCES =	signalview.cpp main.cpp sample.cpp mainwidget.cpp sampleio.c
OBJECTS =	main.o signalview.o sample.o mainwidget.o sampleio.o
SRCMETA =	mclasses.cpp msample.cpp mmain.cpp
OBJMETA =	mclasses.o msample.o mmain.o
TARGET	=	kwave
####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $< 
####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(TARGET)
	rm -f $(SRCMETA) $(TARGET)

######## Meta Objects

mmain.cpp: main.h
	$(MOC) -o mmain.cpp main.h   
mclasses.cpp: classes.h
	$(MOC) -o mclasses.cpp classes.h   
msample.cpp: sample.h
	$(MOC) -o msample.cpp sample.h   
# DO NOT DELETE THIS LINE -- make depend depends on it.

signalview.o: classes.h /usr/X11R6/include/qt/qapp.h
signalview.o: /usr/X11R6/include/qt/qwidget.h
signalview.o: /usr/X11R6/include/qt/qwindefs.h
signalview.o: /usr/X11R6/include/qt/qobjdefs.h
signalview.o: /usr/X11R6/include/qt/qglobal.h /usr/X11R6/include/qt/qobject.h
signalview.o: /usr/X11R6/include/qt/qstring.h /usr/X11R6/include/qt/qarray.h
signalview.o: /usr/X11R6/include/qt/qgarray.h /usr/X11R6/include/qt/qshared.h
signalview.o: /usr/X11R6/include/qt/qgeneric.h /usr/include/string.h
signalview.o: /usr/include/features.h /usr/include/sys/cdefs.h
signalview.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
signalview.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
signalview.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
signalview.o: /usr/X11R6/include/qt/qpaintd.h
signalview.o: /usr/X11R6/include/qt/qpalette.h /usr/X11R6/include/qt/qcolor.h
signalview.o: /usr/X11R6/include/qt/qcursor.h /usr/X11R6/include/qt/qfont.h
signalview.o: /usr/X11R6/include/qt/qfontmet.h
signalview.o: /usr/X11R6/include/qt/qfontinf.h
signalview.o: /usr/X11R6/include/qt/qlayout.h
signalview.o: /usr/X11R6/include/qt/qgmanagr.h
signalview.o: /usr/X11R6/include/qt/qintdict.h /usr/X11R6/include/qt/qgdict.h
signalview.o: /usr/X11R6/include/qt/qcollect.h /usr/X11R6/include/qt/qlist.h
signalview.o: /usr/X11R6/include/qt/qglist.h /usr/X11R6/include/qt/qpushbt.h
signalview.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qfile.h
signalview.o: /usr/X11R6/include/qt/qiodev.h /usr/include/stdio.h
signalview.o: /usr/include/libio.h /usr/include/_G_config.h
signalview.o: /usr/X11R6/include/qt/qscrbar.h
signalview.o: /usr/X11R6/include/qt/qrangect.h
signalview.o: /usr/X11R6/include/qt/qdrawutl.h
signalview.o: /usr/X11R6/include/qt/qpainter.h
signalview.o: /usr/X11R6/include/qt/qregion.h /usr/X11R6/include/qt/qpen.h
signalview.o: /usr/X11R6/include/qt/qbrush.h /usr/X11R6/include/qt/qpntarry.h
signalview.o: /usr/X11R6/include/qt/qwmatrix.h
signalview.o: /usr/X11R6/include/qt/qfiledlg.h /usr/X11R6/include/qt/qdir.h
signalview.o: /usr/X11R6/include/qt/qstrlist.h
signalview.o: /usr/X11R6/include/qt/qdstream.h
signalview.o: /usr/X11R6/include/qt/qfileinf.h
signalview.o: /usr/X11R6/include/qt/qdatetm.h /usr/X11R6/include/qt/qdialog.h
signalview.o: /usr/X11R6/include/qt/qcombo.h /opt/kde/include/kapp.h
signalview.o: /opt/kde/include/kconfig.h /opt/kde/include/kconfigbase.h
signalview.o: /opt/kde/include/kconfigdata.h /usr/X11R6/include/qt/qdict.h
signalview.o: /usr/X11R6/include/qt/qtstream.h /opt/kde/include/kdebug.h
signalview.o: /opt/kde/include/klocale.h /opt/kde/include/drag.h
signalview.o: /usr/X11R6/include/qt/qpixmap.h /usr/include/X11/X.h
signalview.o: /usr/X11R6/include/qt/qpopmenu.h
signalview.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qframe.h
signalview.o: /usr/X11R6/include/qt/qmenudta.h /opt/kde/include/kslider.h
signalview.o: /usr/X11R6/include/qt/qslider.h /opt/kde/include/kselect.h
signalview.o: /usr/X11R6/include/qt/qlined.h /opt/kde/include/ktopwidget.h
signalview.o: /usr/include/stdlib.h /usr/include/errno.h
signalview.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
signalview.o: /usr/include/alloca.h /opt/kde/include/ktoolbar.h
signalview.o: /opt/kde/include/kbutton.h /opt/kde/include/kcombo.h
signalview.o: /usr/X11R6/include/qt/qlistbox.h /opt/kde/include/klined.h
signalview.o: /opt/kde/include/kmenubar.h /usr/X11R6/include/qt/qmenubar.h
signalview.o: /opt/kde/include/kstatusbar.h /usr/X11R6/include/qt/qlabel.h
signalview.o: /opt/kde/include/kbuttonbox.h /usr/X11R6/include/qt/qtimer.h
signalview.o: sample.h sampleop.h
main.o: main.h /usr/X11R6/include/qt/qlist.h /usr/X11R6/include/qt/qglist.h
main.o: /usr/X11R6/include/qt/qcollect.h /usr/X11R6/include/qt/qglobal.h
main.o: /usr/X11R6/include/qt/qgeneric.h classes.h
main.o: /usr/X11R6/include/qt/qapp.h /usr/X11R6/include/qt/qwidget.h
main.o: /usr/X11R6/include/qt/qwindefs.h /usr/X11R6/include/qt/qobjdefs.h
main.o: /usr/X11R6/include/qt/qobject.h /usr/X11R6/include/qt/qstring.h
main.o: /usr/X11R6/include/qt/qarray.h /usr/X11R6/include/qt/qgarray.h
main.o: /usr/X11R6/include/qt/qshared.h /usr/include/string.h
main.o: /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
main.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
main.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
main.o: /usr/X11R6/include/qt/qpaintd.h /usr/X11R6/include/qt/qpalette.h
main.o: /usr/X11R6/include/qt/qcolor.h /usr/X11R6/include/qt/qcursor.h
main.o: /usr/X11R6/include/qt/qfont.h /usr/X11R6/include/qt/qfontmet.h
main.o: /usr/X11R6/include/qt/qfontinf.h /usr/X11R6/include/qt/qlayout.h
main.o: /usr/X11R6/include/qt/qgmanagr.h /usr/X11R6/include/qt/qintdict.h
main.o: /usr/X11R6/include/qt/qgdict.h /usr/X11R6/include/qt/qpushbt.h
main.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qfile.h
main.o: /usr/X11R6/include/qt/qiodev.h /usr/include/stdio.h
main.o: /usr/include/libio.h /usr/include/_G_config.h
main.o: /usr/X11R6/include/qt/qscrbar.h /usr/X11R6/include/qt/qrangect.h
main.o: /usr/X11R6/include/qt/qdrawutl.h /usr/X11R6/include/qt/qpainter.h
main.o: /usr/X11R6/include/qt/qregion.h /usr/X11R6/include/qt/qpen.h
main.o: /usr/X11R6/include/qt/qbrush.h /usr/X11R6/include/qt/qpntarry.h
main.o: /usr/X11R6/include/qt/qwmatrix.h /usr/X11R6/include/qt/qfiledlg.h
main.o: /usr/X11R6/include/qt/qdir.h /usr/X11R6/include/qt/qstrlist.h
main.o: /usr/X11R6/include/qt/qdstream.h /usr/X11R6/include/qt/qfileinf.h
main.o: /usr/X11R6/include/qt/qdatetm.h /usr/X11R6/include/qt/qdialog.h
main.o: /usr/X11R6/include/qt/qcombo.h /opt/kde/include/kapp.h
main.o: /opt/kde/include/kconfig.h /opt/kde/include/kconfigbase.h
main.o: /opt/kde/include/kconfigdata.h /usr/X11R6/include/qt/qdict.h
main.o: /usr/X11R6/include/qt/qtstream.h /opt/kde/include/kdebug.h
main.o: /opt/kde/include/klocale.h /opt/kde/include/drag.h
main.o: /usr/X11R6/include/qt/qpixmap.h /usr/include/X11/X.h
main.o: /usr/X11R6/include/qt/qpopmenu.h /usr/X11R6/include/qt/qtablevw.h
main.o: /usr/X11R6/include/qt/qframe.h /usr/X11R6/include/qt/qmenudta.h
main.o: /opt/kde/include/kslider.h /usr/X11R6/include/qt/qslider.h
main.o: /opt/kde/include/kselect.h /usr/X11R6/include/qt/qlined.h
main.o: /opt/kde/include/ktopwidget.h /usr/include/stdlib.h
main.o: /usr/include/errno.h /usr/include/linux/errno.h
main.o: /usr/include/asm/errno.h /usr/include/alloca.h
main.o: /opt/kde/include/ktoolbar.h /opt/kde/include/kbutton.h
main.o: /opt/kde/include/kcombo.h /usr/X11R6/include/qt/qlistbox.h
main.o: /opt/kde/include/klined.h /opt/kde/include/kmenubar.h
main.o: /usr/X11R6/include/qt/qmenubar.h /opt/kde/include/kstatusbar.h
main.o: /usr/X11R6/include/qt/qlabel.h /opt/kde/include/kbuttonbox.h
main.o: /usr/X11R6/include/qt/qtimer.h sample.h sampleop.h
main.o: /usr/X11R6/include/qt/qkeycode.h
sample.o: /usr/include/unistd.h /usr/include/features.h
sample.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
sample.o: /usr/include/gnu/types.h
sample.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
sample.o: /usr/include/confname.h /usr/include/sys/types.h
sample.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
sample.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
sample.o: /usr/include/sys/bitypes.h /usr/include/fcntl.h
sample.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
sample.o: /usr/include/sys/ioctl.h /usr/include/linux/ioctl.h
sample.o: /usr/include/asm/ioctl.h /usr/include/termios.h
sample.o: /usr/include/linux/termios.h /usr/include/asm/termios.h
sample.o: /usr/include/asm/termbits.h /usr/include/asm/ioctls.h
sample.o: /usr/include/sys/socketio.h /usr/include/linux/sockios.h
sample.o: /usr/include/asm/sockios.h sample.h /usr/X11R6/include/qt/qapp.h
sample.o: /usr/X11R6/include/qt/qwidget.h /usr/X11R6/include/qt/qwindefs.h
sample.o: /usr/X11R6/include/qt/qobjdefs.h /usr/X11R6/include/qt/qglobal.h
sample.o: /usr/X11R6/include/qt/qobject.h /usr/X11R6/include/qt/qstring.h
sample.o: /usr/X11R6/include/qt/qarray.h /usr/X11R6/include/qt/qgarray.h
sample.o: /usr/X11R6/include/qt/qshared.h /usr/X11R6/include/qt/qgeneric.h
sample.o: /usr/include/string.h /usr/X11R6/include/qt/qevent.h
sample.o: /usr/X11R6/include/qt/qrect.h /usr/X11R6/include/qt/qsize.h
sample.o: /usr/X11R6/include/qt/qpoint.h /usr/X11R6/include/qt/qpaintd.h
sample.o: /usr/X11R6/include/qt/qpalette.h /usr/X11R6/include/qt/qcolor.h
sample.o: /usr/X11R6/include/qt/qcursor.h /usr/X11R6/include/qt/qfont.h
sample.o: /usr/X11R6/include/qt/qfontmet.h /usr/X11R6/include/qt/qfontinf.h
sample.o: /opt/kde/include/kapp.h /opt/kde/include/kconfig.h
sample.o: /opt/kde/include/kconfigbase.h /usr/X11R6/include/qt/qstrlist.h
sample.o: /usr/X11R6/include/qt/qlist.h /usr/X11R6/include/qt/qglist.h
sample.o: /usr/X11R6/include/qt/qcollect.h /usr/X11R6/include/qt/qdstream.h
sample.o: /usr/X11R6/include/qt/qiodev.h /opt/kde/include/kconfigdata.h
sample.o: /usr/X11R6/include/qt/qdict.h /usr/X11R6/include/qt/qgdict.h
sample.o: /usr/X11R6/include/qt/qtstream.h /usr/include/stdio.h
sample.o: /usr/include/libio.h /usr/include/_G_config.h
sample.o: /usr/X11R6/include/qt/qfile.h /opt/kde/include/kdebug.h
sample.o: /opt/kde/include/klocale.h /usr/X11R6/include/qt/qintdict.h
sample.o: /opt/kde/include/drag.h /usr/X11R6/include/qt/qpixmap.h
sample.o: /usr/include/X11/X.h /usr/X11R6/include/qt/qpopmenu.h
sample.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qframe.h
sample.o: /usr/X11R6/include/qt/qmenudta.h sampleop.h
sample.o: /opt/kde/include/kmsgbox.h /usr/X11R6/include/qt/qdialog.h
sample.o: /usr/X11R6/include/qt/qpushbt.h /usr/X11R6/include/qt/qbutton.h
sample.o: /usr/X11R6/include/qt/qlabel.h /opt/kde/include/kprogress.h
sample.o: /usr/X11R6/include/qt/qrangect.h /usr/include/linux/soundcard.h
sample.o: /usr/include/sys/ipc.h /usr/include/linux/ipc.h
sample.o: /usr/include/sys/shm.h /usr/include/linux/shm.h
sample.o: /usr/include/asm/shmparam.h
mainwidget.o: classes.h /usr/X11R6/include/qt/qapp.h
mainwidget.o: /usr/X11R6/include/qt/qwidget.h
mainwidget.o: /usr/X11R6/include/qt/qwindefs.h
mainwidget.o: /usr/X11R6/include/qt/qobjdefs.h
mainwidget.o: /usr/X11R6/include/qt/qglobal.h /usr/X11R6/include/qt/qobject.h
mainwidget.o: /usr/X11R6/include/qt/qstring.h /usr/X11R6/include/qt/qarray.h
mainwidget.o: /usr/X11R6/include/qt/qgarray.h /usr/X11R6/include/qt/qshared.h
mainwidget.o: /usr/X11R6/include/qt/qgeneric.h /usr/include/string.h
mainwidget.o: /usr/include/features.h /usr/include/sys/cdefs.h
mainwidget.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
mainwidget.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
mainwidget.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
mainwidget.o: /usr/X11R6/include/qt/qpaintd.h
mainwidget.o: /usr/X11R6/include/qt/qpalette.h /usr/X11R6/include/qt/qcolor.h
mainwidget.o: /usr/X11R6/include/qt/qcursor.h /usr/X11R6/include/qt/qfont.h
mainwidget.o: /usr/X11R6/include/qt/qfontmet.h
mainwidget.o: /usr/X11R6/include/qt/qfontinf.h
mainwidget.o: /usr/X11R6/include/qt/qlayout.h
mainwidget.o: /usr/X11R6/include/qt/qgmanagr.h
mainwidget.o: /usr/X11R6/include/qt/qintdict.h /usr/X11R6/include/qt/qgdict.h
mainwidget.o: /usr/X11R6/include/qt/qcollect.h /usr/X11R6/include/qt/qlist.h
mainwidget.o: /usr/X11R6/include/qt/qglist.h /usr/X11R6/include/qt/qpushbt.h
mainwidget.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qfile.h
mainwidget.o: /usr/X11R6/include/qt/qiodev.h /usr/include/stdio.h
mainwidget.o: /usr/include/libio.h /usr/include/_G_config.h
mainwidget.o: /usr/X11R6/include/qt/qscrbar.h
mainwidget.o: /usr/X11R6/include/qt/qrangect.h
mainwidget.o: /usr/X11R6/include/qt/qdrawutl.h
mainwidget.o: /usr/X11R6/include/qt/qpainter.h
mainwidget.o: /usr/X11R6/include/qt/qregion.h /usr/X11R6/include/qt/qpen.h
mainwidget.o: /usr/X11R6/include/qt/qbrush.h /usr/X11R6/include/qt/qpntarry.h
mainwidget.o: /usr/X11R6/include/qt/qwmatrix.h
mainwidget.o: /usr/X11R6/include/qt/qfiledlg.h /usr/X11R6/include/qt/qdir.h
mainwidget.o: /usr/X11R6/include/qt/qstrlist.h
mainwidget.o: /usr/X11R6/include/qt/qdstream.h
mainwidget.o: /usr/X11R6/include/qt/qfileinf.h
mainwidget.o: /usr/X11R6/include/qt/qdatetm.h /usr/X11R6/include/qt/qdialog.h
mainwidget.o: /usr/X11R6/include/qt/qcombo.h /opt/kde/include/kapp.h
mainwidget.o: /opt/kde/include/kconfig.h /opt/kde/include/kconfigbase.h
mainwidget.o: /opt/kde/include/kconfigdata.h /usr/X11R6/include/qt/qdict.h
mainwidget.o: /usr/X11R6/include/qt/qtstream.h /opt/kde/include/kdebug.h
mainwidget.o: /opt/kde/include/klocale.h /opt/kde/include/drag.h
mainwidget.o: /usr/X11R6/include/qt/qpixmap.h /usr/include/X11/X.h
mainwidget.o: /usr/X11R6/include/qt/qpopmenu.h
mainwidget.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qframe.h
mainwidget.o: /usr/X11R6/include/qt/qmenudta.h /opt/kde/include/kslider.h
mainwidget.o: /usr/X11R6/include/qt/qslider.h /opt/kde/include/kselect.h
mainwidget.o: /usr/X11R6/include/qt/qlined.h /opt/kde/include/ktopwidget.h
mainwidget.o: /usr/include/stdlib.h /usr/include/errno.h
mainwidget.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
mainwidget.o: /usr/include/alloca.h /opt/kde/include/ktoolbar.h
mainwidget.o: /opt/kde/include/kbutton.h /opt/kde/include/kcombo.h
mainwidget.o: /usr/X11R6/include/qt/qlistbox.h /opt/kde/include/klined.h
mainwidget.o: /opt/kde/include/kmenubar.h /usr/X11R6/include/qt/qmenubar.h
mainwidget.o: /opt/kde/include/kstatusbar.h /usr/X11R6/include/qt/qlabel.h
mainwidget.o: /opt/kde/include/kbuttonbox.h /usr/X11R6/include/qt/qtimer.h
mainwidget.o: sample.h sampleop.h
