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

APPSDIR= $(DESTDIR)$(kde_appsdir)/Multimedia
CONFDIR= $(DESTDIR)$(kde_datadir)/$(PACKAGE)
PIXDIR = $(DESTDIR)$(kde_datadir)/$(PACKAGE)/pics
# set the include path for X, qt and KDE
INCLUDES= -I${topdir} @all_includes@
ICONDIR = $(DESTDIR)$(kde_icondir)

bin_PROGRAMS  = kwave

# specifying the bindir is necessary in order to let make install
# use the correct directory when building a binary rpm package
bindir=$(prefix)/bin

# conversion to modules to do for the following files:
# mdialog_sweep.cpp mdialog_frequency.cpp mdialog_fixedfreq.cpp addsynth.h

# Which sources should be compiled for kwave.

BUILTSOURCES=\
 FFTContainer.moc.cpp\
 FFTWindow.moc.cpp\
 ImageView.moc.cpp\
 LabelManager.moc.cpp\
 KwaveApp.moc.cpp\
 MainWidget.moc.cpp\
 PitchContainer.moc.cpp\
 PitchWidget.moc.cpp\
 PitchWindow.moc.cpp\
 PlaybackController.moc.cpp \
 PluginManager.moc.cpp\
 SignalManager.moc.cpp\
 SignalWidget.moc.cpp\
 TopWidget.moc.cpp\
 UndoInsertAction.moc.cpp

kwave_SOURCES =\
 main.cpp\
 ClipBoard.cpp\
 FFTContainer.cpp\
 FFTWindow.cpp\
 ImageView.cpp\
 KwaveApp.cpp\
 LabelManager.cpp\
 MainWidget.cpp\
 MemoryManager.cpp\
 MouseMark.cpp\
 PitchContainer.cpp\
 PitchWidget.cpp\
 PitchWindow.cpp\
 PlaybackController.cpp\
 PluginManager.cpp\
 SignalManager.cpp\
 SignalWidget.cpp\
 SwapFile.cpp\
 TopWidget.cpp\
 UndoDeleteAction.cpp\
 UndoDeleteTrack.cpp\
 UndoInsertAction.cpp\
 UndoInsertTrack.cpp\
 UndoModifyAction.cpp\
 UndoSelection.cpp\
 UndoTransaction.cpp\
 UndoTransactionGuard.cpp\
 $(BUILTSOURCES)

# extra data, icons etc
ICONS = \
 pics/$(PACKAGE)-16x16.png \
 pics/$(PACKAGE)-22x22.png \
 pics/$(PACKAGE)-32x32.png \
 pics/$(PACKAGE)-48x48.png

# some pixmaps to be installed in the data dir
PIXMAPS=\
 delete.xpm\
 light_on.xpm\
 light_off.xpm\
 logo.xpm\
 knife.xpm\
 knob.xpm\
 selectedknob.xpm\
 xspeaker.xpm\
 rspeaker.xpm\
 lspeaker.xpm\
 font.xpm\
 exit.xpm\
 filesave.xpm

# the library search path
kwave_LDFLAGS = -L../mt -L../libkwave -L../libgui \
	@ARTS_LIBS@ @all_libraries@ -export-dynamic -dy

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kwave_LDADD   =  -lkwavegui -lkwave -lkwavemt -lkfile -lkdeui -lkdecore -lqt \
	-lX11 -lXext -lpthread -ldl -lm

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* doc/*~ Makefile.in Makefile

$(APPSDIR)/kwave.desktop:

# holds the content of menus.config for internationalization
menus_config_i18n.cpp: menus.config
	${topdir}/bin/menusconfig2cpp.sh menus.config menus_config_i18n.cpp

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kwave_SOURCES)

#
# Rules for re-creating all icons in their various sizes

pics/$(PACKAGE)-16x16.png: pics/$(PACKAGE).xpm
	convert -geometry 16x16 $< $@

pics/$(PACKAGE)-22x22.png: pics/$(PACKAGE).xpm
	convert -geometry 22x22 $< $@

pics/$(PACKAGE)-32x32.png: pics/$(PACKAGE).xpm
	convert -geometry 32x32 $< $@

pics/$(PACKAGE)-48x48.png: pics/$(PACKAGE).xpm
	convert -geometry 48x48 $< $@

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 

install-data-local: $(ICONS)
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kwave.desktop $(APPSDIR)/kwave.desktop
	$(mkinstalldirs) $(PIXDIR)
	for pix in $(PIXMAPS); do { $(INSTALL_DATA) pics/$$pix $(PIXDIR) ; }; done;
	$(mkinstalldirs) $(CONFDIR)
	$(INSTALL_DATA) menus.config $(CONFDIR)/menus.config
	for col in locolor hicolor; do { \
	  for dim in 16x16 22x22 32x32 48x48; do { \
	    $(mkinstalldirs) $(ICONDIR)/$$col/$$dim/apps/; \
	    $(INSTALL_DATA) pics/${PACKAGE}-$$dim.png $(ICONDIR)/$$col/$$dim/apps/$(PACKAGE).png; \
	  }; done; \
	}; done;

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	for col in locolor hicolor; do { \
	  for dim in 16x16 22x22 32x32 48x48; do { \
	    rm -f $(ICONDIR)/$$col/$$dim/apps/$(PACKAGE).png || true; \
	    (rmdir -p --ignore-fail-on-non-empty $(ICONDIR)/$$col/$$dim/apps/ >/dev/null 2>&1); \
	  }; done; \
	}; done;
	for pix in $(PIXMAPS); do { rm -fv $(PIXDIR)/$$pix ; }; done;
	-rm -f $(APPSDIR)/kwave.desktop
	-rm -f $(CONFDIR)/menus.config
	-rmdir -p --ignore-fail-on-non-empty $(APPSDIR) >/dev/null 2>&1
	-rmdir -p --ignore-fail-on-non-empty $(CONFDIR) >/dev/null 2>&1
	-rmdir -p --ignore-fail-on-non-empty $(PIXDIR) >/dev/null 2>&1
	-rmdir -p --ignore-fail-on-non-empty $(DESTDIR)$(bindir) >/dev/null 2>&1

# dependencies for moc files
%moc.cpp: %h
	${MOC} -o $@ $<

