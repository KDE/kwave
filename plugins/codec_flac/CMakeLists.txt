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

#############################################################################
### check for FLAC API v1.1.3 and above                                   ###
### known to work:
### - v1.1.3
### - v1.1.4
### - v1.2.0
### - v1.2.1

    IF (NOT FLAC_API_FOUND)
        SET(_try_flac_1_1_3 "${_try_src_dir}/cmake_try_flac_v1_1_3.cpp")
        WRITE_FILE("${_try_flac_1_1_3}" "
            #include <FLAC/export.h>
            #include <FLAC/format.h>
            #include <FLAC++/decoder.h>
            #include <FLAC++/encoder.h>
            #include <FLAC++/metadata.h>

            /* FLAC-1.1.3 has cur=8, rev=0 */
            /* FLAC-1.1.4 has cur=8, rev=1 */
            #if !defined(FLAC_API_VERSION_CURRENT) || \\
                !defined(FLAC_API_VERSION_REVISION)
                #error "FLAC\ API\ is\ much\ too\ old"
            #endif

            #if (FLAC_API_VERSION_CURRENT < 8)
                #error "FLAC\ API\ is\ too\ old"
            #endif

            int main(int, char **)
            {
                FLAC__StreamEncoderInitStatus init_state_d =
                        FLAC__STREAM_ENCODER_INIT_STATUS_OK;
                FLAC__StreamDecoderInitStatus init_state_e =
                        FLAC__STREAM_DECODER_INIT_STATUS_OK;
                return 0;
            }
        ")

        TRY_COMPILE(
            FLAC_API_VERSION_1_1_3
            ${CMAKE_BINARY_DIR}
            ${_try_flac_1_1_3}
            CMAKE_FLAGS -DLINK_LIBRARIES:STRING=FLAC++
            OUTPUT_VARIABLE _out
        )

        IF (FLAC_API_VERSION_1_1_3)
            MESSAGE(STATUS "Found FLAC API version 1.1.3 or newer")
            SET(FLAC_API_FOUND BOOL ON)
        ELSE (FLAC_API_VERSION_1_1_3)
            # failed, now try again with linking ogg
            WRITE_FILE("${_try_flac_1_1_3}" "
                #include <FLAC/export.h>
                #include <FLAC/format.h>
                #include <FLAC++/decoder.h>
                #include <FLAC++/encoder.h>
                #include <FLAC++/metadata.h>

                /* FLAC-1.1.3 has cur=8, rev=0 */
                /* FLAC-1.1.4 has cur=8, rev=1 */
                #if !defined(FLAC_API_VERSION_CURRENT) || \\
                    !defined(FLAC_API_VERSION_REVISION)
                    #error "FLAC\ API\ is\ much\ too\ old"
                #endif

                #if (FLAC_API_VERSION_CURRENT < 8)
                    #error "FLAC\ API\ is\ too\ old"
                #endif

                int main(int, char **)
                {
                    FLAC__StreamEncoderInitStatus init_state_d =
                            FLAC__STREAM_ENCODER_INIT_STATUS_OK;
                    FLAC__StreamDecoderInitStatus init_state_e =
                            FLAC__STREAM_DECODER_INIT_STATUS_OK;
                    return 0;
                }
            ")
            TRY_COMPILE(
                FLAC_API_VERSION_1_1_3
                ${CMAKE_BINARY_DIR}
                ${_try_flac_1_1_3}
                CMAKE_FLAGS -DLINK_LIBRARIES:STRING=FLAC++ -DLINK_LIBRARIES:STRING=ogg
                OUTPUT_VARIABLE _out
            )
            IF (FLAC_API_VERSION_1_1_3)
                SET(FLAC_NEEDS_OGG ogg)
                MESSAGE(STATUS "Found FLAC API version 1.1.3 or newer (needs libogg)")
                SET(FLAC_API_FOUND BOOL ON)
            ENDIF (FLAC_API_VERSION_1_1_3)
        ENDIF (FLAC_API_VERSION_1_1_3)

        IF (NOT FLAC_API_VERSION_1_1_3)
#             MESSAGE(STATUS "FLAC API v1.1.3 does not work")
#             MESSAGE(STATUS "${_out}")
        ENDIF (NOT FLAC_API_VERSION_1_1_3)
    ENDIF (NOT FLAC_API_FOUND)

#############################################################################
### check for FLAC API v1.1.2 (older ones are no longer supported)        ###

    IF (NOT FLAC_API_FOUND)
        SET(_try_flac_1_1_2 "${_try_src_dir}/cmake_try_flac_v1_1_2.cpp")
        WRITE_FILE("${_try_flac_1_1_2}" "
            #include <FLAC/format.h>
            #include <FLAC++/decoder.h>
            #include <FLAC++/encoder.h>
            #include <FLAC++/metadata.h>

            int main(int, char **)
            {
                const FLAC::Metadata::VorbisComment vorbis_comments;
                const FLAC__byte *vendor = vorbis_comments.get_vendor_string();
                if (!vendor) return -1;

                FLAC::Decoder::Stream *decoder = 0;
                FLAC::Encoder::Stream *encoder = 0;
                FLAC::Decoder::Stream::State init_state_d = decoder->init();
                FLAC::Encoder::Stream::State init_state_e = encoder->init();

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
#             MESSAGE(STATUS "FLAC API v1.1.2 does not work")
#             MESSAGE(STATUS "${_out}")
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

    SET(plugin_codec_flac_LIBS FLAC++ FLAC ${FLAC_NEEDS_OGG} )

    KWAVE_PLUGIN(codec_flac)

ENDIF(WITH_FLAC)

#############################################################################
#############################################################################
