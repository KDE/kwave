# make sure all subdirs get a makefile....
dirs=`find . -type d | grep -v ./template | grep -v \\\.$$ | \
	sed s°^\\\.\\\/°°g | sort`

distclean-generic:
	-rm -f $(CONFIG_CLEAN_FILES)
	-rm -f stamp-h stamp-h[0-9]*
	-test -z "$(DISTCLEANFILES)" || rm -f $(DISTCLEANFILES)

install clean distclean all:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	echo linking Makefiles
	list=$(dirs); for subdir in $$list; do \
	for template in Makefile Makefile.in Makefile.am; do \
	  if test ! -e $$subdir/$$template; then \
	ln -sfv ../template/$$template $$subdir; fi; \
	done\
	done
	list=$(dirs); for subdir in $$list; do \
	target=`echo $@ | sed s/-recursive//`; \
	echo "Making $$target in $$subdir"; \
	(cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$target) \
	|| case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
    
