//Kwave main file
#include "config.h"

#include <kuniqueapp.h>
#include <kapp.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "KwaveApp.h"


static KCmdLineOptions options[] =
{
{ "a", I18N_NOOP("A short binary option."), 0 },
{ 0, 0, 0 } // End of options.
};


//*****************************************************************************
int main( int argc, char **argv )
{
    // Initialize command line args
    KCmdLineArgs::init(argc, argv, "kwave", "Sound Editor", "version");

    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions( options );

    // Add options from other components
    KApplication::addCmdLineOptions();

//    ....

    // Create application object without passing 'argc' and 'argv' again.
    KwaveApp app(0,0);

//    ....

    // Handle our own options/argments
    // A KApplication will usually do this in main but this is not
    // necassery.
    // A KUniqueApplication might want to handle it in newInstance().

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

//    // A binary option (on / off)
//    if (args->isSet("some-option"))
//    ....

//    // An option which takes an additional argument
//    QCString anotherOptionArg = args->getOption("another-option");
//
//    // Arguments (e.g. files to open)
//    for(int i = 0; i < args->count(); i++) // Counting start at 0!
//    {
//	// don't forget to convert to Unicode!
	app.exec();
//	// Or more convenient:
//	// openURL( args->url(i));
//    }

    args->clear(); // Free up some memory.


    return 0;
//    int result = -1;
//
//    KwaveApp app(argc, argv);
//
//    if (!(app.isOK())) {
//	warning("main: cannot create application");
//	app.quit();
//    } else {
//	app.connect(&app, SIGNAL(lastWindowClosed()),
//	            &app, SLOT(quit()) );
//	result = app.exec();
//    }
//    debug("Kwave: graceful shutdown, return code = %d", result);
//    return result;

}
