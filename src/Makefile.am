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

APPSDIR= $(kde_appsdir)/Multimedia
CONFDIR= $(kde_datadir)/$(PACKAGE)
PIXDIR = $(kde_datadir)/$(PACKAGE)/pics
# set the include path for X, qt and KDE
INCLUDES= @all_includes@

# claim, which subdirectories you want to install

####### This part is very kwave specific
bin_PROGRAMS  = kwave

SINGLE_DIALOG_SRC=dialog_progress.cpp \
 dialog_sweep.cpp dialog_frequency.cpp dialog_fixedfreq.cpp

SINGLE_DIALOGS=mdialog_progress.cpp \
 mdialog_sweep.cpp mdialog_frequency.cpp mdialog_fixedfreq.cpp
 
# Which sources should be compiled for kwave.
kwave_SOURCES =  $(SINGLE_DIALOGS) $(SINGLE_DIALOG_SRC) \
 signalview.cpp main.cpp mainwidget.cpp signalio.cpp signalplay.cpp \
 fftview.cpp addsynth.cpp sonagram.cpp signalmanager.cpp markers.cpp\
 pitchwidget.cpp filterwidget.cpp clipboard.cpp mousemark.cpp\
 mmain.cpp msignalview.cpp mmainwidget.cpp \
 maddsynth.cpp mfftview.cpp msonagram.cpp\
 mmousemark.cpp mfilterwidget.cpp \
 mpitchwidget.cpp msignalmanager.cpp

# the library search path
kwave_LDFLAGS = @all_libraries@ -L$(topdir)/libgui/.libs

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kwave_LDADD   = -lkfm -lkdeui -lkdecore -lkwavegui -lqt -lX11 -lXext \
-lkwave -lpthread -ldl

# just to make sure, automake makes them 
 BUILTSOURCES =	 $(SINGLE_DIALOGS) mmain.cpp msignalview.cpp \
 maddsynth.cpp mdialogs.cpp mfftview.cpp msonagram.cpp\
 mpitchwidget.cpp mmainwidget.cpp mmousemark.cpp msignalmanager.cpp

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* doc/*~

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kwave_SOURCES)

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kwave.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) images/kwave.xpm $(kde_icondir)
	$(mkinstalldirs) $(PIXDIR)
	$(INSTALL_DATA) images/light_on.xpm $(PIXDIR)
	$(INSTALL_DATA) images/light_off.xpm $(PIXDIR)
	$(INSTALL_DATA) images/logo.xpm $(PIXDIR)
	$(INSTALL_DATA) images/knob.xpm $(PIXDIR)
	$(INSTALL_DATA) images/selectedknob.xpm $(PIXDIR)
	$(INSTALL_DATA) images/xspeaker.xpm $(PIXDIR)
	$(INSTALL_DATA) images/rspeaker.xpm $(PIXDIR)
	$(INSTALL_DATA) images/lspeaker.xpm $(PIXDIR)
	$(INSTALL_DATA) images/font.xpm $(PIXDIR)
	$(mkinstalldirs) $(CONFDIR)
	$(INSTALL_DATA) menus.config $(CONFDIR)
	$(mkinstalldirs) $(kde_htmldir)/default/$(PACKAGE)
	$(INSTALL_DATA) doc/* $(kde_htmldir)/default/$(PACKAGE)

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kwave.kdelnk
	-rm -f $(kde_icondir)/kwave.xpm

# add a dependency for every moc file to be full portable

main.cpp: mmain.cpp
mmain.cpp: main.h
	$(MOC) -o mmain.cpp main.h
msignalview.cpp: signalview.h
	$(MOC) -o msignalview.cpp signalview.h
mmainwidget.cpp: mainwidget.h
	$(MOC) -o mmainwidget.cpp mainwidget.h
maddsynth.cpp: addsynth.h
	$(MOC) -o maddsynth.cpp addsynth.h
mfilterwidget.cpp: filterwidget.h
	$(MOC) -o mfilterwidget.cpp filterwidget.h
mfftview.cpp: fftview.h
	$(MOC) -o mfftview.cpp fftview.h
msonagram.cpp: sonagram.h
	$(MOC) -o msonagram.cpp sonagram.h
mmousemark.cpp: mousemark.h
	$(MOC) -o mmousemark.cpp mousemark.h
mpitchwidget.cpp: pitchwidget.h
	$(MOC) -o mpitchwidget.cpp pitchwidget.h
msignalmanager.cpp: signalmanager.h
	$(MOC) -o msignalmanager.cpp signalmanager.h
#single dialogs
mdialog_progress.cpp: dialog_progress.h
	$(MOC) -o mdialog_progress.cpp dialog_progress.h
mdialog_frequency.cpp: dialog_frequency.h
	$(MOC) -o mdialog_frequency.cpp dialog_frequency.h
mdialog_sweep.cpp: dialog_sweep.h
	$(MOC) -o mdialog_sweep.cpp dialog_sweep.h
mdialog_fixedfreq.cpp: dialog_fixedfreq.h
	$(MOC) -o mdialog_fixedfreq.cpp dialog_fixedfreq.h












