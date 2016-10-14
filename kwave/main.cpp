/***************************************************************************
                main.cpp -  Kwave main program
			     -------------------
    begin                : Wed Jul 15 1998
    copyright            : (C) 1998 by Martin Wilz
    email                : mwilz@ernie.mi.uni-koeln.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <errno.h>
#include <stdio.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QString>

#include <KAboutData>
#include <KCrash>
#include <kcrash_version.h>
#include <KDBusService>
#include <KLocalizedString>
#include <kxmlgui_version.h>

#include "libkwave/String.h"

#include "App.h"
#include "Splash.h"

/**
 * add data concerning the developers and
 * contributers to the about data
 */
static void addDataStrings(KAboutData &aboutdata)
{
    //Developers
    aboutdata.addAuthor(
	_(I18N_NOOP("Thomas Eschenbacher")),
	_(I18N_NOOP("Project leader since 2000, core development")),
	_("Thomas.Eschenbacher@gmx.de"),
	_(I18N_NOOP("http://kwave.sourceforge.net"))
    );
    aboutdata.addAuthor(
	_(I18N_NOOP("Martin Wilz")),
	_(I18N_NOOP("Creator of the project, development 1998-2000")),
	_("martin@wilz.de"),
	_(I18N_NOOP("http://www.wilz.de")));
    aboutdata.addAuthor(
	_(I18N_NOOP("Ralf Waspe")),
	_(I18N_NOOP("Creator of the Help/About dialog")),
	_("rwaspe@web.de"),
	QString());
    aboutdata.addAuthor(
	_(I18N_NOOP("Caulier Gilles")),
	_(I18N_NOOP("splashscreen, tests and bugfixes")),
	_("caulier.gilles@free.fr"),
	_(I18N_NOOP("http://caulier.gilles.free.fr")));
    aboutdata.addAuthor(
	_(I18N_NOOP("Dave Flogeras")),
	_(I18N_NOOP("Notch filter plugin")),
	_("d.flogeras@unb.ca"),
	QString());
    aboutdata.addAuthor(
	_(I18N_NOOP("Rik Hemsley")),
	_(I18N_NOOP("Level meter")),
	_("rik@kde.org"),
	_(I18N_NOOP("http://rikkus.info/esoundlevelmeter.html")));
    aboutdata.addAuthor(
	_(I18N_NOOP("Joerg-Christian Boehme")),
	_(I18N_NOOP("PulseAudio record plugin")),
	_("joerg@chaosdorf.de"),
	QString());

    // people who helped
    aboutdata.addCredit(
	_(I18N_NOOP("Stefan Westerfeld")),
	_(I18N_NOOP("Author of aRts")),
	_("stefan@space.twc.de"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Sven-Steffen Arndt")),
	_(I18N_NOOP("Kwave homepage and German online help")),
	_("ssa29@gmx.de"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Aurelien Jarno")),
	_(I18N_NOOP("Debian packager")),
	_("aurel32@debian.org"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Robert M. Stockmann")),
	_(I18N_NOOP("Packaging for Mandrake / X86_64")),
	_("stock@stokkie.net"),
	QString());
//     aboutdata.addCredit(I18N_NOOP("Diederick de Vries"),
//                      I18N_NOOP("Packaging for Crux Linux"),
//                      "diederick76@gmail.com",
//                      "http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave");
    aboutdata.addCredit(
	_(I18N_NOOP("Martin Kuball")),
	_(I18N_NOOP("Tester")),
	_("makube@user.sourceforge.net"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Robert Leslie")),
	_(I18N_NOOP("Author of the 'mad' MP3 decoder library")),
	_("rob@mars.org"),
	_(I18N_NOOP("http://www.mars.org/home/rob/proj/mpeg")));
    aboutdata.addCredit(
	_(I18N_NOOP("T.H.F. Klok and Cedric Tefft")),
	_(I18N_NOOP("Maintainers of the 'id3lib' library")),
	QString(),
	_(I18N_NOOP("http://www.id3lib.org/")));
    aboutdata.addCredit(
	_(I18N_NOOP("Michael Pruett")),
	_(I18N_NOOP("Author of the 'audiofile' library")),
	_("mpruett@sgi.com"),
	_(I18N_NOOP("http://www.68k.org/~michael/audiofile/")));
    aboutdata.addCredit(
	_(I18N_NOOP("Carlos R.")),
	_(I18N_NOOP("Spanish translation")),
	_("pureacetone@gmail.com"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Erik de Castro Lopo")),
	_(I18N_NOOP("Author of the 'sndfile' library")),
	_("erikd@zip.com.au"),
	_(I18N_NOOP("http://www.mega-nerd.com/libsndfile/")));
    aboutdata.addCredit(
	_(I18N_NOOP("Pavel Fric")),
	_(I18N_NOOP("Czech translation")),
	_("pavelfric@seznam.cz"),
	_(I18N_NOOP("http://fripohled.blogspot.com")));
    aboutdata.addCredit(
	_(I18N_NOOP("Panagiotis Papadopoulos")),
	_(I18N_NOOP("String and i18n updates")),
	_("pano_90@gmx.net"),
	QString());
}

#ifdef WITH_OPTIMIZED_MEMCPY
/* forward declaration to libkwave/memcpy.c */
extern "C" void probe_fast_memcpy(void);
#endif /* WITH_OPTIMIZED_MEMCPY */

//***************************************************************************
int main(int argc, char **argv)
{
    int retval = 0;

    // create the application instance first
    Kwave::App app(argc, argv);

    // initialize the crash handler (only if KCrash >= 5.15 is available)
#if KCrash_VERSION >= ((5 << 16) | (15 << 8) | (0))
    KCrash::initialize();
#endif

    // manually connect the translation catalog, otherwise i18n will not work
    KLocalizedString::setApplicationDomain(PACKAGE);

    QCommandLineParser cmdline;
    cmdline.addHelpOption();
    cmdline.addVersionOption();
    cmdline.addOption(QCommandLineOption(
	_("disable-splashscreen"),
	i18n("Disable the Splash Screen.")
    ));
    cmdline.addOption(QCommandLineOption(
	_("iconic"),
	i18n("Start Kwave iconified.")
    ));
    cmdline.addOption(QCommandLineOption(
	_("logfile"),
	i18nc("description of command line parameter",
	      "Log all commands into a file <file>."),
	i18nc("placeholder of command line parameter", "file")
    ));
    cmdline.addOption(QCommandLineOption(
	_("gui"),
	i18nc("description of command line parameter",
	      "Select a GUI type: SDI, MDI or TAB mode."),
	i18nc("placeholder of command line parameter", "sdi|mdi|tab")
    ));
    cmdline.addPositionalArgument(
	_("files"),
	i18nc("List of audio files, Kwave macro files ",
	      "or Kwave URLs to open (optionally)"),
	i18nc("placeholder of command line parameter", "[files...]")
    );

    KAboutData about(
	_(PACKAGE),
	i18n("Kwave"),
	_(PACKAGE_VERSION),
	i18n("A sound editor built on KDE Frameworks 5"),
	KAboutLicense::GPL_V2,
        i18n("(c) 2016, Thomas Eschenbacher"),
	QString(),
	_("http://kwave.sourceforge.net"),
	_("Thomas.Eschenbacher@gmx.de")
    );
    addDataStrings(about);

    /* use the about data above for this application */
    KAboutData::setApplicationData(about);

    /* show some version info */
    printf("\nThis is %s v%s (compiled for KDE Frameworks %s)\n",
	about.productName().toLatin1().data(),
	about.version().toLatin1().data(),
	KXMLGUI_VERSION_STRING
    );

    app.processCmdline(&cmdline);
    app.setApplicationName(_("kwave"));
    app.setApplicationVersion(_(PACKAGE_VERSION));
    app.setOrganizationDomain(_("kde.org"));
    cmdline.process(app);
    about.processCommandLine(&cmdline);

    /* let Kwave be a "unique" application, only one instance */
    KDBusService service(KDBusService::Unique);

     /* check for an optimized version of memcpy() */
#ifdef WITH_OPTIMIZED_MEMCPY
    probe_fast_memcpy();
    printf("\n");
#endif /* WITH_OPTIMIZED_MEMCPY */

    // check whether to start up without splash screen or in iconic mode
    // which implicitly also disables the splash screen
    Kwave::Splash splash(_("pics/kwave-splash.png"));
    bool show_splash = !(cmdline.isSet(_("disable-splashscreen")) ||
                         cmdline.isSet(_("iconic")));
    if (show_splash) splash.show();

    // now as the splash screen is in place, we can start a new instance
    retval = app.newInstance(app.arguments(), QString());

    QObject::connect(
	&service,
	SIGNAL(activateRequested(QStringList,QString)),
	&app,
	SLOT(newInstance(QStringList,QString))
    );

    if (retval != ECANCELED)
	retval = app.exec();

    splash.done();
    splash.close();

    return retval;
}

//***************************************************************************
//***************************************************************************
