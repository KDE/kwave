
MAINTAINERCLEANFILES = de/*.html en/*.html
CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook

LANGUAGES = en de
SUFFIXES = .docbook
SGMLDIR=${kde_datadir}/ksgmltools
DOCDIR = $(DESTDIR)${kde_htmldir}

changes.docbook: ../CHANGES ../bin/changes-to-docbook.sh
	cat ../CHANGES | ../bin/changes-to-docbook.sh > changes.docbook

help_en.docbook: changes.docbook ../bin/insert-changes.sh ../VERSION
	cp help_en.docbook help_en.docbook.orig
	cat help_en.docbook.orig | \
	../bin/insert-changes.sh  changes.docbook | \
	awk -v newval=`date --iso` \
	'{ sub(/<DATE>.*<\/DATE>/, \
	"<DATE>"newval"</DATE>", $$0); print; }' | \
	awk -v newval=`cat ../VERSION` \
	'{ sub(/<RELEASEINFO>.*<\/RELEASEINFO>/, \
	"<RELEASEINFO>"newval"</RELEASEINFO>", $$0); print; }' > \
	help_en.docbook

help_de.docbook: changes.docbook ../bin/insert-changes.sh ../VERSION
	cp help_de.docbook help_de.docbook.orig
	cat help_de.docbook.orig | \
	../bin/insert-changes.sh  changes.docbook | \
	awk -v newval=`date --iso` \
	'{ sub(/<DATE>.*<\/DATE>/, \
	"<DATE>"newval"</DATE>", $$0); print; }' | \
	awk -v newval=`cat ../VERSION` \
	'{ sub(/<RELEASEINFO>.*<\/RELEASEINFO>/, \
	"<RELEASEINFO>"newval"</RELEASEINFO>", $$0); print; }' > \
	help_de.docbook

doc.en:
	rm -f en/*.html
	${JW} -o en -c ${SGMLDIR}/catalog \
	-d ${SGMLDIR}/stylesheets/kde.dsl\#html \
	help_en.docbook

doc.de:
	rm -f de/*.html
	${JW} -o de -c ${SGMLDIR}/catalog \
	-d ${SGMLDIR}/stylesheets/kde.dsl\#html \
	help_de.docbook

doc.timestamp: help_en.docbook help_de.docbook
	${MAKE} doc.en
	${MAKE} doc.de
	date > doc.timestamp

all: doc.timestamp

install-data-local:
	echo installing docs in $(DOCDIR)

	for lang in ${LANGUAGES}; do \
	{ \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) help_$${lang}.docbook ${DOCDIR}/$${lang}/$(PACKAGE)/index.docbook; \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	    $(INSTALL_DATA) $${lang}/*.html ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE)/common ; \
	    $(INSTALL_DATA) $${lang}/common/*.png ${DOCDIR}/$${lang}/$(PACKAGE)/common ; \
	}; done
	cd ${DOCDIR} ; \
	if test ! -e default/$(PACKAGE) -a -e default ; then ln -s ../en/$(PACKAGE) default; fi;

uninstall-local:
	for lang in ${LANGUAGES}; do { \
	    rm -R -f ${DOCDIR}/$${lang}/${PACKAGE} ; \
	    rmdir -p --ignore-fail-on-non-empty $(DOCDIR)/$$lang >/dev/null 2>&1; \
	    if test -L ${DOCDIR}/default/$(PACKAGE); then rm -f ${DOCDIR}/default/$(PACKAGE); fi; \
	}; done
