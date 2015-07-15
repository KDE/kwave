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
	_("http://kwave.sourceforge.net")
    );
    aboutdata.addAuthor(
	_(I18N_NOOP("Martin Wilz")),
	_(I18N_NOOP("Creator of the project, development 1998-2000")),
	_("martin@wilz.de"),
	_("http://www.wilz.de"));
    aboutdata.addAuthor(
	_(I18N_NOOP("Ralf Waspe")),
	_(I18N_NOOP("Creator of the Help/About dialog")),
	_("rwaspe@web.de"),
	QString());
    aboutdata.addAuthor(
	_(I18N_NOOP("Caulier Gilles")),
	_(I18N_NOOP("splashscreen, tests and bugfixes")),
	_("caulier.gilles@free.fr"),
	_("http://caulier.gilles.free.fr"));
    aboutdata.addAuthor(
	_(I18N_NOOP("Dave Flogeras")),
	_(I18N_NOOP("Notch filter plugin")),
	_("d.flogeras@unb.ca"),
	QString());
    aboutdata.addAuthor(
	_(I18N_NOOP("Rik Hemsley")),
	_(I18N_NOOP("Level meter")),
	_("rik@kde.org"),
	_("http://rikkus.info/esoundlevelmeter.html"));
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
	_("http://www.mars.org/home/rob/proj/mpeg"));
    aboutdata.addCredit(
	_(I18N_NOOP("T.H.F. Klok and Cedric Tefft")),
	_(I18N_NOOP("Maintainers of the 'id3lib' library")),
	QString(),
	_("http://www.id3lib.org/"));
    aboutdata.addCredit(
	_(I18N_NOOP("Michael Pruett")),
	_(I18N_NOOP("Author of the 'audiofile' library")),
	_("mpruett@sgi.com"),
	_("http://www.68k.org/~michael/audiofile/"));
    aboutdata.addCredit(
	_(I18N_NOOP("Carlos R.")),
	_(I18N_NOOP("Spanish translation")),
	_("pureacetone@gmail.com"),
	QString());
    aboutdata.addCredit(
	_(I18N_NOOP("Erik de Castro Lopo")),
	_(I18N_NOOP("Author of the 'sndfile' library")),
	_("erikd@zip.com.au"),
	_("http://www.mega-nerd.com/libsndfile/"));
    aboutdata.addCredit(
	_(I18N_NOOP("Pavel Fric")),
	_(I18N_NOOP("Czech translation")),
	_("pavelfric@seznam.cz"),
	_("http://fripohled.blogspot.com"));
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


#define OPTION(option,description) \
    cmdline.addOption(QCommandLineOption(  \
	_(option), _(description)          \
    ))

//***************************************************************************
int main(int argc, char **argv)
{
    QCommandLineParser cmdline;
    cmdline.addHelpOption();
    cmdline.addVersionOption();
    OPTION("disable-splashscreen", I18N_NOOP("Disable the Splash Screen"));
    OPTION("iconic",               I18N_NOOP("Start Kwave iconified"));
    OPTION("logfile",              I18N_NOOP("Log all commands into a file"));   //  <file>
    OPTION("gui",                  I18N_NOOP("GUI type: SDI, MDI or TAB mode")); //  <sdi|mdi|tab>
    OPTION("files",                I18N_NOOP("List of audio files"));            //  !+files

    KAboutData about(
	_(PACKAGE),
	i18n("Kwave"),
	_(PACKAGE_VERSION),
	i18n("A sound editor for KDE"),
	KAboutLicense::GPL_V2,
        i18n("(c) 2015, Thomas Eschenbacher"),
	QString(),
	_("http://kwave.sourceforge.net"),
	_("Thomas.Eschenbacher@gmx.de")
    );
    addDataStrings(about);

    /* use the about data above for this application */
    KAboutData::setApplicationData(about);

    /* show some version info */
    printf("\nThis is %s v%s (compiled for KDE %s)\n",
	about.productName().toLatin1().data(),
	about.version().toLatin1().data(),
	KXMLGUI_VERSION_STRING
    );

    Kwave::App app(argc, argv, cmdline);
    app.setApplicationName(_("kwave"));
    app.setApplicationVersion(_(PACKAGE_VERSION));
    app.setOrganizationDomain(_("kde.org"));
    cmdline.process(app);

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

    QObject::connect(
	&service,
	SIGNAL(activateRequested(const QStringList &, const QString &)),
	&app,
	SLOT(newInstance(const QStringList &, const QString &))
    );

    int retval = app.exec();

    splash.done();
    splash.close();

    return retval;
}

//***************************************************************************
//***************************************************************************
