#############################################################################
##    Kwave                - KwaveRPMSupport.cmake
##                           -------------------
##    begin                : Sun Jun 10 2007
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
### some needed programs                                                  ###

FIND_PROGRAM(TAR_EXECUTABLE NAMES tar) # we need tar-1.16 or newer !
FIND_PROGRAM(RPM_EXECUTABLE NAMES rpm)
FIND_PROGRAM(RPMBUILD_EXECUTABLE NAMES rpmbuild)
FIND_PROGRAM(BZIP2_EXECUTABLE NAMES bzip2)

#############################################################################
### determine all variables in the kwave.spec.in                          ###

SET(RPM_RELEASE "1")
SET(RPM_FULL_VERSION "${KWAVE_VERSION}-${RPM_RELEASE}")

#############################################################################
### generate the .changes file                                            ###

SET(_changes ${DISTFILES_DIR}/kwave.changes)
ADD_CUSTOM_COMMAND(OUTPUT ${_changes}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/bin/make-specfile-changelog.pl
        ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES
        ${_changes}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/make-specfile-changelog.pl
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

ADD_CUSTOM_TARGET(specfile
    DEPENDS ${_specfile}
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
            COMMAND test -e po && find po -name \\*.po ">>" ${files_lst}
            COMMAND find doc -type f -name \\*.txt ">>" ${files_lst}
            COMMAND find doc -type f -name index.docbook ">>" ${files_lst}
            COMMAND find doc -type f -name \\*.png ">>" ${files_lst}
            COMMAND find . -type f -name \\*.spec ">>" ${files_lst}
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
        --transform=s+^+kwave-${KWAVE_VERSION}/+g
        --owner=root --group=root --mode=a+rX,go-w
        -C ${CMAKE_SOURCE_DIR}
        -f ${_tarball}
        ${file_selection}
    COMMAND ${TAR_EXECUTABLE} --append -f ${_tarball}
        --owner=root --group=root --mode=a+rX,go-w
        -C ${DISTFILES_DIR}
        --transform=s+^+kwave-${KWAVE_VERSION}/+g
        kwave.spec
    COMMAND ${CMAKE_COMMAND} -E remove -f ${_tarball_bz2}
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
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_rpm_topdir}/{SPECS,SOURCES,RPMS,SRPMS,BUILD}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${_specfile} ${_rpm_topdir}/SPECS/kwave-${RPM_FULL_VERSION}.spec
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${_tarball_bz2} ${_rpm_topdir}/SOURCES/kwave-${RPM_FULL_VERSION}.tar.bz2
    COMMAND ${RPMBUILD_EXECUTABLE} -bs --nodeps
        ${_rpm_topdir}/SPECS/kwave-${RPM_FULL_VERSION}.spec
    DEPENDS ${_specfile} ${_tarball_bz2}
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
