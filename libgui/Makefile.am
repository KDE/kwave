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
libkwavegui_la_SOURCES =guiitems.cpp multistateimage.cpp scale.cpp \
curvewidget.cpp kwavedialog.cpp faderwidget.cpp fftwidget.cpp \
formantwidget.cpp overview.cpp kwavemenu.cpp\
mguiitems.cpp mmultistateimage.cpp mscale.cpp mcurvewidget.cpp \
mkwavedialog.cpp mfaderwidget.cpp mfftwidget.cpp mformantwidget.cpp \
moverview.cpp mkwavemenu.cpp

BUILTSOURCES=mguiitems.cpp mmultistateimage.cpp mscale.cpp mcurvewidget.cpp \
mfaderwidget.cpp mfftwidget.cpp mformantwidget.cpp moverview.cpp

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

mguiitems.cpp: guiitems.h
	$(MOC) guiitems.h -o mguiitems.cpp
mkwavedialog.cpp: kwavedialog.h
	$(MOC) kwavedialog.h -o mkwavedialog.cpp
mmultistateimage.cpp: multistateimage.h
	$(MOC) multistateimage.h -o mmultistateimage.cpp
mscale.cpp: scale.h
	$(MOC) scale.h -o mscale.cpp
moverview.cpp: overview.h
	$(MOC) overview.h -o moverview.cpp
mfftwidget.cpp: fftwidget.h
	$(MOC) fftwidget.h -o mfftwidget.cpp
mfaderwidget.cpp: faderwidget.h
	$(MOC) faderwidget.h -o mfaderwidget.cpp
mcurvewidget.cpp: curvewidget.h
	$(MOC) curvewidget.h -o mcurvewidget.cpp
mformantwidget.cpp: formantwidget.h
	$(MOC) formantwidget.h -o mformantwidget.cpp
mkwavemenu.cpp: kwavemenu.h
	$(MOC) kwavemenu.h -o mkwavemenu.cpp





