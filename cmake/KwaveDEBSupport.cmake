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
### macro for extracting a field from the kwave.lsm file                  ###

MACRO(GET_LSM _var _field)
    SET(_get_lsm ${CMAKE_SOURCE_DIR}/bin/get_lsm_entry.sh)
    SET(_lsm ${CMAKE_SOURCE_DIR}/kwave.lsm)
    EXECUTE_PROCESS(
        COMMAND ${_get_lsm} ${_lsm} ${_field}
        OUTPUT_VARIABLE ${_var}
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ENDMACRO(GET_LSM)

#############################################################################
### determine all variables in the kwave.spec.in                          ###

SET(PACKAGE "kwave")
SET(PACKAGE_VERSION "${KWAVE_VERSION_FULL}")

SET(DEB_SHORT_VERSION "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")
IF (KWAVE_VERSION_PATCHLEVEL)
    SET(DEB_RELEASE ${KWAVE_VERSION_PATCHLEVEL})
ELSE (KWAVE_VERSION_PATCHLEVEL)
    SET(DEB_RELEASE "1")
ENDIF (KWAVE_VERSION_PATCHLEVEL)

#############################################################################
### binary DEB (for personal use only)                                    ###

ADD_CUSTOM_TARGET(deb
    COMMAND ${CHECKINSTALL_EXECUTABLE}
        -y -D
        --pkgname=${PACKAGE}
        --pkgversion=${DEB_SHORT_VERSION}
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
