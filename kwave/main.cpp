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

#include <kaboutdata.h>
#include <kuniqueapplication.h>
#include <kapplication.h>
#include <kcrash.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "KwaveApp.h"
#include "KwaveSplash.h"

/**
 * add data concerning the developers and
 * contributers to the about data
 */
void addDataStrings(KAboutData& aboutdata)
{
    //Developers
    aboutdata.addAuthor(ki18n("Thomas Eschenbacher"),
                     ki18n("project leader since 2000, core development"),
                     "Thomas.Eschenbacher@gmx.de",
                     0);
    aboutdata.addAuthor(ki18n("Martin Wilz"),
                     ki18n("creator of the project, development 1998-2000"),
                     "martin@wilz.de",
                     "http://www.wilz.de");
    aboutdata.addAuthor(ki18n("Ralf Waspe"),
                     ki18n("creator of the Help/About dialog"),
                     "rwaspe@web.de",
                     0);
    aboutdata.addAuthor(ki18n("Caulier Gilles"),
                     ki18n("french translations, splashscreen, tests and bugfixes"),
                     "caulier.gilles@free.fr",
                     "http://caulier.gilles.free.fr");
    aboutdata.addAuthor(ki18n("Dave Flogeras"),
                     ki18n("notch filter plugin"),
                     "d.flogeras@unb.ca",
                     0);
    aboutdata.addAuthor(ki18n("Rik Hemsley"),
                     ki18n("level meter"),
                     "rik@kde.org",
                     "http://rikkus.info/esoundlevelmeter.html");

    // translators
    aboutdata.addAuthor(ki18n("Pavel Fric"),
                     ki18n("czech translation"),
                     "pavelfric@seznam.cz",
                     0);

    // people who helped
    aboutdata.addCredit(ki18n("Stefan Westerfeld"),
                     ki18n("author of aRts"),
                     "stefan@space.twc.de",
                     0);
    aboutdata.addCredit(ki18n("Sven-Steffen Arndt"),
                     ki18n("Kwave homepage and german online help"),
                     "ssa29@gmx.de",
                     0);
    aboutdata.addCredit(ki18n("Aurelien Jarno"),
                     ki18n("debian packager"),
                     "aurel32@debian.org",
                     0);
    aboutdata.addCredit(ki18n("Robert M. Stockmann"),
                     ki18n("packaging for Mandrake / X86_64"),
                     "stock@stokkie.net",
                     0);
    aboutdata.addCredit(ki18n("Jorge Luis Arzola"),
                     ki18n("packaging for SuSE Linux"),
                     "arzolacub@gmx.de",
                     0);
    aboutdata.addCredit(ki18n("Diederick de Vries"),
                     ki18n("packaging for Crux Linux"),
                     "diederick76@gmail.com",
                     "http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave");
    aboutdata.addCredit(ki18n("Martin Kuball"),
                     ki18n("tester"),
                     "makube@user.sourceforge.net",
                     0);
    aboutdata.addCredit(ki18n("Robert Leslie"),
                     ki18n("author of the 'mad' mp3 decoder library"),
                     "rob@mars.org",
                     "http://www.mars.org/home/rob/proj/mpeg");
    aboutdata.addCredit(ki18n("T.H.F. Klok and Cedric Tefft"),
                     ki18n("maintainers of the 'id3lib' library"),
                     0,
                     "http://www.id3lib.org/");
    aboutdata.addCredit(ki18n("Michael Pruett"),
                     ki18n("author of the 'audiofile' library"),
                     "mpruett@sgi.com",
                     "http://oss.sgi.com/projects/audiofile/");
    aboutdata.addCredit(ki18n("Erik de Castro Lopo"),
                     ki18n("author of the 'sndfile' library"),
                     "erikd@zip.com.au",
                     "http://www.zip.com.au/~erikd/libsndfile/");

}

/* forward declaration to libkwave/memcpy.c */
extern "C" void probe_fast_memcpy(void);

//***************************************************************************
int main( int argc, char **argv )
{
    static KCmdLineOptions options;

    options.add("!+files", ki18n("List of wav files."), 0 );

    KAboutData about(
	PACKAGE, "",
	ki18n("Kwave"),
	PACKAGE_VERSION,
	ki18n("a sound editor for KDE"),
	KAboutData::License_GPL_V2,
        ki18n("(c) 2006, Thomas Eschenbacher"),
	ki18n(""),
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );
    addDataStrings(about);

    /* show some version info */
    QString kde_version = QString::fromLatin1(KDE_VERSION_STRING);
    QString version_text = i18n("This is %1 v%2 (compiled for KDE %3)",
	about.programName(), about.version(), kde_version);
    printf("\n%s\n", version_text.toLocal8Bit().data());

    /* process all interesting commandline parameters */
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KwaveApp::addCmdLineOptions();

     /* check for an optimized version of memcpy() */
    probe_fast_memcpy();
    printf("\n");

#ifdef UNIQUE_APP
    if (!KUniqueApplication::start()) {
	qWarning("Kwave is already running!");
	exit(0);
    }
#endif // UNIQUE_APP

    KwaveApp app;
    KwaveSplash splash("pics/kwave-splash.png");
    splash.show();

    return app.exec();
}

//***************************************************************************
//***************************************************************************
