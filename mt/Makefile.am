
# set the include path for X, qt and KDE
INCLUDES= @all_includes@

BUILTSOURCES=\
 AsyncSync.moc.cpp \
 Mutex.moc.cpp \
 MutexGuard.moc.cpp \
 Thread.moc.cpp \
 ThreadsafeX11Guard.moc.cpp \
 TSS_Object.moc.cpp

# Which sources should be compiled for libkwavemt.
libkwavemt_la_SOURCES=\
 AsyncSync.cpp\
 TSS_Object.cpp \
 Mutex.cpp \
 MutexGuard.cpp \
 MutexSet.cpp \
 SharedLock.cpp \
 SharedLockGuard.cpp \
 Semaphore.cpp \
 Thread.cpp \
 ThreadCondition.cpp \
 ThreadsafeX11Guard.cpp \
 $(BUILTSOURCES)

#noinst_LIBRARIES = libkwavemt.a
libkwavemt_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

lib_LTLIBRARIES = libkwavemt.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

# end of Makefile.am
