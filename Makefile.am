AUTOMAKE_OPTIONS = foreign

srcdir  	 = $(shell pwd)

VERSION	 	 = $(shell cat ./VERSION)
ARCHIVE_DIR	 = ${srcdir}/../ARCHIVE
SRC_DIR   	 = ${srcdir}
SRC_PREFIX 	 = ${PACKAGE}
SUBDIRS  	 = mt libgui libkwave kwave plugins po doc

CLEANFILES       = *~ *.d *.dm *.dep *.moc *.moc.c?? .depend *.ii \#*
DISTCLEANFILES 	 = config.cache config.log config.h ${CLEANFILES}

.PHONY: doc version-labels

configure.in: VERSION
	touch configure.in

clean-generic:
	-rm -fv `find -size 0`
	-rm -fv `find . -name \*\~`
	-rm -fv `find . -name \*.o`
	-rm -fv `find . -name \*.ii`
	-rm -fv `find . -name \*.lo`
	-rm -fv `find . -name \*.la`
	-rm -fv `find . -name \*.moc.c??`
	-rm -fv `find . -name \*.moc`
	-rm -fv `find . -name \*.dep`
	-rm -fv `find . -name \*.d`
	-rm -fv `find . -name \*.dm`
	-rm -fv `find . -name .depend`
	-rm -fv `find . -name \*.uih.h`
	-rm -fv `find . -name \*.uic.cpp`
	-rm -rfv `find . -name .\* | grep -v "^\.\.$$" | grep -v "^\.$$"`

distclean-generic: clean-generic clean
	-rm -f $(CONFIG_CLEAN_FILES)
	-rm -f stamp-h stamp-h[0-9]*
	-test -z "$(DISTCLEANFILES)" || rm -f $(DISTCLEANFILES)
	-rm -Rf doc/api

reallyclean: clean clean-generic distclean-generic distclean
	-rm -f config.cache
	-rm -f config.log
	-rm -fv aclocal.m4
	-rm -fv `find . -type l`
	-rm -fv `find . -name Makefile.in`
	-rm -fv `find ./doc -name \*.html`
	-rm -fv `find . -name Makefile`

backup: ${PACKAGE}.spec
	@ cp ${PACKAGE}.spec ${PACKAGE}.spec.saved ; \
	${MAKE} reallyclean ; \
	mv ${PACKAGE}.spec.saved ${PACKAGE}.spec ; \
	(cd ${srcdir}/..; \
	tar -czh -f ${ARCHIVE_DIR}/${PACKAGE}-${VERSION}-backup.tgz \
	${SRC_PREFIX} ) && \
	echo source backed up to ${PACKAGE}-${VERSION}-backup.tgz

patch:	${PACKAGE}.spec
	@ cp ${PACKAGE}.spec ${PACKAGE}.spec.saved ; \
	${MAKE} reallyclean ; \
	mv ${PACKAGE}.spec.saved ${PACKAGE}.spec ; \
	(cd ${srcdir}/..; \
	tar -czh -f ${ARCHIVE_DIR}/${PACKAGE}-${VERSION}.tar.gz ${SRC_PREFIX} ) && \
	echo source backed up to ${PACKAGE}-${VERSION}.tar.gz
	@ $(srcdir)/bin/make_patch.sh ${PACKAGE} `cat $(srcdir)/VERSION` \
		${ARCHIVE_DIR} ${SRC_DIR} ${SRC_PREFIX}

version-labels:
	$(srcdir)/bin/set_version.sh $(srcdir) \
	`cat $(srcdir)/VERSION` \
	`date --iso`

patchlevel:
	$(srcdir)/bin/increment_patchlevel.sh ${srcdir}
	${MAKE} doc
	${MAKE} patch

release:
	@ $(srcdir)/bin/increment_release.sh ${srcdir}
	( cd doc ; ${MAKE} doc )
	${MAKE} patch

messages:
	rm -f po/*.gmo
	(cd kwave ; ${MAKE} menus_config_i18n.cpp )
	/usr/bin/xgettext -C -ki18n -x ${KDEDIR}/include/kde.pot \
	`find . -name \*.h -o -name \*.cpp` -o ./po/${PACKAGE}.pot
	(cd po ; ${MAKE} merge )

rpm_prep: distclean-generic ${PACKAGE}.spec
	rm -R -f /tmp/${PACKAGE}-@RPM_VERSION@
	mkdir -p /tmp/${PACKAGE}-@RPM_VERSION@
	(cd ${srcdir} ; cp -R -a ./* /tmp/${PACKAGE}-@RPM_VERSION@ )
	(cd /tmp/ ; \
	find ${SRC_PREFIX}-@RPM_VERSION@ -name CVS > \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	echo "${SRC_PREFIX}-@RPM_VERSION@/doc/api" >> \
		/tmp/${PACKAGE}-tar-excludes.lst ; \
	tar --exclude-from=/tmp/${PACKAGE}-tar-excludes.lst \
		-chzf ${PACKAGE}-@RPM_VERSION@.tar.gz \
		${SRC_PREFIX}-@RPM_VERSION@ ; \
	rm -f /tmp/${PACKAGE}-tar-excludes.lst )
	rm -R -f /tmp/${PACKAGE}-@RPM_VERSION@

# does not work with original SuSE-6.4 rpm
# you will need an update of the rpm package !
rpm: rpm_prep
	rpm -ta /tmp/${PACKAGE}-@RPM_VERSION@.tar.gz

src.rpm: rpm_prep
	rpm -bs ${PACKAGE}.spec

doc:
	(cd ./doc ; ${MAKE} all )

apidoc:
	-rm -Rf `find . -name \*.moc.cpp`
	doxygen doxy.cfg
