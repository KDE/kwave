
MAINTAINERCLEANFILES = de/*.html en/*.html
CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook de/*.html en/*.html
DISTCLEANFILES = Makefile.in Makefile

LANGUAGES = en de
SUFFIXES = .docbook .po .pot
SGMLDIR=$(kde_datadir)/ksgmltools2
DOCDIR = $(DESTDIR)${kde_htmldir}

de/index.html: help_de.docbook
	cd de ; \
	meinproc --check ../help_de.docbook ; \
	ln -s ${DOCDIR}/de/common .

en/index.html: help_en.docbook
	cd en ; \
	meinproc --check ../help_en.docbook ; \
	ln -s ${DOCDIR}/de/common .

help_en.pot: help_en.docbook
	xml2pot help_en.docbook > help_en.pot

help_de.po: help_en.pot
	msgmerge -o help_de.po help_de.po help_en.pot
	kbabel help_de.po

help_de.docbook: help_de.po help_en.docbook
	po2xml help_en.docbook help_de.po | \
	sed s/^\ \ \<\!ENTITY\ %\ English\ /\ \ \<\!ENTITY\ %\ German\ /g \
	> help_de.docbook

doc: help_de.docbook

html-doc: en/index.html de/index.html

all: doc

doc-clean:
	for lang in ${LANGUAGES}; do { rm -fv $${lang}/*.html; }; done

install-data-local:
	echo installing docs in $(DOCDIR)
	for lang in ${LANGUAGES}; do \
	{ \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) help_$${lang}.docbook ${DOCDIR}/$${lang}/$(PACKAGE)/index.docbook; \
	    $(INSTALL_DATA) $${lang}/*.png ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	    if test ! -e ${DOCDIR}/$${lang}/$(PACKAGE)/common ; then \
	    rm -f ${DOCDIR}/$${lang}/$(PACKAGE)/common ; fi ; \
	    ln -sf ${DOCDIR}/$${lang}/common ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	}; done
	cd ${DOCDIR} ; \
	if test ! -e default/$(PACKAGE) -a -e default ; then ln -s ../en/$(PACKAGE) default; fi;

uninstall-local:
	for lang in ${LANGUAGES}; do { \
	    rm -R -f ${DOCDIR}/$${lang}/${PACKAGE} ; \
	    rmdir -p --ignore-fail-on-non-empty $(DOCDIR)/$$lang >/dev/null 2>&1; \
	    if test -L ${DOCDIR}/default/$(PACKAGE); then rm -f ${DOCDIR}/default/$(PACKAGE); fi; \
	}; done
