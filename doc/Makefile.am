
MAINTAINERCLEANFILES = de/*.html en/*.html
CLEANFILES = core *~ .deps/* doc/*~ *.orig changes.docbook de/*.html en/*.html
DISTCLEANFILES = Makefile.in Makefile

LANGUAGES = en de fr
SUFFIXES = .docbook .po .pot
SGMLDIR=$(kde_datadir)/ksgmltools2
DOCDIR = $(DESTDIR)${kde_htmldir}

# --- translated html help, not needed in distribution ---

de/index.html: help_de.docbook
	cd de ; $(MEINPROC) --check ../help_de.docbook
	rm -Rf de/common
	mkdir -p de/common
	cp -R ${DOCDIR}/de/common/* de/common/

fr/index.html: help_fr.docbook
	cd fr ; $(MEINPROC) --check ../help_fr.docbook
	rm -Rf fr/common
	mkdir -p fr/common
	cp -R ${DOCDIR}/fr/common/* fr/common/

it/index.html: help_it.docbook
	cd it ; $(MEINPROC) --check ../help_it.docbook
	rm -Rf it/common
	mkdir -p it/common
	cp -R ${DOCDIR}/it/common/* fr/common/

# --------------------------------------------------------

en/index.html: help_en.docbook
	cd en ; $(MEINPROC) --check ../help_en.docbook
	rm -Rf en/common
	mkdir -p en/common
	cp -R ${DOCDIR}/en/common/* en/common/
	cd en/common ; mv gpl-license.html gpl-translated.html

help_en.pot: help_en.docbook
	$(XML2POT) help_en.docbook > help_en.pot

html-doc: en/index.html de/index.html fr/index.html

html-clean:
	for lang in ${LANGUAGES}; do { \
	    rm -Rfv $${lang}/*.html; rm -Rf $${lang}/common; \
	}; done

# --- translated docbook online help ---

help_de.po: help_en.pot
	$(MSGMERGE) -o help_de.po help_de.po help_en.pot
	# kbabel -nofork help_de.po

help_fr.po: help_en.pot
	$(MSGMERGE) -o help_fr.po help_fr.po help_en.pot
	# kbabel -nofork help_fr.po

help_it.po: help_en.pot
	$(MSGMERGE) -o help_it.po help_it.po help_en.pot
	# kbabel -nofork help_it.po

help_de.docbook: help_en.docbook help_en.pot
	${MAKE} help_de.po
	@echo "converting german help"
	@echo "(this takes very very long, please be patient...)"
	@ $(PO2XML) help_en.docbook help_de.po | \
	sed s/^\ \ \<\!ENTITY\ %\ English\ /\ \ \<\!ENTITY\ %\ German\ /g > $@
	@CHECKXML@ $@

help_fr.docbook: help_en.docbook help_en.pot
	${MAKE} help_fr.po
	@echo "converting french help"
	@echo "(this takes very very long, please be patient...)"
	@ $(PO2XML) help_en.docbook help_fr.po | \
	sed s/^\ \ \<\!ENTITY\ %\ English\ /\ \ \<\!ENTITY\ %\ French\ /g > $@
	@CHECKXML@ $@

help_it.docbook: help_en.docbook help_en.pot
	${MAKE} help_it.po
	@echo "converting italian help"
	@echo "(this takes very very long, please be patient...)"
	@ $(PO2XML) help_en.docbook help_it.po | \
	sed s/^\ \ \<\!ENTITY\ %\ English\ /\ \ \<\!ENTITY\ %\ Italian\ /g > $@
	@CHECKXML@ $@

# --------------------------------------

changes.docbook: ../CHANGES ../bin/changes-to-docbook.sh
	cat ../CHANGES | sed s/\>\>\>//g | sed s/\<\<\<//g | \
	../bin/changes-to-docbook.sh | recode lat1..utf-8 > changes.docbook

help_en.docbook: ../bin/insert-changes.sh ../VERSION ../CHANGES
	${MAKE} changes.docbook
	cp help_en.docbook help_en.docbook.orig
	cat help_en.docbook.orig | \
	../bin/insert-changes.sh  changes.docbook | \
	awk -v newval=`date --iso` \
	'{ sub(/<date>.*<\/date>/, \
	"<date>"newval"</date>", $$0); print; }' | \
	awk -v newval=`cat ../VERSION` \
	'{ sub(/<releaseinfo>.*<\/releaseinfo>/, \
	"<releaseinfo>"newval"</releaseinfo>", $$0); print; }' > \
	help_en.docbook

doc: help_en.docbook help_de.docbook help_fr.docbook

all: doc

install-data-local:
	echo installing docs in $(DOCDIR)
	for lang in ${LANGUAGES}; do \
	{ \
	    $(mkinstalldirs) ${DOCDIR}/$${lang}/$(PACKAGE); \
	    $(INSTALL_DATA) help_$${lang}.docbook ${DOCDIR}/$${lang}/$(PACKAGE)/index.docbook; \
	    $(INSTALL_DATA) $${lang}/*.png ${DOCDIR}/$${lang}/$(PACKAGE) ; \
	    ( cd ${DOCDIR}/$${lang}/${PACKAGE} ; \
	    for file in `ls ../../en/${PACKAGE}/*.png`; do \
	    { \
		if test ! -e `basename $${file}` ; then ln -sf $${file} . ; fi; \
	    }; done ); \
	    if test ! -e ${DOCDIR}/$${lang}/$(PACKAGE)/common ; then \
	    rm -f ${DOCDIR}/$${lang}/$(PACKAGE)/common ; fi ; \
	    ( cd ${DOCDIR}/$${lang}/${PACKAGE} ; ln -sf ../common . ) ; \
	}; done
	cd ${DOCDIR} ; \
	if test ! -e default/$(PACKAGE) -a -e default ; then ln -sf ../en/$(PACKAGE) default; fi;

uninstall-local:
	for lang in ${LANGUAGES}; do { \
	    rm -R -f ${DOCDIR}/$${lang}/${PACKAGE} ; \
	    rmdir -p --ignore-fail-on-non-empty $(DOCDIR)/$$lang >/dev/null 2>&1; \
	    if test -L ${DOCDIR}/default/$(PACKAGE); then rm -f ${DOCDIR}/default/$(PACKAGE); fi; \
	}; done
