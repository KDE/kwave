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
    SET(_get_lsm ${CMAKE_SOURCE_DIR}/bin/get_lsm_entry.pl)
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
### generate the .changes file                                            ###

SET(_changes ${DISTFILES_DIR}/kwave.changes)
ADD_CUSTOM_COMMAND(OUTPUT ${_changes}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/make-specfile-changelog.pl
        ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES
        ${_changes}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES
)

SET(KWAVE_ADDITIONAL_CLEAN_FILES ${KWAVE_ADDITIONAL_CLEAN_FILES} ${_changes})

#############################################################################
### generate the .spec file                                               ###

SET(_specfile_without_changelog ${CMAKE_CURRENT_BINARY_DIR}/kwave.spec.no-chglog)
SET(_specfile ${DISTFILES_DIR}/kwave.spec)
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/kwave.spec.in
    ${_specfile_without_changelog}
    @ONLY
)

ADD_CUSTOM_COMMAND(OUTPUT ${_specfile}
    COMMAND ${CAT_EXECUTABLE} ${_specfile_without_changelog} > ${_specfile}
    COMMAND ${CAT_EXECUTABLE} ${_changes} >> ${_specfile}
    DEPENDS ${_specfile_without_changelog} ${_changes}
)

#############################################################################
### "make tarball"                                                        ###

SET(_tarball ${DISTFILES_DIR}/kwave-${RPM_FULL_VERSION}.tar)
SET(_tarball_bz2 ${_tarball}.bz2)

SET(_git "${CMAKE_SOURCE_DIR}/.git")
IF (EXISTS ${_git})
    FIND_PROGRAM(GIT_EXECUTABLE NAMES git)
    IF (GIT_EXECUTABLE)
	MESSAGE(STATUS "Found git: ${GIT_EXECUTABLE}")
	SET(files_lst "${CMAKE_BINARY_DIR}/files.lst")
	ADD_CUSTOM_COMMAND(OUTPUT ${files_lst}
	    COMMENT "Building file list from local .git repository"
	    COMMAND "${GIT_EXECUTABLE}" ls-tree -r --name-only HEAD ">" ${files_lst}
	    COMMAND test -e po && find po -name \\*.gmo ">>" ${files_lst}
	    COMMAND find doc -type f ">>" ${files_lst}
	    COMMAND cat ${files_lst} | sort | uniq > ${files_lst}.tmp
	    COMMAND mv ${files_lst}.tmp ${files_lst}
	    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
	SET(file_selection "--files-from=${files_lst}")
    ELSE (GIT_EXECUTABLE)
	MESSAGE(STATUS "Warning: .git exists but git program not found")
    ENDIF (GIT_EXECUTABLE)
ELSE (EXISTS ${_git})
    MESSAGE(STATUS "No git version control files")
ENDIF (EXISTS ${_git})
IF (NOT file_selection)
    SET(file_selection ".")
ENDIF (NOT file_selection)

ADD_CUSTOM_COMMAND(OUTPUT ${_tarball_bz2}
    COMMAND ${TAR_EXECUTABLE}
        -c
        --exclude=.git --exclude=testfiles --exclude=*~
        --transform=s+^./++g
        --transform=s+^+kwave-${RPM_SHORT_VERSION}/+g
        --owner=root --group=root --mode=a+rX,go-w
        -C ${CMAKE_SOURCE_DIR}
        -f ${_tarball}
        ${file_selection}
    COMMAND ${TAR_EXECUTABLE} --append -f ${_tarball}
        --owner=root --group=root --mode=a+rX,go-w
        -C ${DISTFILES_DIR}
        --transform=s+^+kwave-${RPM_SHORT_VERSION}/+g
        kwave.spec
    COMMAND ${RM_EXECUTABLE} -f ${_tarball_bz2}
    COMMAND ${BZIP2_EXECUTABLE} ${_tarball}
    DEPENDS ${_specfile} ${files_lst}
)

ADD_CUSTOM_TARGET(tarball
    DEPENDS ${_tarball_bz2}
)

SET(KWAVE_ADDITIONAL_CLEAN_FILES ${KWAVE_ADDITIONAL_CLEAN_FILES} ${_tarball_bz2})

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
    DEPENDS ${_src_rpm} ${_specfile} ${_changes}
)

#############################################################################
### binary RPM for this architecture                                      ###

ADD_CUSTOM_TARGET(rpm
    COMMAND ${RPMBUILD_EXECUTABLE} --rebuild --nodeps ${_src_rpm}
    DEPENDS ${_src_rpm} ${_specfile}
)

#############################################################################
### add files to the list of distribution files                           ###

SET(KWAVE_DISTFILES
    ${KWAVE_DISTFILES}
    ${_tarball_bz2}
    ${_specfile}
    ${_changes}
)

#############################################################################
#############################################################################
