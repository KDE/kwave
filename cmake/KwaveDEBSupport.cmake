#############################################################################
##    Kwave                - KwaveDEBSupport.cmake
##                           -------------------
##    begin                : Fri Apr 18 2014
##    copyright            : (C) 2014 by Thomas Eschenbacher
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
### some needed programs                                                  ###

FIND_PROGRAM(CHECKINSTALL_EXECUTABLE NAMES checkinstall)

#############################################################################
### set release number                                                    ###

SET(DEB_RELEASE "1")

#############################################################################
### binary DEB (for personal use only)                                    ###

ADD_CUSTOM_TARGET(deb
    COMMAND ${CHECKINSTALL_EXECUTABLE}
        -y -D
        --pkgname=${PROJECT_NAME}
        --pkgversion=${KWAVE_VERSION}
        --pkgrelease=${DEB_RELEASE}
        --install=no --fstrans
    COMMAND echo    ""
    COMMAND echo -e "    NOTE: This .deb file is for personal use and testing only"
    COMMAND echo -e "          not for distribution!"
    COMMAND echo -e "          It does not have the quality of the .deb packages you"
    COMMAND echo -e "          can get from your official package maintainer!"
    COMMAND echo    ""
)

#############################################################################
#############################################################################
