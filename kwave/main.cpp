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

#include <kaboutdata.h>

#ifdef UNIQUE_APP
#include <kuniqueapp.h>
#else /* UNIQUE_APP */
#include <kapp.h>
#endif /* UNIQUE_APP */

#include <kcrash.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <arts/artsflow.h>

#include "KwaveApp.h"
#include "KwaveSplash.h"

static KCmdLineOptions options[] =
{
    { "!+files", I18N_NOOP("List of wav files."), 0 },
    { 0, 0, 0 } // End of options.
};

/**
 * Dummy define for the i18n macro. With this trick, we can use the
 * i18n() macro so that the various messages in here are recognized
 * by the i18n tools but the program does not crash when loaded.
 * Note: without this, the program would crash because the i18n
 * is called before the qApp is up!
 */
#define i18n(msg) msg

/**
 * add data concerning the developers and
 * contributers to the about data
 */
void addDataStrings(KAboutData& aboutdata)
{
    //Developers
    aboutdata.addAuthor("Thomas Eschenbacher" ,
                    i18n("project leader since 2000, core development"),
                    "Thomas.Eschenbacher@gmx.de",
                    0);
    aboutdata.addAuthor("Martin Wilz" ,
                    i18n("creator of the project, development 1998-2000"),
                    "martin@wilz.de",
                    "http://www.wilz.de");
    aboutdata.addAuthor("Ralf Waspe" ,
                    i18n("creator of the Help/About dialog"),
                    "rwaspe@web.de",
                    0);
    aboutdata.addAuthor("Caulier Gilles",
	        i18n("french translations, splashscreen, tests and bugfixes"),
                     "caulier.gilles@free.fr",
                     "http://caulier.gilles.free.fr");
    aboutdata.addAuthor("Dave Flogeras",
                     i18n("notch filter plugin"),
                     "d.flogeras@unb.ca",
                     0);
    aboutdata.addAuthor("Rik Hemsley",
                     i18n("level meter"),
                     "rik@kde.org",
                     "http://rikkus.info/esoundlevelmeter.html");

    // people who helped
    aboutdata.addCredit("Stefan Westerfeld",
		     i18n("author of aRts"),
		     "stefan@space.twc.de",
		     0);
    aboutdata.addCredit("Sven-Steffen Arndt",
                     i18n("Kwave homepage and german online help"),
                     "ssa29@gmx.de",
                     0);
    aboutdata.addCredit("Aurelien Jarno",
                     i18n("debian packager"),
                     "aurel32@debian.org",
                     0);
    aboutdata.addCredit("Robert M. Stockmann",
                     i18n("packaging for Mandrake / X86_64"),
                     "stock@stokkie.net",
                     0);
    aboutdata.addCredit("Jorge Luis Arzola",
                     i18n("packaging for SuSE Linux"),
                     "arzolacub@gmx.de",
                     0);
    aboutdata.addCredit("Michael Favreau",
                     i18n("packaging for Arch Linux"),
                     "michel.favreau@free.fr",
                     0);
    aboutdata.addCredit("Diederick de Vries",
                     i18n("packaging for Crux Linux"),
                     "diederick76@gmail.com",
                     "http://crux.nu/portdb/?command=viewport&repo=diederick&name=kwave");
    aboutdata.addCredit("Martin Kuball",
                     i18n("tester"),
                     "makube@user.sourceforge.net",
                     0);
    aboutdata.addCredit("Robert Leslie",
                     i18n("author of the 'mad' mp3 decoder library"),
                     "rob@mars.org",
                     "http://www.mars.org/home/rob/proj/mpeg");
    aboutdata.addCredit("T.H.F. Klok and Cedric Tefft",
                     i18n("maintainers of the 'id3lib' library"),
                     0,
                     "http://www.id3lib.org/");
    aboutdata.addCredit("Michael Pruett",
                     i18n("author of the 'audiofile' library"),
                     "mpruett@sgi.com",
                     "http://oss.sgi.com/projects/audiofile/");
    aboutdata.addCredit("Erik de Castro Lopo",
                     i18n("author of the 'sndfile' library"),
                     "erikd@zip.com.au",
                     "http://www.zip.com.au/~erikd/libsndfile/");

}

/* forward declaration to libkwave/memcpy.c */
extern "C" void probe_fast_memcpy(void);

//***************************************************************************
int main( int argc, char **argv )
{
    KAboutData about(
	PACKAGE,
	"Kwave",
	PACKAGE_VERSION,
	i18n("sound editor for KDE"),
	KAboutData::License_GPL_V2,
        "(c) 2006, Thomas Eschenbacher",
	"", //TODO : i18n("");
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );
    addDataStrings(about);

    /* show some version info */
    QString kde_version = QString::fromLatin1(KDE_VERSION_STRING);
    QString kwave_version = about.programName() +
        " " + about.version();
    QString version_text = QString(i18n("This is %1 (using KDE %2)")).arg(
	kwave_version).arg(kde_version);
    printf("\n%s\n", version_text.local8Bit().data());

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

    static KwaveSplash *splash = new KwaveSplash("pics/kwave-splash.png");
    Q_ASSERT(splash);
    if (splash) splash->show();

    return app.exec();
}

//***************************************************************************
//***************************************************************************
