INCLUDES= @all_includes@

# Which sources should be compiled for kwave.
PLUGINDIR=$(libdir)/kwave/modules/time

CXXFLAGS  = -shared -fPIC -rdynamic -nostartfiles -nostdlib
MODDIR:=$(shell pwd)
MODNAME=$(shell basename $(MODDIR))

all: module.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) module.cpp -o$(MODNAME)

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = core *~ .deps/* $(MODNAME)
install-data-local: 
	$(mkinstalldirs) $(PLUGINDIR)
	$(INSTALL_DATA) $(MODNAME) $(PLUGINDIR)








