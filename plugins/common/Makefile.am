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

SUFFIXES= .h .cpp .o .d .moc .dm .ui .cc .idl

# set the include path for X, qt and KDE
INCLUDES  = -I${topdir} @all_includes@ @ARTS_INCLUDES@

# Which sources should be compiled for kwave.
PLUGINDIR=$(DESTDIR)$(kde_datadir)/$(PACKAGE)/plugins

CXXFLAGS  = @CXXFLAGS@ -fPIC
LD_FLAGS  = -shared -fPIC -rdynamic -nostartfiles

MODDIR:=$(shell pwd)
MODNAME=$(shell basename $(MODDIR))

ui_files	:= $(shell find . -name \*.ui)
uih_files	:= $(ui_files:.ui=.uih.h)
uic_files	:= $(ui_files:.ui=.uic.cpp)
	
h_files		:= $(shell find . ! -name \*.uih.h -name \*.h)
h_files   	+= ${uih_files}
cpp_sources 	:= $(shell find . ! -name \*.uic.cpp -name \*.cpp)
idl_files	:= $(shell find . -name \*.idl)
idl_header	:= $(idl_files:.idl=.h)
idl_impl	:= $(idl_files:.idl=.cc)

moc_files	:= $(h_files:.h=.moc.cxx)

OUT_FILES	:= $(cpp_sources:.cpp=.o)
OUT_FILES	+= $(uic_files:.uic.cpp=.uic.o)
OUT_FILES	+= $(h_files:.h=.moc.o)
OUT_FILES	+= $(idl_files:.idl=.o)

DEP_FILES	:= $(h_files:.h=.dm)
DEP_FILES	+= $(cpp_sources:.cpp=.d)
DEP_FILES	+= $(uic_sources:.cpp=.d)

h_files  	+= ${idl_header}

CLEANFILES	 = core *~ *.o ${MODNAME} *.moc.cxx *.moc *.d *.dm \
			.depend *.uih.h *.uic.cpp \
			$(idl_header) $(idl_impl)

DISTCLEANFILES	 = Makefile.in Makefile

%.uic.o: %.uic.cpp
	@echo plugin \"$(MODNAME)\": compiling dialog "'"$<"'"
	@ @CXX@ $(CXXFLAGS)  ${INCLUDES} -c $< -o $@

%.o: %.cpp
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.o: %.cc
	@echo plugin \"$(MODNAME)\": compiling idl implementation "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.moc.o: %.moc.cxx
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -x c++ -c $< -o $@
	@ echo "" >> $@

%.cc: %.idl
	@echo plugin \"$(MODNAME)\": creating idl implementation "'"$<"'"
	@ ${MCOPIDL} -I$(include_dir)/arts $<

%.h: %.idl
	@echo plugin \"$(MODNAME)\": creating idl header "'"$<"'"
	@ ${MCOPIDL} -I$(include_dir)/arts $<

%.uih.h: %.ui
	@echo plugin \"$(MODNAME)\": generating header from $<
	@ @UIC@ -o $@ -tr i18n $<

%.uic.cpp: %.ui
	@echo plugin \"$(MODNAME)\": generating implementation from $<
	@ @UIC@ -o $@ -tr i18n -impl $*.uih.h $<

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
	@ @MOC@ -nw $< -o $@

        # make the right order: first build the .d/.dm files then the .o
# ${OUT_FILES}: ${DEP_FILES}

${idl_header}: ${idl_files}

${idl_impl}: ${idl_files}

.depend: ${uih_files} ${DEP_FILES}
	@echo plugin \"$(MODNAME)\": merging dependencies into $@
	@ cat $^ > $@

#
# include dependency file if it exists
#
## DO NOT DELETE THE LEADING SPACE IN FRONT OF THE FOLLOWING THREE    !!
## LINES. THIS WOULD PRODUCE AN ERROR WITH AUTOMAKE (endif not found) !!
 ifneq ($(wildcard .depend),)
 include .depend
 endif

dep depend: .depend

$(MODNAME): ${uih_files} ${uic_files} ${moc_files} .depend ${OUT_FILES}
	@echo plugin \"$(MODNAME)\": linking \"${OUT_FILES}\" to \"$@\"
	@ @CXX@ $(CXXFLAGS) ${LD_FLAGS} ${OUT_FILES} -o ${MODNAME}

all: $(MODNAME)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = ${CLEANFILES} .deps/* `find . -type l` *.moc *.moc.cpp

install-data-local: 
	$(mkinstalldirs) $(PLUGINDIR)
	$(INSTALL) -m 755 $(MODNAME) $(PLUGINDIR)

uninstall-local: 
	-rm -f $(PLUGINDIR)/$(MODNAME)
	-rmdir -p --ignore-fail-on-non-empty $(PLUGINDIR) >/dev/null 2>&1
