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
INCLUDES= @all_includes@

BUILTSOURCES=\
 TimeLine.moc.cpp\
 FloatLine.moc.cpp\
 MultiStateWidget.moc.cpp\
 ScaleWidget.moc.cpp\
 CornerPatchWidget.moc.cpp\
 CurveWidget.moc.cpp\
 Dialog.moc.cpp\
 FaderWidget.moc.cpp\
 FFTWidget.moc.cpp\
 FormantWidget.moc.cpp\
 OverViewWidget.moc.cpp\
 MenuNode.moc.cpp\
 MenuGroup.moc.cpp \
 MenuItem.moc.cpp\
 MenuSub.moc.cpp\
 MenuToplevel.moc.cpp\
 MenuRoot.moc.cpp\
 MenuManager.moc.cpp

# Which sources should be compiled for kwave.
libkwavegui_la_SOURCES =\
 CurveWidget.cpp\
 FaderWidget.cpp\
 FFTWidget.cpp\
 FloatLine.cpp\
 FormantWidget.cpp\
 Dialog.cpp\
 MenuNode.cpp\
 MenuGroup.cpp \
 MenuItem.cpp\
 MenuSub.cpp\
 MenuToplevel.cpp\
 MenuRoot.cpp\
 MenuManager.cpp\
 MultiStateWidget.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 CornerPatchWidget.cpp\
 TimeLine.cpp\
 $(BUILTSOURCES)

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

Dialog.moc.cpp: Dialog.h
	$(MOC) $< -o $@
MultiStateWidget.moc.cpp: MultiStateWidget.h
	$(MOC) $< -o $@
ScaleWidget.moc.cpp: ScaleWidget.h
	$(MOC) $< -o $@
CornerPatchWidget.moc.cpp: CornerPatchWidget.h
	$(MOC) $< -o $@
OverViewWidget.moc.cpp: OverViewWidget.h
	$(MOC) $< -o $@
FFTWidget.moc.cpp: FFTWidget.h
	$(MOC) $< -o $@
FaderWidget.moc.cpp: FaderWidget.h
	$(MOC) $< -o $@
CurveWidget.moc.cpp: CurveWidget.h
	$(MOC) $< -o $@
FormantWidget.moc.cpp: FormantWidget.h
	$(MOC) $< -o $@
MenuNode.moc.cpp: MenuNode.h
	$(MOC) $< -o $@
MenuGroup.moc.cpp: MenuGroup.h
	$(MOC) $< -o $@
MenuItem.moc.cpp: MenuItem.h
	$(MOC) $< -o $@
MenuSub.moc.cpp: MenuSub.h
	$(MOC) $< -o $@
MenuToplevel.moc.cpp: MenuToplevel.h
	$(MOC) $< -o $@
MenuRoot.moc.cpp: MenuRoot.h
	$(MOC) $< -o $@
MenuManager.moc.cpp: MenuManager.h
	$(MOC) $< -o $@
FloatLine.moc.cpp: FloatLine.h
	$(MOC) $< -o $@
TimeLine.moc.cpp: TimeLine.h
	$(MOC) $< -o $@
