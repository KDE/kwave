
INCLUDES= -I${topdir} @all_includes@

BUILTSOURCES=\
 ConfirmCancelProxy.moc.cpp\
 TimeLine.moc.cpp\
 FloatLine.moc.cpp\
 MultiStateWidget.moc.cpp\
 CurveWidget.moc.cpp\
 Dialog.moc.cpp\
 FFTWidget.moc.cpp\
 FileProgress.moc.cpp\
 FormantWidget.moc.cpp\
 KwaveFileDialog.moc.cpp\
 KwavePlugin.moc.cpp \
 OverViewCache.moc.cpp\
 OverViewWidget.moc.cpp\
 MenuNode.moc.cpp\
 MenuGroup.moc.cpp \
 MenuItem.moc.cpp\
 MenuSub.moc.cpp\
 MenuRoot.moc.cpp\
 MenuManager.moc.cpp \
 TrackPixmap.moc.cpp

# Which sources should be compiled for libgui.
libkwavegui_la_SOURCES =\
 CurveWidget.cpp\
 ConfirmCancelProxy.cpp\
 FFTWidget.cpp\
 FileProgress.cpp\
 FloatLine.cpp\
 FormantWidget.cpp\
 Dialog.cpp\
 KwaveFileDialog.cpp\
 KwavePlugin.cpp \
 MenuNode.cpp\
 MenuGroup.cpp \
 MenuItem.cpp\
 MenuSub.cpp\
 MenuRoot.cpp\
 MenuManager.cpp\
 MultiStateWidget.cpp\
 OverViewCache.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 TimeLine.cpp\
 TrackPixmap.cpp\
 $(BUILTSOURCES)

#noinst_LIBRARIES = libkwavegui.a
libkwavegui_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

# end of Makefile.am
