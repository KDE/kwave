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

#include <stdio.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QString>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

#include "libkwave/String.h"

#include "App.h"
#include "Splash.h"

/**
 * add data concerning the developers and
 * contributers to the about data
 */
// static void addDataStrings(KAboutData &aboutdata)
// {
//     //Developers
//     aboutdata.addAuthor(ki18n("Thomas Eschenbacher"),
//                      ki18n("Project leader since 2000, core development"),
//                      "Thomas.Eschenbacher@gmx.de",
//                      "http://kwave.sourceforge.net");
//     aboutdata.addAuthor(ki18n("Martin Wilz"),
//                      ki18n("Creator of the project, development 1998-2000"),
//                      "martin@wilz.de",
//                      "http://www.wilz.de");
//     aboutdata.addAuthor(ki18n("Ralf Waspe"),
//                      ki18n("Creator of the Help/About dialog"),
//                      "rwaspe@web.de",
//                      0);
//     aboutdata.addAuthor(ki18n("Caulier Gilles"),
//                      ki18n("French translations, splashscreen, tests and bugfixes"),
//                      "caulier.gilles@free.fr",
//                      "http://caulier.gilles.free.fr");
//     aboutdata.addAuthor(ki18n("Dave Flogeras"),
//                      ki18n("Notch filter plugin"),
//                      "d.flogeras@unb.ca",
//                      0);
//     aboutdata.addAuthor(ki18n("Rik Hemsley"),
//                      ki18n("Level meter"),
//                      "rik@kde.org",
//                      "http://rikkus.info/esoundlevelmeter.html");
//     aboutdata.addAuthor(ki18n("Joerg-Christian Boehme"),
//                      ki18n("PulseAudio record plugin"),
//                      "joerg@chaosdorf.de",
//                      0);
//
//     // people who helped
//     aboutdata.addCredit(ki18n("Stefan Westerfeld"),
//                      ki18n("Author of aRts"),
//                      "stefan@space.twc.de",
//                      0);
//     aboutdata.addCredit(ki18n("Sven-Steffen Arndt"),
//                      ki18n("Kwave homepage and German online help"),
//                      "ssa29@gmx.de",
//                      0);
//     aboutdata.addCredit(ki18n("Aurelien Jarno"),
//                      ki18n("Debian packager"),
//                      "aurel32@debian.org",
//                      0);
//     aboutdata.addCredit(ki18n("Robert M. Stockmann"),
//                      ki18n("Packaging for Mandrake / X86_64"),
//                      "stock@stokkie.net",
//                      0);
// //     aboutdata.addCredit(ki18n("Diederick de Vries"),
// //                      ki18n("Packaging for Crux Linux"),
// //                      "diederick76@gmail.com",
// //                      "http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave");
//     aboutdata.addCredit(ki18n("Martin Kuball"),
//                      ki18n("Tester"),
//                      "makube@user.sourceforge.net",
//                      0);
//     aboutdata.addCredit(ki18n("Robert Leslie"),
//                      ki18n("Author of the 'mad' MP3 decoder library"),
//                      "rob@mars.org",
//                      "http://www.mars.org/home/rob/proj/mpeg");
//     aboutdata.addCredit(ki18n("T.H.F. Klok and Cedric Tefft"),
//                      ki18n("Maintainers of the 'id3lib' library"),
//                      0,
//                      "http://www.id3lib.org/");
//     aboutdata.addCredit(ki18n("Michael Pruett"),
//                      ki18n("Author of the 'audiofile' library"),
//                      "mpruett@sgi.com",
//                      "http://www.68k.org/~michael/audiofile/");
//     aboutdata.addCredit(ki18n("Carlos R."),
//                      ki18n("Spanish translation"),
//                      "pureacetone@gmail.com",
//                      0);
//     aboutdata.addCredit(ki18n("Erik de Castro Lopo"),
//                      ki18n("Author of the 'sndfile' library"),
//                      "erikd@zip.com.au",
//                      "http://www.mega-nerd.com/libsndfile/");
//     aboutdata.addCredit(ki18n("Pavel Fric"),
//                      ki18n("Czech translation"),
//                      "pavelfric@seznam.cz",
//                      "http://fripohled.blogspot.com");
//     aboutdata.addCredit(ki18n("Panagiotis Papadopoulos"),
//                      ki18n("String and i18n updates"),
//                      "pano_90@gmx.net",
//                      0);
//
// }

#ifdef WITH_OPTIMIZED_MEMCPY
/* forward declaration to libkwave/memcpy.c */
extern "C" void probe_fast_memcpy(void);
#endif /* WITH_OPTIMIZED_MEMCPY */


#define OPTION(cmdline,option,description) \
    cmdline.addOption(QCommandLineOption(  \
	_(option), _(description)          \
    ))

//***************************************************************************
int main( int argc, char **argv )
{
    QCoreApplication::setApplicationName(i18n("Kwave"));
    QCoreApplication::setApplicationVersion(_(PACKAGE_VERSION));
    QCoreApplication::setOrganizationDomain(_("kde.org"));

    QCommandLineParser cmdline;
    cmdline.addHelpOption();
    cmdline.addVersionOption();
    OPTION(cmdline, "disable-splashscreen", "Disable the Splash Screen");
    OPTION(cmdline, "iconic",               "Start Kwave iconified");
    OPTION(cmdline, "logfile <file>",       "Log all commands into a file");
    OPTION(cmdline, "gui <sdi|mdi|tab>",    "GUI type: SDI, MDI or TAB mode");
    OPTION(cmdline, "!+files",              "List of audio files");

//     KAboutData about(
// 	PACKAGE, "",
// 	ki18n("Kwave"),
// 	PACKAGE_VERSION,
// 	ki18n("A sound editor for KDE"),
// 	KAboutLicense::License_GPL_V2,
//         ki18n("(c) 2014, Thomas Eschenbacher"),
// 	ki18n(0),
// 	"http://kwave.sourceforge.net",
// 	"Thomas.Eschenbacher@gmx.de"
//     );
//     addDataStrings(about);

    /* show some version info */
//     printf("\nThis is %s v%s (compiled for KDE %s)\n",
// 	about.productName().toLatin1().data(),
// 	about.version().toLatin1().data(),
// 	KDE_VERSION_STRING
//     );

    /* let Kwave be a "unique" application, only one instance */
    KDBusService service(KDBusService::Unique);

     /* check for an optimized version of memcpy() */
#ifdef WITH_OPTIMIZED_MEMCPY
    probe_fast_memcpy();
    printf("\n");
#endif /* WITH_OPTIMIZED_MEMCPY */

    Kwave::App app(argc, argv, cmdline);
    cmdline.process(app);

    // check whether to start up without splash screen or in iconic mode
    // which implicitly also disables the splash screen
    Kwave::Splash splash(_("pics/kwave-splash.png"));
    bool show_splash = !(cmdline.isSet(_("disable-splashscreen")) ||
                         cmdline.isSet(_("iconic")));
    if (show_splash) splash.show();

    int retval = app.exec();

    splash.done();
    splash.close();

    return retval;
}

//***************************************************************************
//***************************************************************************
