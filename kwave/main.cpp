//Kwave main file

#include "config.h"
//#include "check/mcheck.h"

#include <kaboutdata.h>

#ifdef UNIQUE_APP
#include <kuniqueapp.h>
#else // UNIQUE_APP
#include <kapp.h>
#endif // UNIQUE_APP

#include <kcrash.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "KwaveApp.h"

static KCmdLineOptions options[] =
{
    { "!+files", I18N_NOOP("List of wav files."), 0 },
    { 0, 0, 0 } // End of options.
};

//***************************************************************************
int main( int argc, char **argv )
{
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
