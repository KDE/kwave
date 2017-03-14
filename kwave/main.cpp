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
	i18n("Thomas Eschenbacher"),
	i18n("Project leader since 2000, core development"),
	_("Thomas.Eschenbacher@gmx.de"),
	i18n("http://kwave.sourceforge.net")
    );
    aboutdata.addAuthor(
	i18n("Martin Wilz"),
	i18n("Creator of the project, development 1998-2000"),
	_("martin@wilz.de"),
	i18n("http://www.wilz.de"));
    aboutdata.addAuthor(
	i18n("Ralf Waspe"),
	i18n("Creator of the Help/About dialog"),
	_("rwaspe@web.de"),
	QString());
    aboutdata.addAuthor(
	i18n("Caulier Gilles"),
	i18n("splashscreen, tests and bugfixes"),
	_("caulier.gilles@free.fr"),
	i18n("http://caulier.gilles.free.fr"));
    aboutdata.addAuthor(
	i18n("Dave Flogeras"),
	i18n("Notch filter plugin"),
	_("d.flogeras@unb.ca"),
	QString());
    aboutdata.addAuthor(
	i18n("Rik Hemsley"),
	i18n("Level meter"),
	_("rik@kde.org"),
	i18n("http://rikkus.info/esoundlevelmeter.html"));
    aboutdata.addAuthor(
	i18n("Joerg-Christian Boehme"),
	i18n("PulseAudio record plugin"),
	_("joerg@chaosdorf.de"),
	QString());

    // people who helped
    aboutdata.addCredit(
	i18n("Stefan Westerfeld"),
	i18n("Author of aRts"),
	_("stefan@space.twc.de"),
	QString());
    aboutdata.addCredit(
	i18n("Sven-Steffen Arndt"),
	i18n("Kwave homepage and German online help"),
	_("ssa29@gmx.de"),
	QString());
    aboutdata.addCredit(
	i18n("Aurelien Jarno"),
	i18n("Debian packager"),
	_("aurel32@debian.org"),
	QString());
    aboutdata.addCredit(
	i18n("Robert M. Stockmann"),
	i18n("Packaging for Mandrake / X86_64"),
	_("stock@stokkie.net"),
	QString());
//     aboutdata.addCredit(i18n("Diederick de Vries"),
//                      i18n("Packaging for Crux Linux"),
//                      _("diederick76@gmail.com"),
//                      _("http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave"));
    aboutdata.addCredit(
	i18n("Martin Kuball"),
	i18n("Tester"),
	_("makube@user.sourceforge.net"),
	QString());
    aboutdata.addCredit(
	i18n("Robert Leslie"),
	i18n("Author of the 'mad' MP3 decoder library"),
	_("rob@mars.org"),
	i18n("http://www.mars.org/home/rob/proj/mpeg"));
    aboutdata.addCredit(
	i18n("T.H.F. Klok and Cedric Tefft"),
	i18n("Maintainers of the 'id3lib' library"),
	QString(),
	i18n("http://www.id3lib.org/"));
    aboutdata.addCredit(
	i18n("Michael Pruett"),
	i18n("Author of the 'audiofile' library"),
	_("mpruett@sgi.com"),
	i18n("http://www.68k.org/~michael/audiofile/"));
    aboutdata.addCredit(
	i18n("Carlos R."),
	i18n("Spanish translation"),
	_("pureacetone@gmail.com"),
	QString());
    aboutdata.addCredit(
	i18n("Erik de Castro Lopo"),
	i18n("Author of the 'sndfile' library"),
	_("erikd@zip.com.au"),
	i18n("http://www.mega-nerd.com/libsndfile/"));
    aboutdata.addCredit(
	i18n("Pavel Fric"),
	i18n("Czech translation"),
	_("pavelfric@seznam.cz"),
	i18n("http://fripohled.blogspot.com"));
    aboutdata.addCredit(
	i18n("Panagiotis Papadopoulos"),
	i18n("String and i18n updates"),
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
    KLocalizedString::setApplicationDomain(PROJECT_NAME);

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
	i18nc("description of command line parameter",
	      "List of audio files, Kwave macro files "\
	      "or Kwave URLs to open (optionally)"),
	i18nc("placeholder of command line parameter", "[files...]")
    );

    KAboutData about(
	_(PROJECT_NAME),
	i18n("Kwave"),
	_(KWAVE_VERSION),
	i18n("A sound editor built on KDE Frameworks 5"),
	KAboutLicense::GPL_V2,
        i18n("(c) 2016, Thomas Eschenbacher"),
	QString(),
	_("http://www.kde.org/applications/multimedia/kwave"),
	_("Thomas.Eschenbacher@gmx.de")
    );
    addDataStrings(about);

    /* use the about data above for this application */
    KAboutData::setApplicationData(about);

    /* show some version info */
    printf("\nThis is %s v%s (compiled with KDE Frameworks %s)\n",
	about.productName().toLatin1().data(),
	about.version().toLatin1().data(),
	KXMLGUI_VERSION_STRING
    );

    app.processCmdline(&cmdline);
    app.setApplicationName(_("kwave"));
    app.setApplicationVersion(_(KWAVE_VERSION));
    app.setOrganizationDomain(_("kde.org"));
    cmdline.process(app);
    about.setupCommandLine(&cmdline);
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
