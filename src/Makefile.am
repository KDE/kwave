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

APPSDIR = $(kde_appsdir)/Multimedia
PIXDIR = $(kde_datadir)/$(PACKAGE)/pics
# set the include path for X, qt and KDE
INCLUDES= @all_includes@ 

# claim, which subdirectories you want to install

####### This part is very kwave specific
bin_PROGRAMS  = kwave

# Which sources should be compiled for kwave.
kwave_SOURCES = signalview.cpp main.cpp sample.cpp mainwidget.cpp sampleio.cpp\
 about.cpp fftview.cpp gsl_fft.cpp mmap.cpp dialogs.cpp configdialogs.cpp\
 curvewidget.cpp interpolation.cpp sampleedit.cpp filter.cpp scale.cpp\
 multistateimage.cpp addsynth.cpp functions.cpp sonagram.cpp markers.cpp\
 formantwidget.cpp faderwidget.cpp pitchwidget.cpp windowfunction.cpp\
 menumanager.cpp clipboard.cpp mousemark.cpp guiitems.cpp\
 mmain.cpp msignalview.cpp mmainwidget.cpp msample.cpp mabout.cpp\
 mconfigdialogs.cpp moverview.cpp mguiitems.cpp\
 mformantwidget.cpp maddsynth.cpp mdialogs.cpp mfftview.cpp msonagram.cpp\
 mfaderwidget.cpp mcurvewidget.cpp mmultistateimage.cpp mmousemark.cpp\
 mfilter.cpp mscale.cpp mpitchwidget.cpp mmenumanager.cpp

# the library search path
kwave_LDFLAGS = @all_libraries@

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kwave_LDADD   = -lkfm -lkdeui -lkdecore -lqt -lX11 -lXext -lpthread

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS =  signalview.h mainwidget.h sample.h main.h about.h fftview.h\
 gsl_fft.h faderwidget.h curvewidget.h interpolation.h mousemark.h\
 multistateimage.h addsynth.h functions.h clipboard.h\
 sonagram.h filter.h scale.h menumanager.h overview.h markers.h slider.h\
 $(DIALOGDIR)dialogs.h $(DIALOGDIR)configdialogs.h
# just to make sure, automake makes them 
BUILTSOURCES =	mmain.cpp msignalview.cpp msample.cpp mabout.cpp maddsynth.cpp\
 mconfigdialogs.cpp mdialogs.cpp mfftview.cpp msonagram.cpp mformantwidget.cpp\
 mfaderwidget.cpp mcurvewidget.cpp mmultistateimage.cpp mscale.cpp\
 mpitchwidget.cpp mmenumanager.cpp mmainwidget.cpp moverview.cpp mmarker.cpp\
 mmousemark.cpp

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
mmarkers.cpp: markers.h
	$(MOC) -o mmarkers.cpp markers.h
msignalview.cpp: signalview.h
	$(MOC) -o msignalview.cpp signalview.h
mmainwidget.cpp: mainwidget.h
	$(MOC) -o mmainwidget.cpp mainwidget.h
moverview.cpp: overview.h
	$(MOC) -o moverview.cpp overview.h
msample.cpp: sample.h
	$(MOC) -o msample.cpp sample.h
mabout.cpp: about.h
	$(MOC) -o mabout.cpp about.h
maddsynth.cpp: addsynth.h
	$(MOC) -o maddsynth.cpp addsynth.h
mfilter.cpp: filter.h
	$(MOC) -o mfilter.cpp filter.h
mfftview.cpp: fftview.h
	$(MOC) -o mfftview.cpp fftview.h
msonagram.cpp: sonagram.h
	$(MOC) -o msonagram.cpp sonagram.h
mmousemark.cpp: mousemark.h
	$(MOC) -o mmousemark.cpp mousemark.h
mfaderwidget.cpp: faderwidget.h
	$(MOC) -o mfaderwidget.cpp faderwidget.h
mpitchwidget.cpp: pitchwidget.h
	$(MOC) -o mpitchwidget.cpp pitchwidget.h
mformantwidget.cpp: formantwidget.h
	$(MOC) -o mformantwidget.cpp formantwidget.h
mcurvewidget.cpp: curvewidget.h
	$(MOC) -o mcurvewidget.cpp curvewidget.h
mmultistateimage.cpp: multistateimage.h
	$(MOC) -o mmultistateimage.cpp multistateimage.h 
mmenumanager.cpp: menumanager.h
	$(MOC) -o mmenumanager.cpp menumanager.h 
mscale.cpp: scale.h
	$(MOC) -o mscale.cpp scale.h 
mguiitems.cpp: guiitems.h
	$(MOC) -o mguiitems.cpp guiitems.h
mdialogs.cpp: dialogs.h
	$(MOC) -o mdialogs.cpp dialogs.h
mconfigdialogs.cpp: configdialogs.h
	$(MOC) -o mconfigdialogs.cpp configdialogs.h










