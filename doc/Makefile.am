MAINTAINERCLEANFILES = HTML/*

CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook

LANGUAGES = en de

SUFFIXES = .docbook

SGMLDIR=${kde_datadir}/ksgmltools

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

doc: doc.en doc.de

doc.en: help_en.docbook
	rm -f en/*.html
	jw -o en -c ${SGMLDIR}/catalog \
	-d ${SGMLDIR}/stylesheets/kde.dsl\#html \
	help_en.docbook

doc.de: help_de.docbook
	rm -f de/*.html
	jw -o de -c ${SGMLDIR}/catalog \
	-d ${SGMLDIR}/stylesheets/kde.dsl\#html \
	help_de.docbook

install-data-local:
	for lang in ${LANGUAGES}; do \
	{ \
	    $(mkinstalldirs) $(kde_htmldir)/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) $${lang}/*.html $(kde_htmldir)/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) $${lang}/*.html $(kde_htmldir)/$${lang}/$(PACKAGE); \
	    $(mkinstalldirs) $(kde_htmldir)/$${lang}/$(PACKAGE)/common ; \
	    $(INSTALL_DATA) $${lang}/common/*.png $(kde_htmldir)/$${lang}/$(PACKAGE)/common ; \
	    $(INSTALL_DATA) $${lang}/common/*.css $(kde_htmldir)/$${lang}/$(PACKAGE)/common ; \
	}; done
	cd $(kde_htmldir) ; \
	if test ! -e default/$(PACKAGE); then ln -s ../en/$(PACKAGE) default; fi;

uninstall-local:
	for lang in ${LANGUAGES}; do \
	{ \
	    rm -R -f ${kde_htmldir}/$${lang}/${PACKAGE} ; \
	    cd $(kde_htmldir) ; \
	    if test -L default/$(PACKAGE); then rm -f default/$(PACKAGE); fi; \
	}; done
