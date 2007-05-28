#############################################################################
##    Kwave                - CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

#############################################################################
### project name                                                          ###

PROJECT(kwave)

SET(KWAVE_LINGUAS
    en:English
    de:Deutsch
    fr:Francais
    # <- add new languages here...
)

#############################################################################
### cmake includes                                                        ###

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE_DIRECTORIES(
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(FindKwaveVersion)

#############################################################################

SET(CMAKE_VERBOSE_MAKEFILE OFF)

#############################################################################
### toplevel compiler flags                                               ###

CHECK_C_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" C_HAVE_PIC)
CHECK_CXX_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" CXX_HAVE_PIC)
IF (CXX_HAVE_PIC AND C_HAVE_PIC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
ELSE (CXX_HAVE_PIC AND C_HAVE_PIC)
    MESSAGE(FATAL_ERROR "shared library support is missing")
ENDIF (CXX_HAVE_PIC AND C_HAVE_PIC)

#############################################################################
### KDE 3 support                                                         ###

INCLUDE(KwaveKDESupport)

#############################################################################
### optionally: builtin libaudiofile                                      ###

INCLUDE(KwaveLibaudiofileSupport)

#############################################################################
### cmake includes                                                        ###

FIND_PACKAGE(RequiredProgram REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)
INCLUDE(FindARTS)
INCLUDE(MCOP)

#############################################################################

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#############################################################################

LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/mt)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libgui)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libkwave)

#############################################################################
### subdirs                                                               ###

ADD_SUBDIRECTORY( mt )
ADD_SUBDIRECTORY( libgui )
ADD_SUBDIRECTORY( libkwave )
ADD_SUBDIRECTORY( kwave )
# ADD_SUBDIRECTORY( plugins )
ADD_SUBDIRECTORY( po )
ADD_SUBDIRECTORY( doc )

#############################################################################
### target package-messages                                               ###

INCLUDE(KwavePackageMessages)

#############################################################################
### Gentoo support                                                        ###

INCLUDE(KwaveEbuild)

#############################################################################
### install files                                                         ###

INSTALL(
    FILES AUTHORS TODO CHANGES README
    DESTINATION ${KDE3_DOCDIR}/kwave-${KWAVE_VERSION_FULL}
)

#############################################################################
### uninstall support                                                     ###

FIND_REQUIRED_PROGRAM(RMDIR_EXECUTABLE rmdir)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY
)

ADD_CUSTOM_TARGET(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

#############################################################################

#original Makefile.am contents follow:

#DISTCLEANFILES 	 = config.cache config.log config.h Makefile.in Makefile \
#	QT-DIR aclocal.m4 config.h.in configure make.log admin/config.guess \
#	admin/config.sub admin/ltmain.sh \
#	libaudiofile/Makefile.in libaudiofile/modules/Makefile.in \
#	plugins/common/Makefile plugins/common/Makefile.in \
#	`find . -type l` autom4*.cache/* ${EBUILD}
#
#clean-generic:
#	-rm -fv `find -size 0`
#	-rm -fv `find . -name \*\~ -o -name \.\#\*`
#	-rm -fv `find . -name \*.o`
#	-rm -fv `find . -name \*.ii`
#	-rm -fv `find . -name \*.lo`
#	-rm -fv `find . -name \*.la`
#	-rm -fv `find . -name \*.moc.c\?\?`
#	-rm -fv `find . -name \*.moc`
#	-rm -fv `find . -name \*.dep`
#	-rm -fv `find . -name \*.d`
#	-rm -fv `find . -name \*.dm`
#	-rm -fv `find . -name .depend`
#	-rm -fv `find . -name \*.uih.h`
#	-rm -fv `find . -name \*.uic.cpp`
#	-rm -fv `find . -name \.inslog\*`
#	-rm -fv `find . -name \.ix\*`
#	-rm -Rf doc/api
#	-rm -f ${EBUILD}
#
#backup: ${PACKAGE}.spec
#	@ cp ${PACKAGE}.spec ${PACKAGE}.spec.saved ; \
#	${MAKE} distclean ; \
#	mv ${PACKAGE}.spec.saved ${PACKAGE}.spec ; \
#	(cd ${srcdir}/..; \
#	tar -czh -f ${ARCHIVE_DIR}/${PACKAGE}-${VERSION}-backup.tgz \
#	${SRC_PREFIX} ) && \
#	echo source backed up to ${PACKAGE}-${VERSION}-backup.tgz
#
#version-labels:
#	$(srcdir)/bin/set_version.sh $(srcdir) \
#	`cat $(srcdir)/VERSION` \
#	`date --iso`
#
#rpm_prep: doc clean distclean-generic ${PACKAGE}.spec
#	rm -R -f /tmp/${PACKAGE}-@RPM_SHORT_VERSION@
#	mkdir -p /tmp/${PACKAGE}-@RPM_SHORT_VERSION@
#	(cd ${srcdir} ; cp -R -a ./* /tmp/${PACKAGE}-@RPM_SHORT_VERSION@ )
#	(cd /tmp/ ; chmod -R a+rwX ${PACKAGE}-@RPM_SHORT_VERSION@ ; \
#	touch ${PACKAGE}-@RPM_SHORT_VERSION@/doc/help_{de,fr}.docbook ; \
#	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name CVS > \
#		/tmp/${PACKAGE}-tar-excludes.lst ; \
#	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name \.\* -type d >> \
#		/tmp/${PACKAGE}-tar-excludes.lst ; \
#	find ${SRC_PREFIX}-@RPM_SHORT_VERSION@ -name Makefile \
#		-o -name Makefile.in -o -name aclocal.m4 \
#		-o -name config.h.in -o -name config.h \
#		-o -name libtool -o -name stamp-h -o -name configure \
#		-o -name stamp-h1 -o -name \*.cache -o -name .depend \
#		-o -name QT-KDIR -o -name KDE-VERSION \
#		-o -name config.status -o -name .deps -o -name \*.log >> \
#		/tmp/${PACKAGE}-tar-excludes.lst ; \
#	echo "${SRC_PREFIX}-@RPM_SHORT_VERSION@/doc/api" >> \
#		/tmp/${PACKAGE}-tar-excludes.lst ; \
#	tar --exclude-from=/tmp/${PACKAGE}-tar-excludes.lst \
#		--owner=root --group=root \
#		-chzf ${PACKAGE}-@RPM_FULL_VERSION@.tar.gz \
#		${SRC_PREFIX}-@RPM_SHORT_VERSION@ ; \
#	 )
#	- rm -f /tmp/${PACKAGE}-tar-excludes.lst
#	- rm -R -f /tmp/${PACKAGE}-@RPM_SHORT_VERSION@
#
#rpm: rpm_prep
#	@RPMBUILD@ -ta /tmp/${PACKAGE}-@RPM_FULL_VERSION@.tar.gz
#
#src.rpm: rpm_prep
#	@RPMBUILD@ -ts /tmp/${PACKAGE}-@RPM_FULL_VERSION@.tar.gz
#
#doc:
#	(cd ./doc ; ${MAKE} all )
#
#apidoc:
#	-rm -Rf `find . -name \*.moc.cpp`
#	doxygen doxy.cfg

#############################################################################
#############################################################################
