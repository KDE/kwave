//Kwave main file

#include <libkwave/Global.h>
#include "KwaveApp.h"

extern struct Global globals;
//*****************************************************************************
int main( int argc, char **argv )
{
    globals.app=new KwaveApp (argc, argv);
    if (globals.app) {
	int result=globals.app->exec();
	return result;
    }
    return -1;
}








