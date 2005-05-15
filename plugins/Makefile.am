############################################################################
#    Kwave                - plugins/Makefile.am
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

# make sure all subdirs get a makefile....
dirs=`@FIND@ . -type d -a ! -empty -mindepth 1 -maxdepth 1 \
	-a ! -name common -a ! -name CVS | \
	@SED@ s+^\\\.\\\/++g | @SORT@`

DISTCLEANFILES = Makefile.in Makefile

distclean-generic: clean
	-rm -fv `@FIND@ . -name \*.d -o -name \*.dm -o -name .depend`
	-rm -f $(CONFIG_CLEAN_FILES)
	-rm -f stamp-h stamp-h[0-9]*
	-test -z "$(DISTCLEANFILES)" || rm -f $(DISTCLEANFILES)

install uninstall clean all rpm:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	echo linking Makefiles
	@list=$(dirs); for subdir in $$list; do \
	for template in Makefile.am Makefile.in Makefile; do \
	  if test ! -e $$subdir/$$template; then \
	ln -s -f ../common/$$template $$subdir; fi; \
	done\
	done
	@list=$(dirs); for subdir in $$list; do \
	target=`echo $@ | sed s/-recursive//`; \
	echo "Making $$target in $$subdir"; \
	$(MAKE) -s -C $$subdir $(AM_MAKEFLAGS) $$target \
	|| case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	@if test x$@ == xdistclean; then \
	    rm -fv `@FIND@ . -name Makefile.in -o -name Makefile` \
	; fi;

