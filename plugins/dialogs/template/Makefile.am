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


SUFFIXES= .h .cpp .o .d .moc .dm

# set the include path for X, qt and KDE

INCLUDES= @all_includes@ -I${topdir} -I${topdir}/libgui

# Which sources should be compiled for kwave.
PLUGINDIR=$(kde_datadir)/kwave/modules/dialogs

CXXFLAGS  = @CXXFLAGS@ 
LD_FLAGS  = -shared -fPIC -rdynamic -nostartfiles -nostdlib

MODDIR:=$(shell pwd)
MODNAME=$(shell basename $(MODDIR))

CLEANFILES	 = core *~ *.o ${MODNAME} *.moc.cxx *.moc *.d *.dm .depend

cpp_sources 	:= $(shell find . -name \*.cpp)
h_files		:= $(shell find . -name \*.h)
moc_files	:= $(h_files:.h=.moc.cxx)

OUT_FILES	:= $(cpp_sources:.cpp=.o)
OUT_FILES	+= $(h_files:.h=.moc.o)

DEP_FILES	:= $(cpp_sources:.cpp=.d) $(h_files:.h=.dm)

%.o: %.cpp
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.moc.o: %.moc.cxx
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -x c++ -c $< -o $@
	@ echo "" >> $@

%.d: %.cpp
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ echo -n "$@ " > $@
	@ @CXX@ -M ${INCLUDES} $(CXXFLAGS) $< >> $@

%.dm: %.moc.cxx
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ echo -n "$@ " > $@
	@ @CXX@ -M ${INCLUDES} $(CXXFLAGS) -x c++ $< >> $@

%.moc.cxx: %.h
	@echo plugin \"$(MODNAME)\": extracting code with MOC for $<
	@ @MOC@ $< -o $@

        # make the right order: first build the .d/.dm files then the .o
# ${OUT_FILES}: ${DEP_FILES}

.depend: ${DEP_FILES}
	@echo plugin \"$(MODNAME)\": merging dependencies into $@
	@ cat $^ > $@

#
# include dependency file if it exists
#
## DO NOT DELETE THE LEADING SPACE IN FORNT OF THE FOLLOWIND THREE    !!
## LINES. THIS WOULD PRODUCE AN ERROR WITH AUTOMAKE (endif not found) !!
 ifneq ($(wildcard .depend),)
 include .depend
 endif

dep depend: .depend

$(MODNAME): .depend ${moc_files} ${OUT_FILES}
	@echo plugin \"$(MODNAME)\": linking \"${OUT_FILES}\" to \"$@\"
	@ @CXX@ $(CXXFLAGS) ${LD_FLAGS} ${OUT_FILES} -o ${MODNAME}

all: $(MODNAME)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = ${CLEANFILES} .deps/* `find . -type l` *.moc *.moc.cpp

install-data-local: 
	$(mkinstalldirs) $(PLUGINDIR)
	$(INSTALL) -m 755 $(MODNAME) $(PLUGINDIR)
