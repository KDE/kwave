AUTOMAKE_OPTIONS = foreign

srcdir  	 = $(shell pwd)

VERSION	 	 = $(shell cat ./VERSION)
ARCHIVE_DIR	 = ${srcdir}/../ARCHIVE
SRC_DIR   	 = ${srcdir}
SRC_PREFIX 	 = kwave
SUBDIRS  	 = libgui src plugins po

DISTCLEANFILES 	 = *~ config.cache config.log \#*

clean-generic:
	-rm -f -v `find -size 0`
	-rm -fv `find . -name \*.moc`
	-rm -fv `find . -name \*.d`
	-rm -fv `find . -name \*.dm`

distclean-generic:
	-rm -f $(CONFIG_CLEAN_FILES)
	-rm -f stamp-h stamp-h[0-9]*
	-test -z "$(DISTCLEANFILES)" || rm -f $(DISTCLEANFILES)

reallyclean: clean-generic distclean
	-rm -f config.cache
	-rm -f config.log
	-rm -f -v `find -size 0`
	-rm -fv `find . -name \*.moc`
	-rm -fv `find . -name \*.d`
	-rm -fv `find . -name \*.dm`
	-rm -fv `find . -type l`
	-rm -fv `find . -name Makefile`

backup:
	@ ${MAKE} reallyclean ; \
	(cd ${srcdir}/..; \
	tar -czh -f ${ARCHIVE_DIR}/kwave-${VERSION}.tar.gz ${SRC_PREFIX} ) && \
	echo source backed up to kwave-${VERSION}.tar.gz

patch: backup
	@ $(srcdir)/bin/make_patch.sh kwave `cat $(srcdir)/VERSION` \
		${ARCHIVE_DIR} ${SRC_DIR} ${SRC_PREFIX}

patchlevel:
	$(srcdir)/bin/increment_patchlevel.sh ${srcdir} && \
	${MAKE} patch

release:
	@ $(srcdir)/bin/increment_release.sh ${srcdir} && \
	${MAKE} patch

messages:
	rm -f po/*.gmo
	(cd src ; ${MAKE} menus_config_i18n.cpp )
	/usr/bin/xgettext -C -ki18n -x ${KDEDIR}/include/kde.pot \
	`find . -name \*.h -o -name \*.cpp` -o ./po/kwave.pot
	(cd po ; ${MAKE} merge )

rpm_prep: clean ${PACKAGE}.spec
	rm -R -f /tmp/${PACKAGE}-@RPM_VERSION@
	mkdir -p /tmp/${PACKAGE}-@RPM_VERSION@
	(cd ${srcdir} ; cp -R -a ./* /tmp/${PACKAGE}-@RPM_VERSION@ )
	(cd /tmp/ ; \
	tar -czh -f /usr/src/packages/SOURCES/kwave-@RPM_VERSION@.tar.gz \
		${SRC_PREFIX}-@RPM_VERSION@ )
	rm -R -f /tmp/${PACKAGE}-@RPM_VERSION@

rpm: rpm_prep
	rpm -ba ${PACKAGE}.spec

src.rpm: rpm_prep
	rpm -bs ${PACKAGE}.spec
