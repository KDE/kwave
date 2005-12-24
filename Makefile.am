############################################################################
#    Kwave                - Makefile.am
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

AUTOMAKE_OPTIONS = 1.7.1   # Require Automake 1.7.1 or better.

.PHONY: backup ebuild rpm rpm_prep doc apidoc src.rpm version-labels

srcdir  	 = $(shell pwd)

VERSION	 	 = $(shell cat ./VERSION)
ARCHIVE_DIR	 = ${srcdir}/../ARCHIVE
SRC_DIR   	 = ${srcdir}
SRC_PREFIX 	 = ${PACKAGE}
SUBDIRS  	 = @LIBAUDIOFILE_SUBDIR@ mt libgui libkwave kwave plugins po doc
EBUILD		 = kwave-@RPM_SHORT_VERSION@.ebuild
CLEANFILES	 = *~ *.d *.dm *.moc *.moc.c?? *.ii \#* *.dep .depend

EXTRA_DIST	 = AUTHORS LICENSES TODO CHANGES GNU-LICENSE README

DISTCLEANFILES 	 = config.cache config.log config.h Makefile.in Makefile \
	QT-DIR aclocal.m4 config.h.in configure make.log admin/config.guess \
	admin/config.sub admin/ltmain.sh \
	libaudiofile/Makefile.in libaudiofile/modules/Makefile.in \
	plugins/common/Makefile plugins/common/Makefile.in \
	`find . -type l` autom4*.cache/* ${EBUILD}

# treat the ebuild file like a generated source:
# do not distribute it, but generate it
BUILT_SOURCES = ${EBUILD}

${EBUILD}: VERSION kwave.ebuild
	@ cp -v kwave.ebuild ${EBUILD}

ebuild: ${EBUILD}

configure.in: VERSION
	$(srcdir)/bin/set_version.sh $(srcdir) \
	`cat $(srcdir)/VERSION` \
	`date --iso`

clean-generic:
	-rm -fv `find -size 0`
	-rm -fv `find . -name \*\~ -o -name \.\#\*`
	-rm -fv `find . -name \*.o`
	-rm -fv `find . -name \*.ii`
	-rm -fv `find . -name \*.lo`
	-rm -fv `find . -name \*.la`
	-rm -fv `find . -name \*.moc.c\?\?`
	-rm -fv `find . -name \*.moc`
	-rm -fv `find . -name \*.dep`
	-rm -fv `find . -name \*.d`
	-rm -fv `find . -name \*.dm`
	-rm -fv `find . -name .depend`
	-rm -fv `find . -name \*.uih.h`
	-rm -fv `find . -name \*.uic.cpp`
	-rm -fv `find . -name \.inslog\*`
	-rm -fv `find . -name \.ix\*`
	-rm -Rf doc/api
	-rm -f ${EBUILD}

backup: ${PACKAGE}.spec
	@ cp ${PACKAGE}.spec ${PACKAGE}.spec.saved ; \
	${MAKE} distclean ; \
	mv ${PACKAGE}.spec.saved ${PACKAGE}.spec ; \
	(cd ${srcdir}/..; \
	tar -czh -f ${ARCHIVE_DIR}/${PACKAGE}-${VERSION}-backup.tgz \
	${SRC_PREFIX} ) && \
	echo source backed up to ${PACKAGE}-${VERSION}-backup.tgz

version-labels:
	$(srcdir)/bin/set_version.sh $(srcdir) \
	`cat $(srcdir)/VERSION` \
	`date --iso`

package-messages: all
	rm -f po/*.gmo
	(cd kwave ; ${MAKE} menus_config_i18n.cpp )
	@XGETTEXT@ -C -ki18n -x @KDE_POT_FILE@ \
	`find . -name \*.h -o -name \*.cpp` -o $(srcdir)/po/${PACKAGE}.pot
	(cd po ; ${MAKE} merge )

rpm_prep: doc clean distclean-generic ${PACKAGE}.spec
	rm -R -f /tmp/${PACKAGE}-@RPM_SHORT_VERSION@
	mkdir -p /tmp/${PACKAGE}-@RPM_SHORT_VERSION@
	(cd ${srcdir} ; cp -R -a ./* /tmp/${PACKAGE}-@RPM_SHORT_VERSION@ )
	(cd /tmp/ ; chmod -R a+rwX ${PACKAGE}-@RPM_SHORT_VERSION@ ; \
	touch ${PACKAGE}-@RPM_SHORT_VERSION@/doc/help_{de,fr}.docbook ; \
	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name CVS > \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name \.\* -type d >> \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name Makefile \
		-o -name Makefile.in -o -name aclocal.m4 \
		-o -name config.h.in -o -name config.h \
		-o -name libtool -o -name stamp-h -o -name configure \
		-o -name stamp-h1 -o -name \*.cache -o -name .depend \
		-o -name QT-KDIR -o -name KDE-VERSION \
		-o -name config.status -o -name .deps -o -name \*.log >> \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	echo "${SRC_PREFIX}-@RPM_SHORT_VERSION@/doc/api" >> \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	tar --exclude-from=/tmp/${PACKAGE}-tar-excludes.lst \
		--owner=root --group=root \
		-chzf ${PACKAGE}-@RPM_FULL_VERSION@.tar.gz \
		${SRC_PREFIX}-@RPM_SHORT_VERSION@ ; \
	 )
	- rm -f /tmp/${PACKAGE}-tar-excludes.lst
	- rm -R -f /tmp/${PACKAGE}-@RPM_SHORT_VERSION@

rpm: rpm_prep
	@RPMBUILD@ -ta /tmp/${PACKAGE}-@RPM_FULL_VERSION@.tar.gz

src.rpm: rpm_prep
	@RPMBUILD@ -ts /tmp/${PACKAGE}-@RPM_FULL_VERSION@.tar.gz

doc:
	(cd ./doc ; ${MAKE} all )

apidoc:
	-rm -Rf `find . -name \*.moc.cpp`
	doxygen doxy.cfg

#
############################################################################
