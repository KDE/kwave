POFILES = de.po 
GMOFILES = $(POFILES:.po=.gmo)

localedir = $(kde_locale)

MAINTAINERCLEANFILES = $(GMOFILES)

GMSGFMT = @GMSGFMT@
MSGFMT = @MSGFMT@
XGETTEXT = @XGETTEXT@
EXTRA_DIST = $(PACKAGE).pot $(POFILES) $(GMOFILES)
CLEANFILES = *.gmo

all: all-yes
all-yes: $(GMOFILES)
all-no:

# install-yes: all
install: all
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(DESTDIR)$(localedir); \
	lang=`echo $$cat | sed 's/\.gmo//'`; \
	name=$(PACKAGE).mo \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	$(mkinstalldirs) $$dir; \
	$(INSTALL_DATA) $$cat $$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

install-no:

uninstall-local:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	  destdir=$(DESTDIR)$(localedir); \
	  lang=`echo $$cat | sed 's/\.gmo//'`; \
	  name=$(PACKAGE).mo; \
	  dir=$$destdir/$$lang/LC_MESSAGES; \
	  rm -f $$cat $$dir/$$name; \
	  echo "removing $$dir/$$name" ; \
	  rmdir -p --ignore-fail-on-non-empty $$dir >/dev/null 2>&1; \
	done

