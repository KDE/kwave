#############################################################################
##    Kwave                - plugins/export_k3b/CMakeLists.txt
##                           -------------------
##    begin                : Thu Mar 23 2017
##    copyright            : (C) 2017 by Thomas Eschenbacher
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

SET(plugin_export_k3b_LIB_SRCS
    K3BExportDialog.cpp
    K3BExportPlugin.cpp
    K3BExportWidget.cpp
)

SET(plugin_export_k3b_LIB_UI
    K3BExportWidgetBase.ui
)

SET(plugin_export_k3b_LIBS
    KF6::Archive
)

KWAVE_PLUGIN(export_k3b)

#############################################################################
#############################################################################

