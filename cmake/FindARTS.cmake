# - Try to find ARTS
# Once done this will define
#
#  ARTS_FOUND - system has ARTS
#  ARTS_CFLAGS - Compiler switches required for using ARTS
#  ARTS_LDLFAGS- Linker switches required for using ARTS
#  ARTSC_INCLUDE_DIR - the ARTS C include directory
#  ARTS_INCLUDE_DIR - the ARTS IDL include directory
#  KDE3_MCOPIDL_EXECUTABLE - MCOP IDL Preprocessor
#
#  based on FindARTS.cmake
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  2007-05-09
#      heavily modified by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

OPTION(WITH_ARTS "use aRts for internal streaming [defalt=on]" ON)

IF (WITH_ARTS)
    IF (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)
        # in cache already
        SET(ARTS_FOUND TRUE)
    ELSE (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)

        FIND_PROGRAM(ARTSC_CONFIG_EXECUTABLE
            NAMES
                artsc-config
            PATHS
                ${KDE3_BIN_DIR}
                /usr/bin
                /usr/local/bin
                /opt/kde3/bin
                /opt/kde4/bin
                /opt/gnome/bin
        )

        IF (ARTSC_CONFIG_EXECUTABLE)

            EXECUTE_PROCESS(
                COMMAND
                    ${ARTSC_CONFIG_EXECUTABLE} --arts-prefix
                OUTPUT_VARIABLE
                    ARTS_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            EXECUTE_PROCESS(
                COMMAND
                    ${ARTSC_CONFIG_EXECUTABLE} --cflags
                OUTPUT_VARIABLE
                    ARTS_CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            EXECUTE_PROCESS(
                COMMAND
                    ${ARTSC_CONFIG_EXECUTABLE} --libs
                OUTPUT_VARIABLE
                    ARTS_LDLFAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            FIND_PROGRAM(KDE3_MCOPIDL_EXECUTABLE
                NAMES
                    mcopidl
                PATHS
                    ${ARTS_PREFIX}/bin
                    ${KDE3_BIN_DIR}
                    /usr/bin
                    /usr/local/bin
                    /opt/kde3/bin
                    /opt/kde4/bin
                    /opt/gnome/bin
            )
            MESSAGE(STATUS "Found KDE3 mcopidl preprocessor: ${KDE3_MCOPIDL_EXECUTABLE}")

            FIND_PATH(ARTSC_INCLUDE_DIR
                NAMES
                    artsc.h
                PATHS
                    ${ARTS_PREFIX}/include
                    ${ARTS_PREFIX}/include/kde
                    ${KDE3_INCLUDE_DIR}
                    /usr/include
                    /usr/local/include
                    /opt/local/include
                    /opt/kde3/include
                    /sw/include
                PATH_SUFFIXES
                    artsc
                    arts
            )
            MESSAGE(STATUS "Found aRts C include dir: ${ARTSC_INCLUDE_DIR}")

            FIND_PATH(ARTS_INCLUDE_DIR
                NAMES
                    artsflow.idl
                PATHS
                    ${ARTS_PREFIX}/include
                    ${ARTS_PREFIX}/include/kde
                    ${KDE3_INCLUDE_DIR}
                    /usr/include
                    /usr/local/include
                    /opt/local/include
                    /opt/kde3/include
                    /sw/include
                PATH_SUFFIXES
                    artsc
                    arts
            )
            MESSAGE(STATUS "Found aRts idl include dir: ${ARTS_INCLUDE_DIR}")

        ENDIF (ARTSC_CONFIG_EXECUTABLE)

        # abort if not all flags found
        IF (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)
            SET(ARTS_FOUND TRUE)
        ELSE (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)
            MESSAGE(FATAL_ERROR "Could not find ARTS")
        ENDIF (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)

        # abort if mcopidl was not found
        IF (KDE3_MCOPIDL_EXECUTABLE)
            SET(KDE3_MCOPIDL_EXECUTABLE_FOUND TRUE)
        ELSE (KDE3_MCOPIDL_EXECUTABLE)
            MESSAGE(FATAL_ERROR "Could not find mcopidl preprocessor")
        ENDIF (KDE3_MCOPIDL_EXECUTABLE)

        # show the ARTS_CFLAGS and ARTS_LDFLAGS variables only in the advanced view
        MARK_AS_ADVANCED(ARTS_CFLAGS ARTS_LDFLAGS)

        # append aRts C include directory and C flags
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARTS_CFLAGS} -I${ARTS_INCLUDE_DIR}")

        # workaround for problems in aRts with non-virtual destructors
        STRING(REGEX REPLACE "-Wnon-virtual-dtor" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
        REMOVE_DEFINITIONS("-Wnon-virtual-dtor")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor")

    ENDIF (ARTS_CFLAGS AND ARTS_LDLFAGS AND ARTSC_INCLUDE_DIR AND ARTS_INCLUDE_DIR)

    # set the flag for config.h
    SET(HAVE_ARTS_SUPPORT true)

ENDIF (WITH_ARTS)

#############################################################################
#############################################################################
