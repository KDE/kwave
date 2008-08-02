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
### build option: force usage of builtin libaudiofile                     ###

OPTION(WITH_BUILTIN_LIBAUDIOFILE "use builtin libaudiofile [default=off]")

#############################################################################
### if builtin version not forced: check if the system has audiofile.h    ###

IF (NOT WITH_BUILTIN_LIBAUDIOFILE)
    CHECK_INCLUDE_FILES(audiofile.h HAVE_AUDIOFILE_H)
ENDIF (NOT WITH_BUILTIN_LIBAUDIOFILE)

#############################################################################
### system libaudiofile can be used & header exists: check for functions  ###

IF (HAVE_AUDIOFILE_H)
    CHECK_LIBRARY_EXISTS(audiofile afOpenVirtualFile "" HAVE_AF_OPEN_VIRTUAL_FILE)
    CHECK_LIBRARY_EXISTS(audiofile af_virtual_file_new "" HAVE_AF_VIRTUAL_FILE_NEW)
ENDIF (HAVE_AUDIOFILE_H)

#############################################################################
### some verbose output of the result + set USE_SYSTEM_LIB_AUDIOFILE      ###

IF (HAVE_AF_OPEN_VIRTUAL_FILE AND HAVE_AF_VIRTUAL_FILE_NEW)
    # system libaudiofile is ok and will be used
    MESSAGE(STATUS "Using the system's libaudiofile")
    SET(USE_SYSTEM_LIB_AUDIOFILE BOOL ON)
ELSE (HAVE_AF_OPEN_VIRTUAL_FILE AND HAVE_AF_VIRTUAL_FILE_NEW)
    IF (WITH_BUILTIN_LIBAUDIOFILE)
        # system libaudiofile maybe is not ok
        # -> don't care, we will use the builtin one anyway
        MESSAGE(STATUS "Using builtin libaudiofile")
    ELSE (WITH_BUILTIN_LIBAUDIOFILE)
        # system libaudiofile should be used but is not ok
        # -> FAIL
        MESSAGE(FATAL_ERROR "system libaudiofile is missing or cannot be used")
    ENDIF (WITH_BUILTIN_LIBAUDIOFILE)
ENDIF (HAVE_AF_OPEN_VIRTUAL_FILE AND HAVE_AF_VIRTUAL_FILE_NEW)

#############################################################################
### enable builtin libaudiofile if needed                                 ###

IF (NOT USE_SYSTEM_LIB_AUDIOFILE)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/libaudiofile)
    LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/libaudiofile)
    ADD_SUBDIRECTORY( libaudiofile )
ENDIF (NOT USE_SYSTEM_LIB_AUDIOFILE)

#############################################################################
#############################################################################
