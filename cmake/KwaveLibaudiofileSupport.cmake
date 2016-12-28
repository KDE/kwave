#############################################################################
##    Kwave                - cmake/KwaveLibaudiofileSupport.txt
##                           -------------------
##    begin                : Tue May 22 2007
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

INCLUDE(CheckLibraryExists)
INCLUDE(FindPkgConfig)
INCLUDE(UsePkgConfig)
INCLUDE(CheckTypeSize)

PKG_CHECK_MODULES(LIBAUDIOFILE REQUIRED audiofile>=0.3.0)
IF (NOT LIBAUDIOFILE_FOUND)
    MESSAGE(FATAL_ERROR "audiofile library not found or too old")
ENDIF(NOT LIBAUDIOFILE_FOUND)

#############################################################################
### check if the system has audiofile.h                                   ###

FIND_PATH(HAVE_AUDIOFILE_H audiofile.h)
FIND_PATH(HAVE_AFS_VFS_H   af_vfs.h)

#############################################################################
### system libaudiofile can be used & header exists: check for functions  ###

IF (HAVE_AUDIOFILE_H AND HAVE_AFS_VFS_H)
    CHECK_LIBRARY_EXISTS(audiofile afOpenVirtualFile ${LIBAUDIOFILE_LIBDIR} HAVE_AF_OPEN_VIRTUAL_FILE)
ENDIF (HAVE_AUDIOFILE_H AND HAVE_AFS_VFS_H)

# check for compression types
SET(CMAKE_EXTRA_INCLUDE_FILES "audiofile.h")
CHECK_TYPE_SIZE("sizeof(AF_COMPRESSION_FLAC)" AF_COMPRESSION_FLAC)
CHECK_TYPE_SIZE("sizeof(AF_COMPRESSION_ALAC)" AF_COMPRESSION_ALAC)
SET(CMAKE_EXTRA_INCLUDE_FILES)

#############################################################################
### some verbose output of the result                                     ###

IF (HAVE_AF_OPEN_VIRTUAL_FILE)
    # system libaudiofile is ok and will be used
    MESSAGE(STATUS "Found libaudiofile")
ELSE (HAVE_AF_OPEN_VIRTUAL_FILE)
    # system libaudiofile should be used but is not ok -> FAIL
    MESSAGE(FATAL_ERROR "system libaudiofile is missing or cannot be used")
ENDIF (HAVE_AF_OPEN_VIRTUAL_FILE)

#############################################################################
#############################################################################
