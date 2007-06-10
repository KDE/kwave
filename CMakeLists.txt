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
### build options:                                                        ###

# OPTION(WITH_ALSA "enable playback/recording via ALSA [default=on]" ON)
# OPTION(WITH_BUILTIN_LIBAUDIOFILE "use builtin libaudiofile [default=off]")
# OPTION(WITH_DOC "build online documentation [default=on]" ON)
# OPTION(WITH_FLAC "enable support for FLAC files [default=on]" ON)
# OPTION(WITH_MP3 "enable support for mp3 files [default=on]" ON)
# OPTION(WITH_OGG "enable support for ogg files [default=on]" ON)
# OPTION(WITH_OSS "enable playback/recording via OSS [default=on]" ON)

#############################################################################
### toplevel build targets:                                               ###

# all            - default target, build all files
# clean          - clean up the current build directory
# doc            - generate docbook files for online help
# html_doc       - generate HTML help (for the web)
# install        - install the package, with support for DESTDIR
# rpm            - create a RPM package + a src.rpm file
# src_rpm        - create the source RPM only
# tarball        - create a tar.bz2 archive with the sources + specfile
# uninstall      - uninstall the package, with support for DESTDIR
# version-labels - update all version numbers and build dates
# tarball        - create a tar.gz file with the sources

#############################################################################
### check for the proper cmake version                                    ###

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)

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
### optionally: OSS and ALSA support for playback and recording           ###

INCLUDE(KwaveOSSSupport)
INCLUDE(KwaveALSASupport)

#############################################################################
### cmake includes                                                        ###

FIND_PACKAGE(RequiredProgram REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)
INCLUDE(FindARTS)
INCLUDE(MCOP)

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
ADD_SUBDIRECTORY( plugins )
ADD_SUBDIRECTORY( po )

OPTION(WITH_DOC "build online documentation [default=on]" ON)
IF (WITH_DOC)
    ADD_SUBDIRECTORY( doc )
ENDIF (WITH_DOC)

#############################################################################
### target package-messages                                               ###

INCLUDE(KwavePackageMessages)

#############################################################################

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

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
    EXTRA_UNINSTALL_FILES="$ENV{EXTRA_UNINSTALL_FILES}"
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

#############################################################################
### "make version-labels"                                                 ###

ADD_CUSTOM_TARGET(version-labels
    ${CMAKE_SOURCE_DIR}/bin/set_version.sh
        ${CMAKE_SOURCE_DIR}
        ${KWAVE_VERSION_FULL}
        ${KWAVE_VERSION_DATE_ISO}
)

#############################################################################
### RPM support                                                           ###

INCLUDE(KwaveRPMSupport)

#original Makefile.am contents follow:

#apidoc:
#	-rm -Rf `find . -name \*.moc`
#	doxygen doxy.cfg

#############################################################################
#############################################################################
