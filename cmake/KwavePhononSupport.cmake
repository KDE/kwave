#############################################################################
##    Kwave                - cmake/KwavePhononSupport.cmake
##                           -------------------
##    begin                : Fri May 15 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
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

OPTION(WITH_PHONON "enable playback via Phonon [default=off]" OFF)

IF (WITH_PHONON)

    FIND_PACKAGE(Phonon4Qt5 REQUIRED)

    IF (NOT PHONON_FOUND AND PHONON_FOUND_EXPERIMENTAL)
	SET(PHONON_FOUND 1)
        MESSAGE(STATUS "Found EXPERIMENTAL Phonon version")
    ENDIF (NOT PHONON_FOUND AND PHONON_FOUND_EXPERIMENTAL)

    IF (PHONON_FOUND)
        MESSAGE(STATUS "Found Phonon version ${PHONON_VERSION}")
        SET(HAVE_PHONON_SUPPORT  ON CACHE BOOL "enable Phonon support")
    ELSE (PHONON_FOUND)
        MESSAGE(FATAL_ERROR "Your system lacks Phonon support")
    ENDIF (PHONON_FOUND)

ENDIF (WITH_PHONON)

#############################################################################
#############################################################################
