#############################################################################
##    Kwave                - cmake/FindKwaveVersion.cmake
##                           -------------------
##    begin                : Fri May 11 2007
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
SET(KWAVE_VERSION_MAJOR   ${KDE_APPLICATIONS_VERSION_MAJOR})
SET(KWAVE_VERSION_MINOR   ${KDE_APPLICATIONS_VERSION_MINOR})
SET(KWAVE_VERSION_RELEASE ${KDE_APPLICATIONS_VERSION_MICRO})

#############################################################################
### assemble full version number: major.minor.release                     ###
SET(KWAVE_VERSION "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")

#############################################################################
### status output                                                         ###
MESSAGE(STATUS "Building Kwave version ${KWAVE_VERSION}")

#############################################################################
#############################################################################
