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
    { "a", I18N_NOOP("A short binary option."), 0 },
    { "+[arg1]", I18N_NOOP("An optional argument 'arg1'."), 0 },
    { "!+command", I18N_NOOP("A required argument 'command', that can contain multiple words, even starting with '-'."), 0 },
    { 0, 0, 0 } // End of options.
};

//***************************************************************************
int main( int argc, char **argv )
{
    debug("main() --1--"); // ###

    KAboutData about("Kwave",
	i18n("Kwave"),
	"0.5.99",
	i18n("sound editor for KDE2"),
	KAboutData::License_GPL_V2,
	i18n("(c) 2001, Thomas Eschenbacher"),
	"...text...",
	"http://kwave.sourceforge.net",
	"Thomas.Eschenbacher@gmx.de"
    );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KwaveApp::addCmdLineOptions();

    debug("main() --2--"); // ###

#ifdef UNIQUE_APP
    if (!KUniqueApplication::start()) {
	warning("myAppName is already running!\n");
	exit(0);
    }
#endif // UNIQUE_APP

    KwaveApp app;
//	app.connect(&app, SIGNAL(lastWindowClosed()),
//	            &app, SLOT(quit()) );
    debug("main() --3--"); // ###
    app.exec();

    return 0;
}

//***************************************************************************
//***************************************************************************
