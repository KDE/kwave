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

#############################################################################
### build options:                                                        ###

# OPTION(DEBUG           "build debug code [default=off]" OFF)
# OPTION(DEBUG_MEMORY    "enable memory management debug code [default=off]" OFF)
# OPTION(WITH_ALSA       "enable playback/recording via ALSA [default=on]" ON)
# OPTION(WITH_DOC        "build online documentation [default=on]" ON)
# OPTION(WITH_FLAC       "enable support for FLAC files [default=on]" ON)
# OPTION(WITH_MP3        "enable support for mp3 files [default=off]" OFF)
# OPTION(WITH_OGG_OPUS   "enable support for ogg/opus files [default=on]" ON)
# OPTION(WITH_OGG_VORBIS "enable support for ogg/vorbis files [default=on]" ON)
# OPTION(WITH_OSS        "enable playback/recording via OSS [default=on]" ON)
# OPTION(WITH_OPTIMIZED_MEMCPY "enable optimized memcpy [default=on]" ON)
# OPTION(WITH_PHONON     "enable playback via Phonon [default=off]" OFF)
# OPTION(WITH_PULSEAUDIO "enable playback/recording via PulseAudio [default=on]" ON)

#############################################################################
### toplevel build targets:                                               ###

# all                 - default target, build all files
# clean               - clean up the current build directory
# deb                 - create a debian package
# doc                 - generate docbook files for online help
# distfiles           - generate subdirectory with all files for distribution
# html_doc            - generate HTML help (for the web)
# install             - install the package, with support for DESTDIR
# rpm                 - create a RPM package + a src.rpm file
# src_rpm             - create the source RPM only
# tarball             - create a tar.bz2 archive with the sources + specfile
# uninstall           - uninstall the package, with support for DESTDIR
# update-translations - update translation files from KDE SVN
# version-labels      - update all version numbers and build dates
# tarball             - create a tar.gz file with the sources

#############################################################################
### check for the proper cmake version                                    ###

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

# Honor visibility properties for all target types (since cmake 3.3)
IF (POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF (POLICY CMP0063)

#############################################################################
### path to additional cmake modules and includes                         ###

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#############################################################################
### Kwave version number                                                  ###

FIND_PACKAGE(RequiredProgram REQUIRED)
FIND_REQUIRED_PROGRAM(CAT_EXECUTABLE cat)
INCLUDE(FindKwaveVersion)

#############################################################################
### show the compiler and it's version                                    ###

EXECUTE_PROCESS(
    COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE COMPILER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
GET_FILENAME_COMPONENT(COMPILER_SHORT "${CMAKE_C_COMPILER}" NAME_WE CACHE)
MESSAGE(STATUS "Building with ${COMPILER_SHORT} version ${COMPILER_VERSION}")

#############################################################################
### check for the CPU we build for                                        ###

EXECUTE_PROCESS(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE MACHINE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Building for target ${MACHINE}")

OPTION(WITH_OPTIMIZED_MEMCPY "enable optimized memcpy [default=on]" ON)
IF (WITH_OPTIMIZED_MEMCPY)

    STRING(REGEX MATCH "(i.86-*)|(athlon-*)|(pentium-*)" _mach_x86 ${MACHINE})
    IF (_mach_x86)
        MESSAGE(STATUS "Found target optimized memcpy() for X86 (from xine)")
        SET(ARCH_X86 1)
    ENDIF (_mach_x86)

    STRING(REGEX MATCH "(x86_64-*)|(X86_64-*)|(AMD64-*)|(amd64-*)" _mach_x86_64 ${MACHINE})
    IF (_mach_x86_64)
        MESSAGE(STATUS "Found target optimized memcpy() for X86_64 (from xine)")
        SET(ARCH_X86_64 1)
    ENDIF (_mach_x86_64)

    STRING(REGEX MATCH "(ppc-*)|(powerpc-*)" _mach_ppc ${MACHINE})
    IF (_mach_ppc)
        MESSAGE(STATUS "Found target optimized memcpy() for PPC (from xine)")
        SET(ARCH_PPC 1)
    ENDIF (_mach_ppc)

    IF (NOT ARCH_X86 AND NOT ARCH_X86_64 AND NOT ARCH_PPC)
        MESSAGE(STATUS "No platform specific memcpy available")
    ENDIF (NOT ARCH_X86 AND NOT ARCH_X86_64 AND NOT ARCH_PPC)

ELSE (WITH_OPTIMIZED_MEMCPY)
    MESSAGE(STATUS "Platform specific memcpy is disabled")
ENDIF (WITH_OPTIMIZED_MEMCPY)

#############################################################################
### cmake includes                                                        ###

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckTypeSize)
INCLUDE(CheckFunctionExists)

#############################################################################

SET(CMAKE_VERBOSE_MAKEFILE OFF)

#############################################################################
### toplevel compiler flags                                               ###

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)
ADD_DEFINITIONS(-DQT_NO_CAST_FROM_ASCII)

CHECK_C_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" C_HAVE_PIC)
CHECK_CXX_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}" CXX_HAVE_PIC)
IF (CXX_HAVE_PIC AND C_HAVE_PIC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
ELSE (CXX_HAVE_PIC AND C_HAVE_PIC)
    MESSAGE(FATAL_ERROR "shared library support is missing")
ENDIF (CXX_HAVE_PIC AND C_HAVE_PIC)

#############################################################################

LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libgui)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libkwave)

#############################################################################
### Qt 4 support                                                          ###

SET(QT_USE_QT3SUPPORT OFF)
SET(QT_USE_QTUITOOLS ON)
SET(QT_USE_QTCORE ON)
SET(QT_DONTUSE_QTGUI OFF)
SET(QT_USE_QTMAIN ON)

FIND_PACKAGE(Qt4 4.7.0 REQUIRED)
MESSAGE(STATUS "Found Qt4 version ${QTVERSION}")
MESSAGE(STATUS "Found Qt4 libraries in ${QT_LIBRARY_DIR}")
MESSAGE(STATUS "Found Qt4 header files in ${QT_INCLUDE_DIR}")

#############################################################################
### KDE 4 support                                                         ###

SET(KDE4_ENABLE_FPIE ON)

FIND_PROGRAM(KDECONFIG_EXECUTABLE kde4-config)
FIND_PACKAGE(KDE4 REQUIRED)
IF(WIN32)
  FIND_PACKAGE(KDEWIN32 REQUIRED)
ENDIF(WIN32)
INCLUDE(KDE4Defaults)
ADD_DEFINITIONS(${KDE4_DEFINITIONS})

SET(QT_AND_KDECORE_LIBS
    ${QT_QTGUI_LIBRARY}
    ${QT_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    pthread
)

INCLUDE_DIRECTORIES(
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################
### RPATH settings                                                        ###

SET(CMAKE_SKIP_BUILD_RPATH TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
MESSAGE(STATUS "Using ${CMAKE_INSTALL_RPATH} as install RPATH")

#############################################################################
### checks for needed header files                                        ###

CHECK_INCLUDE_FILES(sys/times.h HAVE_SYS_TIMES_H)
CHECK_INCLUDE_FILES(signal.h HAVE_SIGNAL_H)

SET(_inc errno.h limits.h math.h signal.h
         stdint.h stdio.h stdlib.h string.h sys/types.h unistd.h pthread.h)
CHECK_INCLUDE_FILES("${_inc}" REQUIRED_STD_HEADERS)
IF (NOT REQUIRED_STD_HEADERS)
    MESSAGE(FATAL_ERROR "
        unable to find one or more of the following header files:
        ${_inc}")
ENDIF (NOT REQUIRED_STD_HEADERS)

#############################################################################
### checks for some functions                                             ###

CHECK_FUNCTION_EXISTS(unlink HAVE_UNLINK)

INCLUDE(KwaveSysinfo)

#############################################################################
### libaudiofile and libsamplerate support                                ###

INCLUDE(KwaveLibaudiofileSupport)
INCLUDE(KwaveLibsamplerateSupport)

#############################################################################
### optionally: OSS, ALSA, PulseAudio and Phonon support                  ###
### for playback/recording                                                ###

INCLUDE(KwaveOSSSupport)
INCLUDE(KwaveALSASupport)
INCLUDE(KwavePhononSupport)
INCLUDE(KwavePulseAudioSupport)

#############################################################################
### cmake includes                                                        ###

INCLUDE(KwaveL10N)

#############################################################################
### flags for debugging                                                   ###

OPTION(DEBUG "build debug code [default=off]" OFF)
IF (DEBUG)
    SET(CMAKE_DEBUG_FLAGS "-O0 -fno-inline -g -Wall -W -Wextra -pedantic -fstack-protector-all")
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${CMAKE_DEBUG_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_DEBUG_FLAGS} -Wold-style-cast -Wno-non-virtual-dtor")
    SET(CMAKE_SHARED_LINKER_FLAGS "$ENV{CMAKE_SHARED_LINKER_FLAGS}")
    ADD_DEFINITIONS(-DDEBUG -D_DEBUG)
    SET(HAVE_DEBUG_PLUGIN  ON CACHE BOOL "enable debug plugin in the menu")
ELSE(DEBUG)
    ADD_DEFINITIONS(-DNO_DEBUG -DQT_NO_DEBUG)
ENDIF (DEBUG)

OPTION(DEBUG_MEMORY "enable memory management debug code [default=off]" OFF)

#############################################################################
### subdirs                                                               ###

ADD_SUBDIRECTORY( libgui )
ADD_SUBDIRECTORY( libkwave )
ADD_SUBDIRECTORY( kwave )
ADD_SUBDIRECTORY( plugins )

OPTION(WITH_DOC "build online documentation [default=on]" ON)
IF (WITH_DOC)
    ADD_SUBDIRECTORY( doc )
ENDIF (WITH_DOC)

#############################################################################
### RPM support                                                           ###

# directory that receives the files of the "distfiles" target
SET(DISTFILES_DIR ${CMAKE_BINARY_DIR}/v${KWAVE_VERSION_FULL})
FILE(MAKE_DIRECTORY ${DISTFILES_DIR})

INCLUDE(KwaveRPMSupport)

#############################################################################
### DEB support                                                           ###

INCLUDE(KwaveDEBSupport)

#############################################################################

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#############################################################################
### KDE .desktop file / mime types                                        ###

SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/wav;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-wav;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/basic;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-aifc;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-aiff;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-ircam;")
IF (WITH_MP3)
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/mpeg;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-mp1;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-mp2;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-mp3;")
ENDIF (WITH_MP3)
IF (WITH_FLAC)
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}application/x-flac;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-flac;")
ENDIF (WITH_FLAC)
IF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/ogg;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}application/ogg;")
ENDIF (WITH_OGG_OPUS OR WITH_OGG_VORBIS)
IF (WITH_OGG_VORBIS)
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-ogg;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}application/x-ogg;")
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-vorbis+ogg;")
ENDIF (WITH_OGG_VORBIS)
IF (WITH_OGG_OPUS)
    SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/opus;")
ENDIF (WITH_OGG_OPUS)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/kwave/kwave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/kwave/kwave.desktop
)

#############################################################################
### Gentoo support                                                        ###

INCLUDE(KwaveEbuild)

#############################################################################
### collection of all files used for distribution                         ###

FIND_PROGRAM(SED_EXECUTABLE NAMES sed)
SET(_distfiles_md5sum ${DISTFILES_DIR}/MD5SUMS-${KWAVE_VERSION_FULL})

ADD_CUSTOM_COMMAND(OUTPUT ${_distfiles_md5sum}
    COMMAND md5sum ${KWAVE_DISTFILES} |
        ${SED_EXECUTABLE} s+${DISTFILES_DIR}/++g > ${_distfiles_md5sum}
    DEPENDS ${KWAVE_DISTFILES}
)

ADD_CUSTOM_TARGET(distfiles
    DEPENDS ${KWAVE_DISTFILES} ${_distfiles_md5sum}
)

#############################################################################
### uninstall support                                                     ###

FIND_REQUIRED_PROGRAM(RMDIR_EXECUTABLE rmdir)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY
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
### "make apidoc"                                                         ###

FIND_PROGRAM(RM_EXECUTABLE rm)
FIND_PROGRAM(DOXYGEN_EXECUTABLE doxygen)

IF (DOXYGEN_EXECUTABLE AND RM_EXECUTABLE)
    SET(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    SET(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/apidoc)
    SET(DOXYGEN_LOGFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.log)

    MESSAGE(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")

    CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/doxy.cfg.in
	${DOXYFILE}
	@ONLY
    )

    ADD_CUSTOM_TARGET(apidoc
	COMMAND "${RM_EXECUTABLE}" -R -f "${CMAKE_CURRENT_BINARY_DIR}/html"
	COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
	COMMAND "${CAT_EXECUTABLE}" "${DOXYGEN_LOGFILE}"
	DEPENDS ${DOXYFILE}
    )

    SET(KWAVE_ADDITIONAL_CLEAN_FILES
	${KWAVE_ADDITIONAL_CLEAN_FILES}
	${DOXYGEN_OUTPUT_DIR}
	${DOXYGEN_LOGFILE}
    )

ENDIF (DOXYGEN_EXECUTABLE AND RM_EXECUTABLE)

#############################################################################
### additional files for "make clean"                                     ###

IF (KWAVE_ADDITIONAL_CLEAN_FILES)
    SET_DIRECTORY_PROPERTIES(PROPERTIES
	ADDITIONAL_MAKE_CLEAN_FILES
	"${KWAVE_ADDITIONAL_CLEAN_FILES}"
    )
ENDIF (KWAVE_ADDITIONAL_CLEAN_FILES)

#############################################################################
### "make wrapper"                                                        ###
### -> creates a wrapper script named "kw"                                ###

EXECUTE_PROCESS(
    COMMAND ${KDE4_KDECONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE KDE4_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

SET(WRAPPER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/kw")

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/kwave.wrapper.in"
    "${WRAPPER_SCRIPT}"
    @ONLY
)

ADD_CUSTOM_TARGET(wrapper
    COMMAND chmod +rx "${WRAPPER_SCRIPT}"
    DEPENDS "${WRAPPER_SCRIPT}"
)

#############################################################################

MESSAGE(STATUS "Using CFLAGS=${CMAKE_C_FLAGS}")
MESSAGE(STATUS "Using CXXFLAGS=${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "Using LDFLAGS=${LDFLAGS}")

#############################################################################
#############################################################################
