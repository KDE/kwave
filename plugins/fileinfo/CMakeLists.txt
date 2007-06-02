#############################################################################
##    Kwave                - plugins/fileinfo/CMakeLists.txt
##                           -------------------
##    begin                : Sat Jun 02 2007
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

SET(plugin_fileinfo_LIB_SRCS
    BitrateSpinBox.cpp
    BitrateWidget.cpp
    CompressionWidget.cpp
    FileInfoDialog.cpp
    FileInfoPlugin.cpp
    KeywordWidget.cpp
    SelectDateDialog.cpp
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
