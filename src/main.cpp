//Kwave main file
#include "config.h"
#include <kapp.h>
#include "KwaveApp.h"

//*****************************************************************************
int main( int argc, char **argv )
{
    int result = -1;

    KwaveApp app(argc, argv);

    if (!(app.isOK())) {
	warning("main: cannot create application");
	app.quit();
    } else {
	app.connect(&app, SIGNAL(lastWindowClosed()),
	            &app, SLOT(quit()) );
	result = app.exec();
    }

    return result;
}
