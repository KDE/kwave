############################################################################
#    Kwave                - plugins/common/Makefile.am
#			     -------------------
#    begin                : Sat Nov 11 2001
#    copyright            : (C) 2000 by Thomas Eschenbacher
#    email                : Thomas.Eschenbacher@gmx.de
############################################################################

############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################
#
# generic common Makefile.am for all Kwave plugins
#

.NOTPARALLEL: right_order

SUFFIXES= .h .c .cpp .o .d .moc .dm .ui .cc .idl .cxx

# produced errors with SuSE 8.0's automake ???
# DISTCLEANFILES = Makefile.in Makefile

-include Makefile.plugin

# set the include path for X, qt and KDE
INCLUDES  = -I${topdir} @all_includes@ @ARTS_INCLUDES@

# Which sources should be compiled for kwave.
PLUGINDIR=$(DESTDIR)$(kde_datadir)/$(PACKAGE)/plugins

CXXFLAGS  = @CXXFLAGS@ -fPIC
LD_FLAGS  = -shared -fPIC -rdynamic

MODDIR:=$(shell pwd)
MODNAME=$(shell basename $(MODDIR))

ui_files	:= $(shell @FIND@ . -name \*.ui | @SORT@)
uih_files	:= $(ui_files:.ui=.uih.h)
uic_files	:= $(ui_files:.ui=.uic.cpp)
	
h_files		:= $(shell @FIND@ . ! -name \*.uih.h -name \*.h | @SORT@)
h_files   	+= ${uih_files}
cpp_sources 	:= $(shell @FIND@ . ! -name \*.uic.cpp -name \*.cpp | @SORT@)
c_sources 	:= $(shell @FIND@ . -name \*.c | @SORT@)
idl_files	:= $(shell @FIND@ . -name \*.idl | @SORT@)
idl_header	:= $(idl_files:.idl=.h)
idl_impl	:= $(idl_files:.idl=.cc)

moc_files	:= $(h_files:.h=.moc.cxx)

OUT_FILES	:= $(cpp_sources:.cpp=.o)
OUT_FILES	+= $(c_sources:.c=.o)
OUT_FILES	+= $(uic_files:.uic.cpp=.uic.o)
OUT_FILES	+= $(h_files:.h=.moc.o)
OUT_FILES	+= $(idl_files:.idl=.o)

PLUGINDEP_FILES	:= $(h_files:.h=.dm)
PLUGINDEP_FILES	+= $(cpp_sources:.cpp=.d)
PLUGINDEP_FILES	+= $(c_sources:.c=.d)
PLUGINDEP_FILES	+= $(uic_sources:.cpp=.d)

h_files  	+= ${idl_header}

GENERATED_FILES := ${uih_files} ${uic_files} ${idl_header} ${idl_impl} ${moc_files}

CLEANFILES	 = core *~ *.o ${MODNAME} *.moc.cxx *.moc *.d *.dm \
			.depend *.uih.h *.uic.cpp \
			${GENERATED_FILES}

%.uic.o: %.uic.cpp
	@echo plugin \"$(MODNAME)\": compiling dialog "'"$<"'"
	@ @CXX@ $(CXXFLAGS)  ${INCLUDES} -c $< -o $@

%.o: %.cpp
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.o: %.c
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CC@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.o: %.cc
	@echo plugin \"$(MODNAME)\": compiling idl implementation "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -c $< -o $@

%.moc.o: %.moc.cxx
	@echo plugin \"$(MODNAME)\": compiling "'"$<"'"
	@ @CXX@ $(CXXFLAGS) ${INCLUDES} -x c++ -c $< -o $@
	@ echo "" >> $@

%.cc: %.idl
	@echo plugin \"$(MODNAME)\": creating idl implementation "'"$<"'"
	@ ${MCOPIDL} @ARTS_INCLUDES@ $<

%.h: %.idl
	@echo plugin \"$(MODNAME)\": creating idl header "'"$<"'"
	@ ${MCOPIDL} @ARTS_INCLUDES@ $<

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
	@ echo "" >> $@

%.d: %.c
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ echo -n "$@ " > $@
	@ @CC@ -M ${INCLUDES} $(CXXFLAGS) $< >> $@
	@ echo "" >> $@

%.dm: %.moc.cxx
	@echo plugin \"$(MODNAME)\": making dependencies for $<
	@ echo -n "$@ " > $@
	@ @CXX@ -M ${INCLUDES} $(CXXFLAGS) -x c++ $< >> $@
	@ echo "" >> $@

%.moc.cxx: %.h
	@echo plugin \"$(MODNAME)\": extracting code with MOC for $<
	@ @MOC@ -nw $< -o $@

.depend: ${PLUGINDEP_FILES}
	@echo plugin \"$(MODNAME)\": merging dependencies into $@
	@ cat ${PLUGINDEP_FILES} > $@

#
# include dependency file if it exists
#
## DO NOT DELETE THE LEADING SPACE IN FRONT OF THE FOLLOWING THREE    !!
## LINES. THIS WOULD PRODUCE AN ERROR WITH AUTOMAKE (endif not found) !!
 ifneq ($(wildcard .depend),)
 include .depend
 endif

dep depend: .depend

$(MODNAME): ${OUT_FILES}
	@echo plugin \"$(MODNAME)\": linking \"${OUT_FILES}\" to \"$@\"
	@ @CXX@ $(CXXFLAGS) ${OUT_FILES} ${LD_FLAGS} \
	  ${EXTRA_LIBDIRS} ${EXTRA_LDFLAGS} -o ${MODNAME}

right_order: ${GENERATED_FILES} depend ${OUT_FILES} $(MODNAME)

all: right_order

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = ${CLEANFILES} .deps/* `@FIND@ . -type l` *.moc *.moc.cxx

install-data-local:
	$(mkinstalldirs) $(PLUGINDIR)
	$(INSTALL) -m 755 $(MODNAME) $(PLUGINDIR)

uninstall-local:
	-rm -f $(PLUGINDIR)/$(MODNAME)
	-rmdir -p --ignore-fail-on-non-empty $(PLUGINDIR) >/dev/null 2>&1
