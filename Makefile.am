SUBDIRS = libgui src plugins po

# not a GNU package.
AUTOMAKE_OPTIONS = foreign

DISTCLEANFILES = *~ #*

release: distclean
	cd .. ; tar -cvzf kwavedevel-`date +%Y%m%d`.tar.gz kwave/

