#############################################################################
##    Kwave                - cmake/FindKwaveVersion.cmake
##                           -------------------
##    begin                : Fri May 11 2007
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
