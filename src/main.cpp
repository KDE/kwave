//Kwave main file

#include "config.h"
#include "KwaveApp.h"

//*****************************************************************************
int main( int argc, char **argv )
{
    KwaveApp app(argc, argv);
    int result = -1;

    app.connect(&app, SIGNAL(lastWindowClosed()),
                &app, SLOT(quit()) );

    if (!(app.isOK())) {
	warning("main: cannot create application");
	app.quit();
    } else {
	result = app.exec();
    }

    return result;
}
