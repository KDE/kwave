#############################################################################
##    Kwave                - KwaveDEBSupport.cmake
##                           -------------------
##    begin                : Fri Apr 18 2014
##    copyright            : (C) 2014 by Thomas Eschenbacher
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
