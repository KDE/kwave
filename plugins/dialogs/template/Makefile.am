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

INCLUDES= @all_includes@ -I${topdir} -I${topdir}/libgui

# Which sources should be compiled for kwave.
PLUGINDIR=$(kde_datadir)/kwave/modules/dialogs

CXXFLAGS  = @CXXFLAGS@ -shared -fPIC -rdynamic -nostartfiles -nostdlib

MODDIR:=$(shell pwd)
MODNAME=$(shell basename $(MODDIR))

SUFFIXES = .h .moc

SOURCES		 = module.cpp module.moc

CLEANFILES	 = *.d *.dm *.moc *.moc.cpp core *~ *.o ${MODNAME}

cpp_sources 	:= $(shell find . -name \*.cpp)
h_files		:= $(shell find . -name \*.h)
moc_files	:= $(h_files:.h=.moc.cpp)

ALL_CPP    	:= ${cpp_sources} ${moc_files}
OUT_FILES	:= $(ALL_CPP:.cpp=.o)

%o: %cpp
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%d: %cpp
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ $(SHELL) -ec '@CXX@ -MM ${INCLUDES} $(CXXFLAGS) $< \
	| sed s^${notdir $*}o^$*d^g > $@; \
	[ -s $@ ] || rm -f $@'

%dm: %moc
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ $(SHELL) -ec '@CXX@ -x c++ -MM ${INCLUDES} $(CXXFLAGS) $< \
	| sed s^${notdir $*}moc\.o^$*dm^g > $@; \
	[ -s $@ ] || rm -f $@'

%moc.cpp: %h
	@echo plugin \"$(MODNAME)\": extracting code with MOC for $<
	@ @MOC@ $< -o $@

$(MODNAME): ${OUT_FILES}
	@echo plugin \"$(MODNAME)\": linking \"$^\" to \"$@\"
	@ @CXX@ $(CXXFLAGS) ${OUT_FILES} -o ${MODNAME}

all: $(MODNAME)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = ${CLEANFILES} .deps/* `find . -type l`

install-data-local: 
	$(mkinstalldirs) $(PLUGINDIR)
	$(INSTALL) -m 755 $(MODNAME) $(PLUGINDIR)
