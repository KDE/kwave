# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable

# set the include path for X, qt and KDE
INCLUDES= -I${topdir} @all_includes@

BUILTSOURCES=\
 TimeLine.moc.cpp\
 FloatLine.moc.cpp\
 MultiStateWidget.moc.cpp\
 CornerPatchWidget.moc.cpp\
 CurveWidget.moc.cpp\
 Dialog.moc.cpp\
 FaderWidget.moc.cpp\
 FFTWidget.moc.cpp\
 FileProgress.moc.cpp\
 FormantWidget.moc.cpp\
 KwavePlugin.moc.cpp \
 OverViewWidget.moc.cpp\
 MenuNode.moc.cpp\
 MenuGroup.moc.cpp \
 MenuItem.moc.cpp\
 MenuSub.moc.cpp\
 MenuRoot.moc.cpp\
 MenuManager.moc.cpp

# Which sources should be compiled for libgui.
libkwavegui_la_SOURCES =\
 CurveWidget.cpp\
 FaderWidget.cpp\
 FFTWidget.cpp\
 FileProgress.cpp\
 FloatLine.cpp\
 FormantWidget.cpp\
 Dialog.cpp\
 KwavePlugin.cpp \
 MenuNode.cpp\
 MenuGroup.cpp \
 MenuItem.cpp\
 MenuSub.cpp\
 MenuRoot.cpp\
 MenuManager.cpp\
 MultiStateWidget.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 CornerPatchWidget.cpp\
 TimeLine.cpp\
 $(BUILTSOURCES)

#noinst_LIBRARIES = libkwavegui.a
libkwavegui_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

%moc.cpp: %h
	${MOC} $< -o $@

# end of Makefile.am
