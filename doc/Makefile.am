
MAINTAINERCLEANFILES = de/*.html en/*.html
CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook de/*.html en/*.html
DISTCLEANFILES = Makefile.in Makefile

LANGUAGES = en de
SUFFIXES = .docbook .po .pot
SGMLDIR=$(kde_datadir)/ksgmltools2
DOCDIR = $(DESTDIR)${kde_htmldir}

de/index.html: help_de.docbook
	cd de ; $(MEINPROC) --check ../help_de.docbook
	rm -Rf de/common
	mkdir -p de/common
	cp -R ${DOCDIR}/de/common/* de/common/

en/index.html: help_en.docbook
	cd en ; $(MEINPROC) --check ../help_en.docbook
	rm -Rf en/common
	mkdir -p en/common
	cp -R ${DOCDIR}/en/common/* en/common/
	cd en/common ; mv gpl-license.html gpl-translated.html

help_en.pot: help_en.docbook
	$(XML2POT) help_en.docbook > help_en.pot

help_de.po: help_en.pot
	$(MSGMERGE) -o help_de.po help_de.po help_en.pot
	# kbabel help_de.po

help_de.docbook: help_de.po help_en.docbook
	@echo "converting german help"
	@echo "(this takes very very long, please be patient...)"
	$(PO2XML) help_en.docbook help_de.po | \
	sed s/^\ \ \<\!ENTITY\ %\ English\ /\ \ \<\!ENTITY\ %\ German\ /g \
	> help_de.docbook

doc: help_de.docbook

html-doc: en/index.html de/index.html

html-clean:
	for lang in ${LANGUAGES}; do { \
	    rm -Rfv $${lang}/*.html; rm -Rf $${lang}/common; \
	}; done

all: doc

install-data-local:
	echo installing docs in $(DOCDIR)
	for lang in ${LANGUAGES}; do \
	{ \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) help_$${lang}.docbook ${DOCDIR}/$${lang}/$(PACKAGE)/index.docbook; \
	    $(INSTALL_DATA) $${lang}/*.png ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	    if test ! -e ${DOCDIR}/$${lang}/$(PACKAGE)/common ; then \
	    rm -f ${DOCDIR}/$${lang}/$(PACKAGE)/common ; fi ; \
	    ( cd ${DOCDIR}/$${lang}/${PACKAGE} ; ln -s ../common . ) ; \
	}; done
	cd ${DOCDIR} ; \
	if test ! -e default/$(PACKAGE) -a -e default ; then ln -s ../en/$(PACKAGE) default; fi;

uninstall-local:
	for lang in ${LANGUAGES}; do { \
	    rm -R -f ${DOCDIR}/$${lang}/${PACKAGE} ; \
	    rmdir -p --ignore-fail-on-non-empty $(DOCDIR)/$$lang >/dev/null 2>&1; \
	    if test -L ${DOCDIR}/default/$(PACKAGE); then rm -f ${DOCDIR}/default/$(PACKAGE); fi; \
	}; done
