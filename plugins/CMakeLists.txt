#############################################################################
##    Kwave                - plugins/CMakeLists.txt
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

MACRO(KWAVE_PLUGIN _plugin)

    #########################################################################
    ### compile stage                                                     ###

    INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDE3_INCLUDE_DIR}
        ${QT_INCLUDE_DIR}
    )
    KDE4_ADD_UI_FILES(plugin_${_plugin}_LIB_SRCS ${plugin_${_plugin}_LIB_UI})
    QT4_AUTOMOC(${plugin_${_plugin}_LIB_SRCS})

    #########################################################################
    ### link stage                                                        ###

    KDE4_ADD_LIBRARY(plugin_${_plugin} SHARED ${plugin_${_plugin}_LIB_SRCS})
    TARGET_LINK_LIBRARIES(plugin_${_plugin}
        ${QT_AND_KDECORE_LIBS}
        ${plugin_${_plugin}_LIBS}
    )
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES OUTPUT_NAME "${_plugin}")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES LINK_FLAGS
        "-Wl,--export-dynamic -Wl,--add-needed")
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    #########################################################################
    ### install                                                           ###

    INSTALL(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lib${_plugin}.so
        RENAME ${_plugin}
        DESTINATION ${DESTDIR}${DATA_INSTALL_DIR}/kwave/plugins
    )

    ADD_DEPENDENCIES(plugin_${_plugin} libkwave libkwavegui)

ENDMACRO(KWAVE_PLUGIN)

#############################################################################

ADD_SUBDIRECTORY( about )
ADD_SUBDIRECTORY( amplifyfree )
ADD_SUBDIRECTORY( band_pass )
ADD_SUBDIRECTORY( codec_ascii )
ADD_SUBDIRECTORY( codec_audiofile ) # needs libaudiofile
ADD_SUBDIRECTORY( codec_flac )      # needs libflac 1.1.1 or 1.1.2
ADD_SUBDIRECTORY( codec_ogg )       # needs libogg and libvorbis
ADD_SUBDIRECTORY( codec_wav )       # needs libaudiofile
ADD_SUBDIRECTORY( decoder_mp3 )     # needs libmad and id3lib
ADD_SUBDIRECTORY( fileinfo )
ADD_SUBDIRECTORY( lowpass )
ADD_SUBDIRECTORY( memory )
ADD_SUBDIRECTORY( newsignal )
# ADD_SUBDIRECTORY( noise )
# ADD_SUBDIRECTORY( notch_filter )
# ADD_SUBDIRECTORY( pitch_shift )
# ADD_SUBDIRECTORY( playback )        # needs OSS, ALSA and/or aRts
# ADD_SUBDIRECTORY( record )          # needs OSS, ALSA and/or aRts
# ADD_SUBDIRECTORY( saveblocks )
# ADD_SUBDIRECTORY( selectrange )
# ADD_SUBDIRECTORY( sonagram )        # needs libgsl >= 1.3
# ADD_SUBDIRECTORY( volume )
ADD_SUBDIRECTORY( zero )

#############################################################################
#############################################################################
