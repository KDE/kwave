####### Makefile for kwave
####### define IS_BIG_ENDIAN, if running Linux on 68k, PowerPc, etc...
####### remove the -m486 CFLAG

INCDIR	=	/usr/X11R6/include/qt -I/opt/kde/include
CFLAGS	=	-Wall -O6 -I/usr/X11R6/include -m486
LFLAGS	=	-L/usr/X11R6/lib -L/opt/kde/lib -lXext -lqt -LX11 -lkdecore -lkdeui
CC	=	gcc
MOC	=	/usr/bin/moc

####### Files

HEADERS =       classes.h sample.h main.h about.h fftview.h\
		gsl_fft.h
SOURCES =	signalview.cpp main.cpp sample.cpp mainwidget.cpp sampleio.c\
		dialogs.cpp about.cpp fftview.cpp gsl_fft.cpp 
OBJECTS =	main.o signalview.o sample.o mainwidget.o sampleio.o\
		dialogs.o about.o fftview.o gsl_fft.o 
SRCMETA =	mclasses.cpp msample.cpp mmain.cpp mabout.cpp mfftview.cpp
OBJMETA =	mclasses.o msample.o mmain.o mabout.o mfftview.o
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
mabout.cpp: about.h
	$(MOC) -o mabout.cpp about.h   
mfftview.cpp: fftview.h
	$(MOC) -o mfftview.cpp fftview.h   
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
signalview.o: sample.h /usr/X11R6/include/qt/qbttngrp.h
signalview.o: /usr/X11R6/include/qt/qgrpbox.h
signalview.o: /usr/X11R6/include/qt/qradiobt.h gsl_fft.h sampleop.h
signalview.o: /usr/include/math.h /usr/include/huge_val.h
signalview.o: /usr/include/endian.h /usr/include/bytesex.h /usr/include/nan.h
signalview.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
signalview.o: /usr/include/values.h /usr/include/ieee754.h
signalview.o: /usr/include/ieee854.h /usr/include/limits.h
signalview.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
signalview.o: /usr/include/posix2_lim.h
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
main.o: /usr/X11R6/include/qt/qtimer.h sample.h
main.o: /usr/X11R6/include/qt/qbttngrp.h /usr/X11R6/include/qt/qgrpbox.h
main.o: /usr/X11R6/include/qt/qradiobt.h gsl_fft.h sampleop.h about.h
main.o: /usr/X11R6/include/qt/qmlined.h /usr/X11R6/include/qt/qimage.h
main.o: /usr/X11R6/include/qt/qkeycode.h
sample.o: /usr/include/unistd.h /usr/include/features.h
sample.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
sample.o: /usr/include/gnu/types.h
sample.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
sample.o: /usr/include/confname.h /usr/include/sys/types.h
sample.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
sample.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
sample.o: /usr/include/sys/bitypes.h sample.h /usr/include/stdlib.h
sample.o: /usr/include/errno.h /usr/include/linux/errno.h
sample.o: /usr/include/asm/errno.h /usr/include/alloca.h
sample.o: /usr/X11R6/include/qt/qapp.h /usr/X11R6/include/qt/qwidget.h
sample.o: /usr/X11R6/include/qt/qwindefs.h /usr/X11R6/include/qt/qobjdefs.h
sample.o: /usr/X11R6/include/qt/qglobal.h /usr/X11R6/include/qt/qobject.h
sample.o: /usr/X11R6/include/qt/qstring.h /usr/X11R6/include/qt/qarray.h
sample.o: /usr/X11R6/include/qt/qgarray.h /usr/X11R6/include/qt/qshared.h
sample.o: /usr/X11R6/include/qt/qgeneric.h /usr/include/string.h
sample.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
sample.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
sample.o: /usr/X11R6/include/qt/qpaintd.h /usr/X11R6/include/qt/qpalette.h
sample.o: /usr/X11R6/include/qt/qcolor.h /usr/X11R6/include/qt/qcursor.h
sample.o: /usr/X11R6/include/qt/qfont.h /usr/X11R6/include/qt/qfontmet.h
sample.o: /usr/X11R6/include/qt/qfontinf.h /usr/X11R6/include/qt/qlabel.h
sample.o: /usr/X11R6/include/qt/qframe.h /usr/X11R6/include/qt/qscrbar.h
sample.o: /usr/X11R6/include/qt/qrangect.h /usr/X11R6/include/qt/qdrawutl.h
sample.o: /usr/X11R6/include/qt/qpainter.h /usr/X11R6/include/qt/qregion.h
sample.o: /usr/X11R6/include/qt/qpen.h /usr/X11R6/include/qt/qbrush.h
sample.o: /usr/X11R6/include/qt/qpntarry.h /usr/X11R6/include/qt/qwmatrix.h
sample.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qcombo.h
sample.o: /usr/X11R6/include/qt/qdialog.h /usr/X11R6/include/qt/qbttngrp.h
sample.o: /usr/X11R6/include/qt/qgrpbox.h /usr/X11R6/include/qt/qradiobt.h
sample.o: gsl_fft.h /opt/kde/include/kapp.h /opt/kde/include/kconfig.h
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
sample.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qmenudta.h
sample.o: sampleop.h /opt/kde/include/kmsgbox.h
sample.o: /usr/X11R6/include/qt/qpushbt.h /opt/kde/include/kprogress.h
sample.o: /usr/include/sys/ipc.h /usr/include/linux/ipc.h
sample.o: /usr/include/sys/shm.h /usr/include/linux/shm.h
sample.o: /usr/include/asm/shmparam.h /usr/include/limits.h
sample.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
sample.o: /usr/include/posix2_lim.h /usr/include/math.h
sample.o: /usr/include/huge_val.h /usr/include/endian.h
sample.o: /usr/include/bytesex.h /usr/include/nan.h
sample.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
sample.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
sample.o: fftview.h /opt/kde/include/kslider.h
sample.o: /usr/X11R6/include/qt/qslider.h /opt/kde/include/kselect.h
sample.o: /usr/X11R6/include/qt/qlined.h /opt/kde/include/ktopwidget.h
sample.o: /opt/kde/include/ktoolbar.h /opt/kde/include/kbutton.h
sample.o: /opt/kde/include/kcombo.h /usr/X11R6/include/qt/qlistbox.h
sample.o: /opt/kde/include/klined.h /opt/kde/include/kmenubar.h
sample.o: /usr/X11R6/include/qt/qmenubar.h /opt/kde/include/kstatusbar.h
sample.o: /opt/kde/include/kbuttonbox.h /usr/X11R6/include/qt/qtimer.h
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
mainwidget.o: sample.h /usr/X11R6/include/qt/qbttngrp.h
mainwidget.o: /usr/X11R6/include/qt/qgrpbox.h
mainwidget.o: /usr/X11R6/include/qt/qradiobt.h gsl_fft.h sampleop.h
mainwidget.o: /usr/X11R6/include/qt/qkeycode.h
dialogs.o: /usr/include/unistd.h /usr/include/features.h
dialogs.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
dialogs.o: /usr/include/gnu/types.h
dialogs.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
dialogs.o: /usr/include/confname.h /usr/include/sys/types.h
dialogs.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
dialogs.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
dialogs.o: /usr/include/sys/bitypes.h sample.h /usr/include/stdlib.h
dialogs.o: /usr/include/errno.h /usr/include/linux/errno.h
dialogs.o: /usr/include/asm/errno.h /usr/include/alloca.h
dialogs.o: /usr/X11R6/include/qt/qapp.h /usr/X11R6/include/qt/qwidget.h
dialogs.o: /usr/X11R6/include/qt/qwindefs.h /usr/X11R6/include/qt/qobjdefs.h
dialogs.o: /usr/X11R6/include/qt/qglobal.h /usr/X11R6/include/qt/qobject.h
dialogs.o: /usr/X11R6/include/qt/qstring.h /usr/X11R6/include/qt/qarray.h
dialogs.o: /usr/X11R6/include/qt/qgarray.h /usr/X11R6/include/qt/qshared.h
dialogs.o: /usr/X11R6/include/qt/qgeneric.h /usr/include/string.h
dialogs.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
dialogs.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
dialogs.o: /usr/X11R6/include/qt/qpaintd.h /usr/X11R6/include/qt/qpalette.h
dialogs.o: /usr/X11R6/include/qt/qcolor.h /usr/X11R6/include/qt/qcursor.h
dialogs.o: /usr/X11R6/include/qt/qfont.h /usr/X11R6/include/qt/qfontmet.h
dialogs.o: /usr/X11R6/include/qt/qfontinf.h /usr/X11R6/include/qt/qlabel.h
dialogs.o: /usr/X11R6/include/qt/qframe.h /usr/X11R6/include/qt/qscrbar.h
dialogs.o: /usr/X11R6/include/qt/qrangect.h /usr/X11R6/include/qt/qdrawutl.h
dialogs.o: /usr/X11R6/include/qt/qpainter.h /usr/X11R6/include/qt/qregion.h
dialogs.o: /usr/X11R6/include/qt/qpen.h /usr/X11R6/include/qt/qbrush.h
dialogs.o: /usr/X11R6/include/qt/qpntarry.h /usr/X11R6/include/qt/qwmatrix.h
dialogs.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qcombo.h
dialogs.o: /usr/X11R6/include/qt/qdialog.h /usr/X11R6/include/qt/qbttngrp.h
dialogs.o: /usr/X11R6/include/qt/qgrpbox.h /usr/X11R6/include/qt/qradiobt.h
dialogs.o: gsl_fft.h /opt/kde/include/kapp.h /opt/kde/include/kconfig.h
dialogs.o: /opt/kde/include/kconfigbase.h /usr/X11R6/include/qt/qstrlist.h
dialogs.o: /usr/X11R6/include/qt/qlist.h /usr/X11R6/include/qt/qglist.h
dialogs.o: /usr/X11R6/include/qt/qcollect.h /usr/X11R6/include/qt/qdstream.h
dialogs.o: /usr/X11R6/include/qt/qiodev.h /opt/kde/include/kconfigdata.h
dialogs.o: /usr/X11R6/include/qt/qdict.h /usr/X11R6/include/qt/qgdict.h
dialogs.o: /usr/X11R6/include/qt/qtstream.h /usr/include/stdio.h
dialogs.o: /usr/include/libio.h /usr/include/_G_config.h
dialogs.o: /usr/X11R6/include/qt/qfile.h /opt/kde/include/kdebug.h
dialogs.o: /opt/kde/include/klocale.h /usr/X11R6/include/qt/qintdict.h
dialogs.o: /opt/kde/include/drag.h /usr/X11R6/include/qt/qpixmap.h
dialogs.o: /usr/include/X11/X.h /usr/X11R6/include/qt/qpopmenu.h
dialogs.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qmenudta.h
dialogs.o: sampleop.h /opt/kde/include/kmsgbox.h
dialogs.o: /usr/X11R6/include/qt/qpushbt.h /usr/X11R6/include/qt/qlayout.h
dialogs.o: /usr/X11R6/include/qt/qgmanagr.h /usr/X11R6/include/qt/qtooltip.h
dialogs.o: /usr/X11R6/include/qt/qtimer.h
about.o: about.h /usr/include/stdlib.h /usr/include/features.h
about.o: /usr/include/sys/cdefs.h
about.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
about.o: /usr/include/errno.h /usr/include/linux/errno.h
about.o: /usr/include/asm/errno.h /usr/include/alloca.h
about.o: /usr/X11R6/include/qt/qapp.h /usr/X11R6/include/qt/qwidget.h
about.o: /usr/X11R6/include/qt/qwindefs.h /usr/X11R6/include/qt/qobjdefs.h
about.o: /usr/X11R6/include/qt/qglobal.h /usr/X11R6/include/qt/qobject.h
about.o: /usr/X11R6/include/qt/qstring.h /usr/X11R6/include/qt/qarray.h
about.o: /usr/X11R6/include/qt/qgarray.h /usr/X11R6/include/qt/qshared.h
about.o: /usr/X11R6/include/qt/qgeneric.h /usr/include/string.h
about.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
about.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
about.o: /usr/X11R6/include/qt/qpaintd.h /usr/X11R6/include/qt/qpalette.h
about.o: /usr/X11R6/include/qt/qcolor.h /usr/X11R6/include/qt/qcursor.h
about.o: /usr/X11R6/include/qt/qfont.h /usr/X11R6/include/qt/qfontmet.h
about.o: /usr/X11R6/include/qt/qfontinf.h /usr/X11R6/include/qt/qpushbt.h
about.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qdialog.h
about.o: /usr/X11R6/include/qt/qmlined.h /usr/X11R6/include/qt/qtablevw.h
about.o: /usr/X11R6/include/qt/qframe.h /usr/X11R6/include/qt/qlist.h
about.o: /usr/X11R6/include/qt/qglist.h /usr/X11R6/include/qt/qcollect.h
about.o: /usr/X11R6/include/qt/qpainter.h /usr/X11R6/include/qt/qregion.h
about.o: /usr/X11R6/include/qt/qpen.h /usr/X11R6/include/qt/qbrush.h
about.o: /usr/X11R6/include/qt/qpntarry.h /usr/X11R6/include/qt/qwmatrix.h
about.o: /usr/X11R6/include/qt/qpixmap.h /usr/X11R6/include/qt/qimage.h
about.o: /usr/X11R6/include/qt/qstrlist.h /usr/X11R6/include/qt/qdstream.h
about.o: /usr/X11R6/include/qt/qiodev.h /usr/X11R6/include/qt/qtimer.h
about.o: /usr/X11R6/include/qt/qaccel.h /usr/X11R6/include/qt/qkeycode.h
about.o: /usr/include/math.h /usr/include/huge_val.h /usr/include/endian.h
about.o: /usr/include/bytesex.h /usr/include/nan.h
about.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
about.o: /usr/include/values.h /usr/include/ieee754.h /usr/include/ieee854.h
fftview.o: fftview.h /usr/X11R6/include/qt/qapp.h
fftview.o: /usr/X11R6/include/qt/qwidget.h /usr/X11R6/include/qt/qwindefs.h
fftview.o: /usr/X11R6/include/qt/qobjdefs.h /usr/X11R6/include/qt/qglobal.h
fftview.o: /usr/X11R6/include/qt/qobject.h /usr/X11R6/include/qt/qstring.h
fftview.o: /usr/X11R6/include/qt/qarray.h /usr/X11R6/include/qt/qgarray.h
fftview.o: /usr/X11R6/include/qt/qshared.h /usr/X11R6/include/qt/qgeneric.h
fftview.o: /usr/include/string.h /usr/include/features.h
fftview.o: /usr/include/sys/cdefs.h
fftview.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
fftview.o: /usr/X11R6/include/qt/qevent.h /usr/X11R6/include/qt/qrect.h
fftview.o: /usr/X11R6/include/qt/qsize.h /usr/X11R6/include/qt/qpoint.h
fftview.o: /usr/X11R6/include/qt/qpaintd.h /usr/X11R6/include/qt/qpalette.h
fftview.o: /usr/X11R6/include/qt/qcolor.h /usr/X11R6/include/qt/qcursor.h
fftview.o: /usr/X11R6/include/qt/qfont.h /usr/X11R6/include/qt/qfontmet.h
fftview.o: /usr/X11R6/include/qt/qfontinf.h /usr/X11R6/include/qt/qpushbt.h
fftview.o: /usr/X11R6/include/qt/qbutton.h /usr/X11R6/include/qt/qpainter.h
fftview.o: /usr/X11R6/include/qt/qregion.h /usr/X11R6/include/qt/qpen.h
fftview.o: /usr/X11R6/include/qt/qbrush.h /usr/X11R6/include/qt/qpntarry.h
fftview.o: /usr/X11R6/include/qt/qwmatrix.h /opt/kde/include/kapp.h
fftview.o: /opt/kde/include/kconfig.h /opt/kde/include/kconfigbase.h
fftview.o: /usr/X11R6/include/qt/qstrlist.h /usr/X11R6/include/qt/qlist.h
fftview.o: /usr/X11R6/include/qt/qglist.h /usr/X11R6/include/qt/qcollect.h
fftview.o: /usr/X11R6/include/qt/qdstream.h /usr/X11R6/include/qt/qiodev.h
fftview.o: /opt/kde/include/kconfigdata.h /usr/X11R6/include/qt/qdict.h
fftview.o: /usr/X11R6/include/qt/qgdict.h /usr/X11R6/include/qt/qtstream.h
fftview.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
fftview.o: /usr/X11R6/include/qt/qfile.h /opt/kde/include/kdebug.h
fftview.o: /opt/kde/include/klocale.h /usr/X11R6/include/qt/qintdict.h
fftview.o: /opt/kde/include/drag.h /usr/X11R6/include/qt/qpixmap.h
fftview.o: /usr/include/X11/X.h /usr/X11R6/include/qt/qpopmenu.h
fftview.o: /usr/X11R6/include/qt/qtablevw.h /usr/X11R6/include/qt/qframe.h
fftview.o: /usr/X11R6/include/qt/qmenudta.h /opt/kde/include/kslider.h
fftview.o: /usr/X11R6/include/qt/qslider.h /usr/X11R6/include/qt/qrangect.h
fftview.o: /opt/kde/include/kselect.h /usr/X11R6/include/qt/qdialog.h
fftview.o: /usr/X11R6/include/qt/qlined.h /opt/kde/include/ktopwidget.h
fftview.o: /usr/include/stdlib.h /usr/include/errno.h
fftview.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
fftview.o: /usr/include/alloca.h /opt/kde/include/ktoolbar.h
fftview.o: /opt/kde/include/kbutton.h /opt/kde/include/kcombo.h
fftview.o: /usr/X11R6/include/qt/qlistbox.h /usr/X11R6/include/qt/qcombo.h
fftview.o: /opt/kde/include/klined.h /opt/kde/include/kmenubar.h
fftview.o: /usr/X11R6/include/qt/qmenubar.h /opt/kde/include/kstatusbar.h
fftview.o: /usr/X11R6/include/qt/qlabel.h /opt/kde/include/kbuttonbox.h
fftview.o: /usr/X11R6/include/qt/qtimer.h gsl_fft.h /usr/include/math.h
fftview.o: /usr/include/huge_val.h /usr/include/endian.h
fftview.o: /usr/include/bytesex.h /usr/include/nan.h
fftview.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
fftview.o: /usr/include/values.h /usr/include/ieee754.h
fftview.o: /usr/include/ieee854.h /usr/include/limits.h
fftview.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
fftview.o: /usr/include/posix2_lim.h
gsl_fft.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/stddef.h
gsl_fft.o: /usr/include/stdlib.h /usr/include/features.h
gsl_fft.o: /usr/include/sys/cdefs.h /usr/include/errno.h
gsl_fft.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
gsl_fft.o: /usr/include/alloca.h /usr/include/stdio.h /usr/include/libio.h
gsl_fft.o: /usr/include/_G_config.h /usr/include/math.h
gsl_fft.o: /usr/include/huge_val.h /usr/include/endian.h
gsl_fft.o: /usr/include/bytesex.h /usr/include/nan.h
gsl_fft.o: /usr/lib/gcc-lib/i486-linux/2.7.2.1/include/float.h
gsl_fft.o: /usr/include/values.h /usr/include/ieee754.h
gsl_fft.o: /usr/include/ieee854.h /usr/include/time.h /usr/include/sys/time.h
gsl_fft.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
gsl_fft.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
gsl_fft.o: /usr/include/linux/time.h /usr/include/sys/types.h
gsl_fft.o: /usr/include/sys/bitypes.h gsl_fft.h
