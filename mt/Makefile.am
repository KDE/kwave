############################################################################
#    Kwave                - mt/Makefile.am
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

noinst_LTLIBRARIES = libkwavemt.la

# set the include path for X, qt and KDE
INCLUDES= @all_includes@

BUILTSOURCES=\
 AsyncSync.moc.cpp \
 Thread.moc.cpp \
 ThreadsafeX11Guard.moc.cpp

# Which sources should be compiled for libkwavemt.
libkwavemt_la_SOURCES=\
 AsyncSync.cpp\
 MutexGuard.cpp \
 SharedLock.cpp \
 SharedLockGuard.cpp \
 Semaphore.cpp \
 Thread.cpp \
 ThreadCondition.cpp \
 ThreadsafeX11Guard.cpp \
 $(BUILTSOURCES)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

# end of Makefile.am
