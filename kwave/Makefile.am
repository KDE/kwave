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
INCLUDES= -I${topdir} @all_includes@

# claim, which subdirectories you want to install

####### This part is very kwave specific
bin_PROGRAMS  = kwave

# specifying the bindir is necessary in order to let make install
# use the correct directory when building a binary rpm package
bindir=$(prefix)/bin
# bindir=@bindir@ <- does not work, why ???

# conversion to modules to do for the following files:
# mdialog_sweep.cpp mdialog_frequency.cpp mdialog_fixedfreq.cpp addsynth.h

# Which sources should be compiled for kwave.

BUILTSOURCES=\
 FFTContainer.moc.cpp\
 FFTWindow.moc.cpp\
 ImageView.moc.cpp\
 LabelManager.moc.cpp\
 MouseMark.moc.cpp\
 KwaveApp.moc.cpp\
 MainWidget.moc.cpp\
 PitchContainer.moc.cpp\
 PitchWidget.moc.cpp\
 PitchWindow.moc.cpp\
 PlaybackController.moc.cpp \
 PluginManager.moc.cpp\
 SignalManager.moc.cpp\
 SignalWidget.moc.cpp\
 TopWidget.moc.cpp

kwave_SOURCES =\
 main.cpp\
 ClipBoard.cpp\
 FFTContainer.cpp\
 FFTWindow.cpp\
 ImageView.cpp\
 KwaveApp.cpp\
 LabelManager.cpp\
 MainWidget.cpp\
 MouseMark.cpp\
 PitchContainer.cpp\
 PitchWidget.cpp\
 PitchWindow.cpp\
 PlaybackController.cpp\
 PluginManager.cpp\
 SignalManager.cpp\
 SignalWidget.cpp\
 TopWidget.cpp\
 $(BUILTSOURCES)

# the library search path
# kwave_LDFLAGS = -dy -L$(topdir)/libgui/.libs @all_libraries@ --export-dynamic
kwave_LDFLAGS = -L$(topdir)/libkwave -L$(topdir)/libgui @all_libraries@ -dy -export-dynamic

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kwave_LDADD   = $(topdir)/mt/libkwavemt.a -lkwavegui -lkwave -lkfile \
	-lkdeui -lkdecore -lqt -lX11 -lXext -lpthread -ldl

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* doc/*~

# holds the content of menus.config for internationalization
menus_config_i18n.cpp: menus.config
	${topdir}/bin/menusconfig2cpp.sh menus.config menus_config_i18n.cpp

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
	$(INSTALL_DATA) images/exit.xpm $(PIXDIR)
	$(INSTALL_DATA) images/filesave.xpm $(PIXDIR)
	$(mkinstalldirs) $(CONFDIR)
	$(INSTALL_DATA) menus.config $(CONFDIR)

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kwave.kdelnk
	-rm -f $(kde_icondir)/kwave.xpm

# dependencies for moc files
%moc.cpp: %h
	${MOC} -o $@ $<

