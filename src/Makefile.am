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



# conversion to modules to do for the following files:
# mdialog_sweep.cpp mdialog_frequency.cpp mdialog_fixedfreq.cpp addsynth.h

# Which sources should be compiled for kwave.

BUILTSOURCES=\
 mFFTContainer.cpp\
 mFFTWindow.cpp\
 mImageView.cpp\
 mMouseMark.cpp\
 mKwaveApp.cpp\
 mMainWidget.cpp\
 mPitchContainer.cpp\
 mPitchWidget.cpp\
 mPitchWindow.cpp\
 mProgressDialog.cpp\
 mSignalWidget.cpp\
 mSonagramContainer.cpp\
 mSonagramWindow.cpp\
 mTopWidget.cpp

kwave_SOURCES =\
 main.cpp\
 ClipBoard.cpp\
 FFTContainer.cpp\
 FFTWindow.cpp\
 ImageView.cpp\
 KwaveApp.cpp\
 MainWidget.cpp\
 MouseMark.cpp\
 PitchContainer.cpp\
 PitchWidget.cpp\
 PitchWindow.cpp\
 ProgressDialog.cpp\
 SignalManager.cpp\
 SignalWidget.cpp\
 SonagramContainer.cpp\
 SonagramWindow.cpp\
 TopWidget.cpp\
 $(BUILTSOURCES)

# the library search path
kwave_LDFLAGS = @all_libraries@ -L$(topdir)/libgui/.libs

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kwave_LDADD   = -lkfm -lkdeui -lkdecore -lkwavegui -lqt -lX11 -lXext \
-lkwave -lpthread -ldl

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

mFFTWindow.cpp: FFTWindow.h
	$(MOC) -o mFFTWindow.cpp FFTWindow.h
mFFTContainer.cpp: FFTContainer.h
	$(MOC) -o mFFTContainer.cpp FFTContainer.h
mImageView.cpp: ImageView.h
	$(MOC) -o mImageView.cpp ImageView.h
mKwaveApp.cpp: KwaveApp.h
	$(MOC) -o mKwaveApp.cpp KwaveApp.h
mMainWidget.cpp: MainWidget.h
	$(MOC) -o mMainWidget.cpp MainWidget.h
mMouseMark.cpp: MouseMark.h
	$(MOC) -o mMouseMark.cpp MouseMark.h
mPitchWidget.cpp: PitchWidget.h
	$(MOC) -o mPitchWidget.cpp PitchWidget.h
mPitchContainer.cpp: PitchContainer.h
	$(MOC) -o mPitchContainer.cpp PitchContainer.h
mPitchWindow.cpp: PitchWindow.h
	$(MOC) -o mPitchWindow.cpp PitchWindow.h
mProgressDialog.cpp: ProgressDialog.h
	$(MOC) -o mProgressDialog.cpp ProgressDialog.h
mSignalWidget.cpp: SignalWidget.h
	$(MOC) -o mSignalWidget.cpp SignalWidget.h
mSonagramContainer.cpp: SonagramContainer.h
	$(MOC) -o mSonagramContainer.cpp SonagramContainer.h
mSonagramWindow.cpp: SonagramWindow.h
	$(MOC) -o mSonagramWindow.cpp SonagramWindow.h
mTopWidget.cpp: TopWidget.h
	$(MOC) -o mTopWidget.cpp TopWidget.h

