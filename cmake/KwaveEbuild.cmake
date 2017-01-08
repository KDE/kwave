#############################################################################
##    Kwave                - cmake/KwaveEbuild.cmake
##                           -------------------
##    begin                : Wed May 23 2007
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

SET(_ebuild_version "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")
SET(_ebuild ${DISTFILES_DIR}/kwave-${_ebuild_version}.ebuild)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/kwave.ebuild.in" "${_ebuild}" @ONLY)

SET(KWAVE_DISTFILES ${KWAVE_DISTFILES} ${_ebuild})
SET(KWAVE_ADDITIONAL_CLEAN_FILES ${KWAVE_ADDITIONAL_CLEAN_FILES} ${_ebuild})

#############################################################################
#############################################################################
