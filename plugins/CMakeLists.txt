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
        kwavegui
        kwave
        ${plugin_${_plugin}_LIBS}
        ${QT_AND_KDECORE_LIBS}
    )

    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES SUFFIX "")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES OUTPUT_NAME "${_plugin}")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES LINK_FLAGS
        "-Wl,--export-dynamic -Wl,--add-needed")
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    #########################################################################
    ### install                                                           ###

    ADD_DEPENDENCIES(plugin_${_plugin} libkwave libkwavegui)

    EXECUTE_PROCESS(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}"
        --expandvars --install module
        OUTPUT_VARIABLE KDE4_MODULE_INSTALL_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    INSTALL(TARGETS
        plugin_${_plugin}
        LIBRARY
        DESTINATION ${DESTDIR}${KDE4_MODULE_INSTALL_DIR}/plugins/kwave
    )

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
ADD_SUBDIRECTORY( debug )
ADD_SUBDIRECTORY( decoder_mp3 )     # needs libmad and id3lib
ADD_SUBDIRECTORY( fileinfo )
ADD_SUBDIRECTORY( goto )
ADD_SUBDIRECTORY( lowpass )
ADD_SUBDIRECTORY( memory )
ADD_SUBDIRECTORY( newsignal )
ADD_SUBDIRECTORY( noise )
ADD_SUBDIRECTORY( normalize )
ADD_SUBDIRECTORY( notch_filter )
ADD_SUBDIRECTORY( pitch_shift )
ADD_SUBDIRECTORY( playback )        # needs OSS and/or ALSA
ADD_SUBDIRECTORY( record )          # needs OSS and/or ALSA
ADD_SUBDIRECTORY( reverse )
ADD_SUBDIRECTORY( saveblocks )
ADD_SUBDIRECTORY( selectrange )
ADD_SUBDIRECTORY( sonagram )        # needs fftw >= 3.0
ADD_SUBDIRECTORY( volume )
ADD_SUBDIRECTORY( zero )

#############################################################################
#############################################################################
