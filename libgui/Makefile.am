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

BUILTSOURCES=\
 ConfirmCancelProxy.moc.cpp\
 CurveWidget.moc.cpp\
 FileProgress.moc.cpp\
 FrequencyResponseWidget.moc.cpp\
 HMSTimeWidgetBase.uih.h\
 HMSTimeWidgetBase.uih.moc.cpp\
 HMSTimeWidgetBase.uic.cpp\
 HMSTimeWidget.moc.cpp\
 IntValidatorProxy.moc.cpp\
 InvertableSpinBox.moc.cpp\
 KwaveFileDialog.moc.cpp\
 MenuNode.moc.cpp\
 MenuGroup.moc.cpp \
 MenuItem.moc.cpp\
 MenuSub.moc.cpp\
 MenuRoot.moc.cpp\
 MenuManager.moc.cpp \
 MultiStateWidget.moc.cpp\
 Notice.moc.cpp\
 OverViewCache.moc.cpp\
 OverViewWidget.moc.cpp\
 SelectTimeWidgetBase.uih.h \
 SelectTimeWidgetBase.uih.moc.cpp \
 SelectTimeWidgetBase.uic.cpp \
 SelectTimeWidget.moc.cpp\
 TrackPixmap.moc.cpp

# Which sources should be compiled for libgui.
libkwavegui_la_SOURCES =\
 $(BUILTSOURCES) \
 CurveWidget.cpp\
 ConfirmCancelProxy.cpp\
 FileProgress.cpp\
 FrequencyResponseWidget.cpp\
 HMSTimeWidget.cpp\
 IntValidatorProxy.cpp\
 InvertableSpinBox.cpp\
 KwaveFileDialog.cpp\
 MenuNode.cpp\
 MenuGroup.cpp \
 MenuItem.cpp\
 MenuSub.cpp\
 MenuRoot.cpp\
 MenuManager.cpp\
 MultiStateWidget.cpp\
 Notice.cpp\
 OverViewCache.cpp\
 OverViewWidget.cpp\
 ScaleWidget.cpp\
 SelectTimeWidget.cpp \
 TrackPixmap.cpp

#noinst_LIBRARIES = libkwavegui.a
libkwavegui_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

lib_LTLIBRARIES = libkwavegui.la

CLEANFILES:=$(CLEANFILES) $(BUILTSOURCES)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

%.uih.h: %.ui
	@UIC@ -o $@ -tr i18n $<

%.uic.cpp: %.ui
	@UIC@ -o $@ -tr i18n -impl $*.uih.h $<

# end of Makefile.am
