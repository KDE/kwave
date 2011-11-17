#############################################################################
##    Kwave                - KwaveRPMSupport.cmake
##                           -------------------
##    begin                : Sun Jun 10 2007
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
### some needed programs                                                  ###

FIND_PROGRAM(MKDIR_EXECUTABLE NAMES mkdir)
FIND_PROGRAM(RM_EXECUTABLE NAMES rm)
FIND_PROGRAM(TAR_EXECUTABLE NAMES tar) # we need tar-1.16 or newer !
FIND_PROGRAM(RPM_EXECUTABLE NAMES rpm)
FIND_PROGRAM(RPMBUILD_EXECUTABLE NAMES rpmbuild)
FIND_PROGRAM(BZIP2_EXECUTABLE NAMES bzip2)

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

SET(RPM_SHORT_VERSION "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")
IF (KWAVE_VERSION_PATCHLEVEL)
    SET(RPM_RELEASE ${KWAVE_VERSION_PATCHLEVEL})
ELSE (KWAVE_VERSION_PATCHLEVEL)
    SET(RPM_RELEASE "1")
ENDIF (KWAVE_VERSION_PATCHLEVEL)
SET(RPM_FULL_VERSION "${RPM_SHORT_VERSION}-${RPM_RELEASE}")

SET(RPM_GROUP "X11/Applications/Sound")
GET_LSM(RPM_DESCRIPTION "Description")
GET_LSM(RPM_SUMMARY "Keywords")
GET_LSM(RPM_NAME "Title")
GET_LSM(RPM_COPYRIGHT "Copying-policy")
GET_LSM(RPM_URL "Homepage")
GET_LSM(RPM_VENDOR "Maintained-by")
SET(RPM_VENDOR "Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>")
SET(prefix "${KDE4_INSTALL_DIR}")

#############################################################################
### generate the .spec file                                               ###

SET(_specfile kwave.spec)
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/kwave.spec.in
    ${CMAKE_CURRENT_BINARY_DIR}/${_specfile}
    @ONLY
)

#############################################################################
### "make tarball"                                                        ###

SET(_tarball /tmp/kwave-${RPM_FULL_VERSION}.tar)
SET(_tarball_bz2 ${_tarball}.bz2)

ADD_CUSTOM_COMMAND(OUTPUT ${_tarball_bz2}
    COMMAND ${TAR_EXECUTABLE}
        -c
        --exclude=.svn --exclude=testfiles
        --owner=root --group=root
        -C ${CMAKE_SOURCE_DIR}
        --transform=s+^./+kwave-${RPM_SHORT_VERSION}/+g
        -f ${_tarball}
        .
    COMMAND ${TAR_EXECUTABLE} --append -f ${_tarball}
        --owner=root --group=root
        -C ${CMAKE_BINARY_DIR}
        --transform=s+^./+kwave-${RPM_SHORT_VERSION}/+g
        ./${_specfile}
    COMMAND ${RM_EXECUTABLE} -f ${_tarball_bz2}
    COMMAND ${BZIP2_EXECUTABLE} ${_tarball}
    DEPENDS ${CMAKE_BINARY_DIR}/${_specfile}
)

ADD_CUSTOM_TARGET(tarball
    DEPENDS ${_tarball_bz2}
)

#############################################################################
### source RPM                                                            ###

EXECUTE_PROCESS(
    COMMAND ${RPM_EXECUTABLE} -E %{_topdir}
    OUTPUT_VARIABLE _rpm_topdir
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

SET(_src_rpm ${_rpm_topdir}/SRPMS/kwave-${RPM_FULL_VERSION}.src.rpm)

ADD_CUSTOM_COMMAND(OUTPUT ${_src_rpm}
    COMMAND ${MKDIR_EXECUTABLE} -p ${_rpm_topdir}/{SPECS,SOURCES,RPMS,SRPMS,BUILD}
    COMMAND ${TAR_EXECUTABLE} -x -O -f ${_tarball_bz2} --wildcards \\*.spec >
        ${_rpm_topdir}/SPECS/kwave-${RPM_FULL_VERSION}.spec
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${_tarball_bz2} ${_rpm_topdir}/SOURCES/kwave-${RPM_FULL_VERSION}.tar.bz2
    COMMAND ${RPMBUILD_EXECUTABLE} -bs --nodeps
        ${_rpm_topdir}/SPECS/kwave-${RPM_FULL_VERSION}.spec
    DEPENDS ${_tarball_bz2}
)

ADD_CUSTOM_TARGET(src_rpm
    DEPENDS ${_src_rpm} ${CMAKE_CURRENT_BINARY_DIR}/${_specfile}
)

#############################################################################
### binary RPM for this architecture                                      ###

ADD_CUSTOM_TARGET(rpm
    COMMAND ${RPMBUILD_EXECUTABLE} --rebuild --nodeps ${_src_rpm}
    DEPENDS ${_src_rpm} ${CMAKE_CURRENT_BINARY_DIR}/${_specfile}
)

#############################################################################
#############################################################################
