############################################################################
#    Kwave                - libgui/Makefile.am
#			     -------------------
#    begin                : Sat Nov 11 2001
#    copyright            : (C) 2000 by Thomas Eschenbacher
#    email                : Thomas.Eschenbacher@gmx.de
############################################################################

############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################

INCLUDES= -I${topdir} @all_includes@

METASOURCES=AUTO

# Which sources should be compiled for libgui.
libkwavegui_la_SOURCES = \
 CurveWidget.cpp\
 ConfirmCancelProxy.cpp\
 FileProgress.cpp\
 FrequencyResponseWidget.cpp\
 HMSTimeWidget.cpp\
 HMSTimeWidgetBase.ui\
 IntValidatorProxy.cpp\
 InvertableSpinBox.cpp\
 KwaveFileDialog.cpp\
 LabelPropertiesWidget.cpp\
 LabelPropertiesWidgetBase.ui\
 MenuNode.cpp\
 MenuGroup.cpp\
 MenuItem.cpp\
 MenuSub.cpp\
 MenuRoot.cpp\
 MenuManager.cpp\
 MultiStateWidget.cpp\
 OverViewCache.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 SelectTimeWidget.cpp\
 SelectTimeWidgetBase.ui\
 TrackPixmap.cpp

#noinst_LIBRARIES = libkwavegui.a
libkwavegui_la_LDFLAGS = -version-info @LD_VERSION_INFO@ --export-dynamic

lib_LTLIBRARIES = libkwavegui.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = core *~ .deps/* Makefile.in Makefile

# end of Makefile.am
