SUBDIRS = src plugins po

# not a GNU package.
AUTOMAKE_OPTIONS = foreign

DISTCLEANFILES = *~

release: distclean
	cd .. ; tar -cvzf libkwave-`cat libkwave/VERSION`.tar.gz libkwave/
