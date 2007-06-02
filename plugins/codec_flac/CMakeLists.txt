#############################################################################
##    Kwave                - plugins/codec_flac/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

OPTION(WITH_FLAC "enable support for FLAC files [default=on]" ON)
IF (WITH_FLAC)

    INCLUDE(CheckLibraryExists)

#############################################################################
### check for FLAC header files                                           ###

    CHECK_INCLUDE_FILES(FLAC/format.h HAVE_FLAC_HEADERS)
    IF (HAVE_FLAC_HEADERS)
        MESSAGE(STATUS "Found FLAC include files")
    ELSE (HAVE_FLAC_HEADERS)
        MESSAGE(FATAL_ERROR "libFLAC not found, please install libflac !")
    ENDIF (HAVE_FLAC_HEADERS)

#############################################################################
### check for FLAC++ header files                                         ###

    IF (HAVE_FLAC_HEADERS)
        CHECK_INCLUDE_FILES(FLAC++/decoder.h HAVE_FLAC_HEADERS)
    ENDIF (HAVE_FLAC_HEADERS)
    IF (HAVE_FLAC_HEADERS)
        CHECK_INCLUDE_FILES(FLAC++/metadata.h HAVE_FLAC_HEADERS)
    ENDIF (HAVE_FLAC_HEADERS)

    IF (HAVE_FLAC_HEADERS)
        MESSAGE(STATUS "Found FLAC++ include files")
    ELSE (HAVE_FLAC_HEADERS)
        MESSAGE(FATAL_ERROR "libFLAC++ not found, please install libflac++ !")
    ENDIF (HAVE_FLAC_HEADERS)

    SET(_try_src_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
    SET(_try_flac_1_1_1 "${_try_src_dir}/cmake_try_flac_v1_1_1.cpp")
    WRITE_FILE("${_try_flac_1_1_1}" "
        #include <FLAC/format.h>
        #include <FLAC++/decoder.h>
        #include <FLAC++/metadata.h>

        int main()
        {
            const FLAC::Metadata::VorbisComment vorbis_comments;
            if (vorbis_comments.get_vendor_string().is_valid()) {
                const FLAC::Metadata::VorbisComment::Entry &entry =
                    vorbis_comments.get_vendor_string();
                const char *field = entry.get_field();
                unsigned int len  = entry.get_field_length();
                if (!field || !len) return -1;
            }
            return 0;
        }
    ")
    TRY_COMPILE(
        FLAC_API_VERSION_1_1_1_OR_OLDER
        ${CMAKE_BINARY_DIR}
        ${_try_flac_1_1_1}
        CMAKE_FLAGS -DLINK_LIBRARIES:STRING=FLAC++
        OUTPUT_VARIABLE _out
    )
    IF (FLAC_API_VERSION_1_1_1_OR_OLDER)
        MESSAGE(STATUS "Found FLAC API version 1.1.1 or older")
        SET(FLAC_API_FOUND BOOL ON)
    ELSE (FLAC_API_VERSION_1_1_1_OR_OLDER)
        MESSAGE(STATUS "FLAC API v1.1.1 or older does not work")
    ENDIF (FLAC_API_VERSION_1_1_1_OR_OLDER)

    IF (NOT FLAC_API_FOUND)
        SET(_try_flac_1_1_2 "${_try_src_dir}/cmake_try_flac_v1_1_2.cpp")
        WRITE_FILE("${_try_flac_1_1_2}" "
            #include <FLAC/format.h>
            #include <FLAC++/decoder.h>
            #include <FLAC++/metadata.h>

            int main(int, char **)
            {
                const FLAC::Metadata::VorbisComment vorbis_comments;
                const FLAC__byte *vendor = vorbis_comments.get_vendor_string();
                if (!vendor) return -1;
                return 0;
            }
        ")

        TRY_COMPILE(
            FLAC_API_VERSION_1_1_2
            ${CMAKE_BINARY_DIR}
            ${_try_flac_1_1_2}
            CMAKE_FLAGS -DLINK_LIBRARIES:STRING=FLAC++
            OUTPUT_VARIABLE _out
        )
        IF (FLAC_API_VERSION_1_1_2)
            MESSAGE(STATUS "Found FLAC API version 1.1.2")
            SET(FLAC_API_FOUND BOOL ON)
        ELSE (FLAC_API_VERSION_1_1_2)
            MESSAGE(STATUS "FLAC API v1.1.2 does not work")
        ENDIF (FLAC_API_VERSION_1_1_2)
    ENDIF (NOT FLAC_API_FOUND)

#############################################################################
### abort if no working FLAC API has been found :-(                       ###

    IF (NOT FLAC_API_FOUND)
        MESSAGE(FATAL_ERROR "Found no supported FLAC API")
    ENDIF (NOT FLAC_API_FOUND)

    SET(plugin_codec_flac_LIB_SRCS
        FlacCodecPlugin.cpp
        FlacDecoder.cpp
        FlacEncoder.cpp
        VorbisCommentMap.cpp
    )

    SET(plugin_codec_flac_LIBS FLAC++ FLAC)

    KWAVE_PLUGIN(codec_flac)

ENDIF(WITH_FLAC)

#############################################################################
#############################################################################
