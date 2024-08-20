#############################################################################
##    Kwave                - plugins/saveblocks/CMakeLists.txt
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

SET(plugin_saveblocks_LIB_SRCS
    SaveBlocksDialog.cpp
    SaveBlocksPlugin.cpp
    SaveBlocksWidget.cpp

    SaveBlocksDialog.h
    SaveBlocksPlugin.h
    SaveBlocksWidget.h
)

SET(plugin_saveblocks_LIB_UI
    SaveBlocksWidgetBase.ui
)

KWAVE_PLUGIN(saveblocks)

#############################################################################
#############################################################################
