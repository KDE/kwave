#############################################################################
##    Kwave                - plugins/fileinfo/CMakeLists.txt
##                           -------------------
##    begin                : Sat Jun 02 2007
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

SET(plugin_fileinfo_LIB_SRCS
    BitrateSpinBox.cpp
    BitrateWidget.cpp
    CompressionWidget.cpp
    FileInfoDialog.cpp
    FileInfoPlugin.cpp
    KeywordWidget.cpp
    SelectDateDialog.cpp

    BitrateSpinBox.h
    BitrateWidget.h
    CompressionWidget.h
    FileInfoDialog.h
    FileInfoPlugin.h
    KeywordWidget.h
    SelectDateDialog.h
)

SET(plugin_fileinfo_LIB_UI
    BitrateWidgetBase.ui
    CompressionWidgetBase.ui
    FileInfoDlg.ui
    KeywordWidgetBase.ui
    SelectDateDlg.ui
)

KWAVE_PLUGIN(fileinfo)

#############################################################################
#############################################################################
