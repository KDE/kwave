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

# Which sources should be compiled for kwave.
libkwavegui_la_SOURCES =\
 CurveWidget.cpp\
 FaderWidget.cpp\
 FFTWidget.cpp\
 FloatLine.cpp\
 FormantWidget.cpp\
 Dialog.cpp\
 Menu.cpp\
 MenuManager.cpp\
 MenuCommand.cpp\
 MultiStateImage.cpp\
 NumberedMenu.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 TimeLine.cpp\
 mTimeLine.cpp\
 mFloatLine.cpp\
 mMultiStateImage.cpp\
 mScaleWidget.cpp\
 mCurveWidget.cpp\
 mDialog.cpp\
 mFaderWidget.cpp\
 mFFTWidget.cpp\
 mFormantWidget.cpp\
 mOverViewWidget.cpp\
 mMenu.cpp\
 mMenuManager.cpp

BUILTSOURCES=\
 mTimeLine.cpp\
 mFloatLine.cpp\
 mMultiStateImage.cpp\
 mScaleWidget.cpp\
 mCurveWidget.cpp\
 mDialog.cpp\
 mFaderWidget.cpp\
 mFFTWidget.cpp\
 mFormantWidget.cpp\
 mOverViewWidget.cpp\
 mMenu.cpp\
 mMenuManager.cpp

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

mDialog.cpp: Dialog.h
	$(MOC) Dialog.h -o mDialog.cpp
mMultiStateImage.cpp: MultiStateImage.h
	$(MOC) MultiStateImage.h -o mMultiStateImage.cpp
mScaleWidget.cpp: ScaleWidget.h
	$(MOC) ScaleWidget.h -o mScaleWidget.cpp
mOverViewWidget.cpp: OverViewWidget.h
	$(MOC) OverViewWidget.h -o mOverViewWidget.cpp
mFFTWidget.cpp: FFTWidget.h
	$(MOC) FFTWidget.h -o mFFTWidget.cpp
mFaderWidget.cpp: FaderWidget.h
	$(MOC) FaderWidget.h -o mFaderWidget.cpp
mCurveWidget.cpp: CurveWidget.h
	$(MOC) CurveWidget.h -o mCurveWidget.cpp
mFormantWidget.cpp: FormantWidget.h
	$(MOC) FormantWidget.h -o mFormantWidget.cpp
mMenu.cpp: Menu.h
	$(MOC) Menu.h -o mMenu.cpp
mMenuManager.cpp: MenuManager.h
	$(MOC) -o mMenuManager.cpp MenuManager.h 
mFloatLine.cpp: FloatLine.h
	$(MOC) FloatLine.h -o mFloatLine.cpp
mTimeLine.cpp: TimeLine.h
	$(MOC) TimeLine.h -o mTimeLine.cpp



