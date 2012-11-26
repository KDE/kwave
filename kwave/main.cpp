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

#include <QtCore/QString>

#include <kaboutdata.h>
#include <kuniqueapplication.h>
#include <kapplication.h>
#include <kcrash.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "App.h"
#include "Splash.h"

/**
 * add data concerning the developers and
 * contributers to the about data
 */
void addDataStrings(KAboutData &aboutdata)
{
    //Developers
    aboutdata.addAuthor(ki18n("Thomas Eschenbacher"),
                     ki18n("Project leader since 2000, core development"),
                     "Thomas.Eschenbacher@gmx.de",
                     "http://kwave.sourceforge.net");
    aboutdata.addAuthor(ki18n("Martin Wilz"),
                     ki18n("Creator of the project, development 1998-2000"),
                     "martin@wilz.de",
                     "http://www.wilz.de");
    aboutdata.addAuthor(ki18n("Ralf Waspe"),
                     ki18n("Creator of the Help/About dialog"),
                     "rwaspe@web.de",
                     0);
    aboutdata.addAuthor(ki18n("Caulier Gilles"),
                     ki18n("French translations, splashscreen, tests and bugfixes"),
                     "caulier.gilles@free.fr",
                     "http://caulier.gilles.free.fr");
    aboutdata.addAuthor(ki18n("Dave Flogeras"),
                     ki18n("Notch filter plugin"),
                     "d.flogeras@unb.ca",
                     0);
    aboutdata.addAuthor(ki18n("Rik Hemsley"),
                     ki18n("Level meter"),
                     "rik@kde.org",
                     "http://rikkus.info/esoundlevelmeter.html");

    // translators
    aboutdata.setTranslator(ki18n("NAME OF TRANSLATORS"),
                            ki18n("EMAIL OF TRANSLATORS"));

    // people who helped
    aboutdata.addCredit(ki18n("Stefan Westerfeld"),
                     ki18n("Author of aRts"),
                     "stefan@space.twc.de",
                     0);
    aboutdata.addCredit(ki18n("Sven-Steffen Arndt"),
                     ki18n("Kwave homepage and German online help"),
                     "ssa29@gmx.de",
                     0);
    aboutdata.addCredit(ki18n("Aurelien Jarno"),
                     ki18n("Debian packager"),
                     "aurel32@debian.org",
                     0);
    aboutdata.addCredit(ki18n("Robert M. Stockmann"),
                     ki18n("Packaging for Mandrake / X86_64"),
                     "stock@stokkie.net",
                     0);
//     aboutdata.addCredit(ki18n("Jorge Luis Arzola"),
//                      ki18n("Packaging for SuSE Linux"),
//                      "arzolacub@gmx.de",
//                      0);
//     aboutdata.addCredit(ki18n("Diederick de Vries"),
//                      ki18n("Packaging for Crux Linux"),
//                      "diederick76@gmail.com",
//                      "http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave");
    aboutdata.addCredit(ki18n("Martin Kuball"),
                     ki18n("Tester"),
                     "makube@user.sourceforge.net",
                     0);
    aboutdata.addCredit(ki18n("Robert Leslie"),
                     ki18n("Author of the 'mad' MP3 decoder library"),
                     "rob@mars.org",
                     "http://www.mars.org/home/rob/proj/mpeg");
    aboutdata.addCredit(ki18n("T.H.F. Klok and Cedric Tefft"),
                     ki18n("Maintainers of the 'id3lib' library"),
                     0,
                     "http://www.id3lib.org/");
    aboutdata.addCredit(ki18n("Michael Pruett"),
                     ki18n("Author of the 'audiofile' library"),
                     "mpruett@sgi.com",
                     "http://www.68k.org/~michael/audiofile/");
    aboutdata.addCredit(ki18n("Erik de Castro Lopo"),
                     ki18n("Author of the 'sndfile' library"),
                     "erikd@zip.com.au",
                     "http://www.mega-nerd.com/libsndfile/");
    aboutdata.addCredit(ki18n("Pavel Fric"),
                     ki18n("Czech translation"),
                     "pavelfric@seznam.cz",
                     "http://fripohled.blogspot.com");
    aboutdata.addCredit(ki18n("Panagiotis Papadopoulos"),
                     ki18n("String and i18n updates"),
                     "pano_90@gmx.net",
                     0);

}

#ifdef HAVE_OPTIMIZED_MEMCPY
/* forward declaration to libkwave/memcpy.c */
extern "C" void probe_fast_memcpy(void);
#endif /* HAVE_OPTIMIZED_MEMCPY */

//***************************************************************************
int main( int argc, char **argv )
{
    static KCmdLineOptions options;

    options.add("!+files", ki18n("List of audio files"), 0 );

    KAboutData about(
	PACKAGE, "",
	ki18n("Kwave"),
	PACKAGE_VERSION,
	ki18n("A sound editor for KDE"),
	KAboutData::License_GPL_V2,
        ki18n("(c) 2011, Thomas Eschenbacher"),
	ki18n(0),
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );
    addDataStrings(about);

    /* show some version info */
    printf("\nThis is %s v%s (compiled for KDE %s)\n",
	about.productName().toAscii().data(),
	about.version().toAscii().data(),
	KDE_VERSION_STRING
    );

    /* process all interesting commandline parameters */
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    Kwave::App::addCmdLineOptions();

     /* check for an optimized version of memcpy() */
#ifdef HAVE_OPTIMIZED_MEMCPY
    probe_fast_memcpy();
    printf("\n");
#endif /* HAVE_OPTIMIZED_MEMCPY */

    if (!KUniqueApplication::start()) {
	qWarning("Kwave is already running!");
	exit(0);
    }

    Kwave::App app;
    Kwave::Splash splash("pics/kwave-splash.png");
    splash.show();

    return app.exec();
}

//***************************************************************************
//***************************************************************************
