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

#include <artsc/artsc.h> // for arts_init()
#include <arts/artsflow.h>

#include "KwaveApp.h"

static KCmdLineOptions options[] =
{
    { "!+files", I18N_NOOP("List of wav files."), 0 },
    { 0, 0, 0 } // End of options.
};

//***************************************************************************
int main( int argc, char **argv )
{
    /*
     * This is a work-around to avoid problems/crashes with the aRts
     * dispatcher. Maybe this can be removed when it gets possible
     * to create/delete Arts::Dispatcher objects without crashing in
     * Arts::GlobalX11Comm.  (maybe in KDE-3 ?)
     *
     * Meanwhile we avoid the creation of new new dispatchers and only
     * use the one that is created implicitely by this call to the artsc
     * interface.
     */
    int errorcode = arts_init();
    volatile Arts::Dispatcher *dispatcher;
    if (errorcode < 0) {
	warning("arts_init error: %s", arts_error_text(errorcode));
	dispatcher = new Arts::Dispatcher();
	ASSERT(dispatcher);
    }

    KAboutData about(PACKAGE, "Kwave", VERSION,
	"sound editor for KDE2",
	KAboutData::License_GPL_V2,
	"(c) 2001, Thomas Eschenbacher",
	0 /*"...text..." */,
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );

    about.addAuthor("Thomas Eschenbacher" ,
                    "project leader since 2000, core development",
                    "Thomas.Eschenbacher@gmx.de",
                    0);
    about.addAuthor("Martin Wilz" ,
                    "creator of the project, development 1998-2000",
                    "mwilz@ernie.MI.Uni-Koeln.DE",
                    "http://www.wilz.de");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KwaveApp::addCmdLineOptions();

//    KCrash::setCrashHandler(0);

#ifdef UNIQUE_APP
    if (!KUniqueApplication::start()) {
	warning("Kwave is already running!");
	exit(0);
    }
#endif // UNIQUE_APP

    KwaveApp app;
    app.exec();

    return 0;
}

//***************************************************************************
//***************************************************************************
