MAINTAINERCLEANFILES = HTML/*

CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook

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

doc: doc.en

doc.en: help_en.docbook
	rm -f en/*.html
	jw -o en -c ${SGMLDIR}/catalog \
	-d ${SGMLDIR}/stylesheets/kde.dsl\#html \
	help_en.docbook

#doc.de: help_de.docbook
	# not done yet

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/$(PACKAGE)
	$(INSTALL_DATA) en/*.html $(kde_htmldir)/en/$(PACKAGE)
	$(mkinstalldirs) $(kde_htmldir)/en/$(PACKAGE)/common
	$(INSTALL_DATA) en/common/*.png $(kde_htmldir)/en/$(PACKAGE)/common
	$(INSTALL_DATA) en/common/*.css $(kde_htmldir)/en/$(PACKAGE)/common
	cd $(kde_htmldir) ; \
	if test ! -e default/$(PACKAGE); then ln -s ../en/$(PACKAGE) default; fi; \
	if test ! -e de/$(PACKAGE);      then ln -s ../en/$(PACKAGE) de;      fi;

uninstall-local:
	-rm -R -f ${kde_htmldir}/en/${PACKAGE}
	cd $(kde_htmldir) ; \
	if test -L default/$(PACKAGE); then rm -f default/$(PACKAGE); fi; \
	if test -L de/$(PACKAGE);      then rm -f de/$(PACKAGE);      fi;

