#############################################################################
##    Kwave                - CMakeLists.txt
##                           -------------------
##    begin                : Tue May 01 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
#                                                                           #
# For details see the accompanying cmake/COPYING-CMAKE-SCRIPTS file.        #
#                                                                           #
#############################################################################

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")

#############################################################################
### build options:                                                        ###

# OPTION(DEBUG           "build debug code [default=off]" OFF)
# OPTION(WITH_ALSA       "enable playback/recording via ALSA [default=on]" ON)
# OPTION(WITH_DOC        "build online documentation [default=on]" ON)
# OPTION(WITH_FLAC       "enable support for FLAC files [default=on]" ON)
# OPTION(WITH_MP3        "enable support for mp3 files [default=off]" OFF)
# OPTION(WITH_OGG_OPUS   "enable support for ogg/opus files [default=on]" ON)
# OPTION(WITH_OGG_VORBIS "enable support for ogg/vorbis files [default=on]" ON)
# OPTION(WITH_OSS        "enable playback/recording via OSS [default=on]" ON)
# OPTION(WITH_OPTIMIZED_MEMCPY "enable optimized memcpy [default=on]" ON)
# OPTION(WITH_PULSEAUDIO "enable playback/recording via PulseAudio [default=on]" ON)
# OPTION(WITH_QT_AUDIO   "enable playback via Qt Multimedia [default=on]" ON)

#############################################################################
### toplevel build targets:                                               ###

# all                 - default target, build all files
# clean               - clean up the current build directory
# deb                 - create a debian package
# doc                 - generate docbook files for online help
# distfiles           - generate subdirectory with all files for distribution
# html_doc            - generate HTML help (for the web)
# install             - install the package, with support for DESTDIR
# msgstats            - show the progress of translations
# rpm                 - create a RPM package + a src.rpm file
# src_rpm             - create the source RPM only
# tarball             - create a tar.bz2 archive with the sources + specfile
# uninstall           - uninstall the package, with support for DESTDIR
# update-translations - update translation files from KDE SVN
# version-labels      - update all version numbers and build dates
# tarball             - create a tar.gz file with the sources

#############################################################################
### required versions                                                     ###

CMAKE_MINIMUM_REQUIRED(VERSION 3.16.0 FATAL_ERROR)

# Honor visibility properties for all target types (since cmake 3.3)
IF (POLICY CMP0063)
    CMAKE_POLICY(SET CMP0063 NEW)
ENDIF (POLICY CMP0063)

# exclude generated automoc files per default (since cmake 3.10)
IF (POLICY CMP0071)
    CMAKE_POLICY(SET CMP0071 NEW)
ENDIF (POLICY CMP0071)

# include file check macros honor CMAKE_REQUIRED_LIBRARIES (since cmake 3.12)
IF (POLICY CMP0075)
    CMAKE_POLICY(SET CMP0075 NEW)
ENDIF (POLICY CMP0075)

# Set policy for the VERSION inside PROJECT
IF (POLICY CMP0048)
    CMAKE_POLICY(SET CMP0048 NEW)
ENDIF (POLICY CMP0048)

SET(QT_MIN_VERSION  "5.15.0")
SET(KF5_MIN_VERSION "5.90.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
#############################################################################
### Kwave version number                                                  ###

SET(KWAVE_VERSION_MAJOR ${RELEASE_SERVICE_VERSION_MAJOR})
SET(KWAVE_VERSION_MINOR ${RELEASE_SERVICE_VERSION_MINOR})
SET(KWAVE_VERSION_MICRO ${RELEASE_SERVICE_VERSION_MICRO})
SET(KWAVE_VERSION "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_MICRO}")
MESSAGE(STATUS "Building Kwave version ${KWAVE_VERSION}")

#############################################################################
### project name and version                                              ###

PROJECT(kwave VERSION ${KWAVE_VERSION})

#############################################################################
### show the compiler name and version                                    ###

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
### path to additional cmake modules and includes                         ###

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#############################################################################
### cmake includes                                                        ###

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckIncludeFilesCXX)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckTypeSize)
INCLUDE(CheckFunctionExists)
INCLUDE(FindRequiredProgram)

#############################################################################
### toplevel compiler flags                                               ###

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)
ADD_DEFINITIONS(-DQT_NO_CAST_FROM_ASCII)
ADD_DEFINITIONS(-DQT_USE_FAST_OPERATOR_PLUS)
ADD_DEFINITIONS(-DQT_NO_URL_CAST_FROM_STRING)
ADD_DEFINITIONS(-DQT_DEPRECATED_WARNINGS)
ADD_DEFINITIONS(-DKXMLGUI_NO_DEPRECATED)

CHECK_C_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_C_FLAGS}" C_HAVE_PIC)
CHECK_CXX_COMPILER_FLAG(" ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}" CXX_HAVE_PIC)
IF (CXX_HAVE_PIC AND C_HAVE_PIC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_SHARED_LIBRARY_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
ELSE (CXX_HAVE_PIC AND C_HAVE_PIC)
    MESSAGE(FATAL_ERROR "shared library support is missing")
ENDIF (CXX_HAVE_PIC AND C_HAVE_PIC)

CHECK_CXX_COMPILER_FLAG("--std=c++11" CXX_HAVE_CXX11)
IF (CXX_HAVE_CXX11)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
ENDIF (CXX_HAVE_CXX11)

#############################################################################

LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libgui)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libkwave)

#############################################################################
### Qt 5 support                                                          ###

SET(CMAKE_AUTOMOC TRUE)
SET(CMAKE_AUTOMOC_RELAXED_MODE FALSE)

FIND_PACKAGE(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Concurrent
    Core
    Widgets
)

# NOTE: this must be *after* FIND_PACKAGE(Qt5 Core ...), otherwise it would
#       be overwritten and produce many annoying build warnings
LIST(APPEND CMAKE_AUTOMOC_MACRO_NAMES "KWAVE_PLUGIN")

# Qt Multimedia support
OPTION(WITH_QT_AUDIO "enable playback via Qt Multimedia [default=on]" ON)
IF (WITH_QT_AUDIO)
    SET(HAVE_QT_AUDIO_SUPPORT on)
    FIND_PACKAGE(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
        Multimedia
    )
ENDIF (WITH_QT_AUDIO)


#############################################################################
### KF5 support                                                           ###

FIND_PACKAGE(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH}  )

INCLUDE(KDEInstallDirs)
INCLUDE(KDECompilerSettings)
INCLUDE(KDECMakeSettings)
INCLUDE(GenerateExportHeader)
INCLUDE(FeatureSummary)

FIND_PACKAGE(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Completion
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    I18n
    IconThemes
    KIO
    TextWidgets
    XmlGui
    WidgetsAddons
)

IF (NOT WITH_DOC)
    SET_PACKAGE_PROPERTIES(KF5DocTools
        PROPERTIES DESCRIPTION "Tools to generate documentation"
        TYPE OPTIONAL
    )
ENDIF (NOT WITH_DOC)

FEATURE_SUMMARY(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

INCLUDE_DIRECTORIES(
    ${INTERFACE_INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#############################################################################
### checks for needed header files                                        ###

CHECK_INCLUDE_FILES(signal.h HAVE_SIGNAL_H)

SET(_inc_c errno.h math.h signal.h stdlib.h string.h unistd.h pthread.h)
CHECK_INCLUDE_FILES("${_inc_c}" HAVE_REQUIRED_STD_C_HEADERS)
IF (NOT HAVE_REQUIRED_STD_C_HEADERS)
    MESSAGE(FATAL_ERROR "
        unable to find one or more of the following C header files:
        ${_inc_c}")
ENDIF (NOT HAVE_REQUIRED_STD_C_HEADERS)

SET(_inc_cpp algorithm complex limits new)
CHECK_INCLUDE_FILES_CXX("${_inc_cpp}")

#############################################################################
### checks for some functions                                             ###

CHECK_FUNCTION_EXISTS(unlink HAVE_UNLINK)

#############################################################################
### libaudiofile and libsamplerate support                                ###

INCLUDE(KwaveLibaudiofileSupport)
INCLUDE(KwaveLibsamplerateSupport)

#############################################################################
### optionally: OSS, ALSA and PulseAudio support                          ###
### for playback/recording                                                ###

INCLUDE(KwaveOSSSupport)
INCLUDE(KwaveALSASupport)
INCLUDE(KwavePulseAudioSupport)

#############################################################################
### cmake includes                                                        ###

INCLUDE(KwaveL10N)

#############################################################################
### flags for debugging                                                   ###

OPTION(DEBUG "enable the debug plugin in the menu [default=off]" OFF)
IF (DEBUG)
    SET(HAVE_DEBUG_PLUGIN  ON CACHE BOOL "enable debug plugin in the menu")
ENDIF (DEBUG)

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
SET(DISTFILES_DIR ${CMAKE_BINARY_DIR}/v${KWAVE_VERSION})
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
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-8svx;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-aifc;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-aiff;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-avr;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-caf;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-ircam;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-nist;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-smp;")
SET(KWAVE_DESKTOP_MIME_TYPES "${KWAVE_DESKTOP_MIME_TYPES}audio/x-voc;")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/kwave/org.kde.kwave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/kwave/org.kde.kwave.desktop
)

#############################################################################
### Gentoo support                                                        ###

INCLUDE(KwaveEbuild)

#############################################################################
### collection of all files used for distribution                         ###

FIND_PROGRAM(SED_EXECUTABLE NAMES sed)
SET(_distfiles_md5sum ${DISTFILES_DIR}/MD5SUMS-${KWAVE_VERSION})

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
### "make apidoc"                                                         ###

FIND_PROGRAM(DOXYGEN_EXECUTABLE doxygen)
FIND_REQUIRED_PROGRAM(CAT_EXECUTABLE cat)

IF (DOXYGEN_EXECUTABLE)
    SET(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    SET(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc/api)
    SET(DOXYGEN_LOGFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.log)

    MESSAGE(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")

    CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/doxy.cfg.in
	${DOXYFILE}
	@ONLY
    )

    ADD_CUSTOM_TARGET(apidoc
	DEPENDS html_doc_devel
	COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_BINARY_DIR}/doc/api"
	COMMAND "${CMAKE_COMMAND}" -E make_directory   "${CMAKE_BINARY_DIR}/doc/api"
	COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
	COMMAND "${CAT_EXECUTABLE}" "${DOXYGEN_LOGFILE}"
	DEPENDS ${DOXYFILE}
    )

    SET(KWAVE_ADDITIONAL_CLEAN_FILES
	${KWAVE_ADDITIONAL_CLEAN_FILES}
	${DOXYGEN_OUTPUT_DIR}
	${DOXYGEN_LOGFILE}
    )

ENDIF (DOXYGEN_EXECUTABLE)

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
