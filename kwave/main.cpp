//Kwave main file

#include "config.h"

#include <kaboutdata.h>

#ifdef UNIQUE_APP
#include <kuniqueapp.h>
#else // UNIQUE_APP
#include <kapp.h>
#endif // UNIQUE_APP

#include <klocale.h>
#include <kcmdlineargs.h>

#include "KwaveApp.h"

static KCmdLineOptions options[] =
{
    { "!+files", I18N_NOOP("list of wav files."), 0 },
    { 0, 0, 0 } // End of options.
};

//***************************************************************************
int main( int argc, char **argv )
{
    KAboutData about("Kwave",
	"Kwave",
	"0.5.99",
	I18N_NOOP("sound editor for KDE2"),
	KAboutData::License_GPL_V2,
	I18N_NOOP("(c) 2001, Thomas Eschenbacher"),
	I18N_NOOP("...text..."),
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KwaveApp::addCmdLineOptions();

#ifdef UNIQUE_APP
    if (!KUniqueApplication::start()) {
	warning("myAppName is already running!\n");
	exit(0);
    }
#endif // UNIQUE_APP

    KwaveApp app;
    app.exec();
}

//***************************************************************************
//***************************************************************************
