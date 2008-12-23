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

#############################################################################
### checks for some data types                                            ###

CHECK_TYPE_SIZE("size_t" SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("long" SIZEOF_LONG)

#############################################################################
### check if the system has audiofile.h                                   ###

CHECK_INCLUDE_FILES(audiofile.h HAVE_AUDIOFILE_H)
CHECK_INCLUDE_FILES(af_vfs.h    HAVE_AF_VFS_H)

#############################################################################
### system libaudiofile can be used & header exists: check for functions  ###

IF (HAVE_AUDIOFILE_H AND HAVE_AF_VFS_H)
    CHECK_LIBRARY_EXISTS(audiofile afOpenVirtualFile "" HAVE_AF_OPEN_VIRTUAL_FILE)
ENDIF (HAVE_AUDIOFILE_H AND HAVE_AF_VFS_H)

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
