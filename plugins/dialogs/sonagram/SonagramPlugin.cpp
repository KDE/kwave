#include <stdio.h>
#include <stdlib.h>

#include <libkwave/WindowFunction.h>
#include <libgui/KwavePlugin.h>
#include <libgui/PluginContext.h>
#include <kapp.h>

#include "SonagramPlugin.h"
#include "SonagramDialog.h"


KWAVE_PLUGIN(SonagramPlugin,"sonagram","Martin Wilz");


SonagramPlugin::SonagramPlugin(PluginContext *c)
    :KwavePlugin(c)
{
}

//***************************************************************************
QStrList *SonagramPlugin::setup(QStrList *previous_params)
{
    QStrList *result = 0;

    SonagramDialog *dlg = new SonagramDialog(*this);
    ASSERT(dlg);
    if (!dlg) return 0;

    if (dlg->exec() == QDialog::Accepted) {
	result = new QStrList();
	ASSERT(result);
	if (result) {
	    result->append("123");
	}
    };

    delete dlg;
    return result;
}

//***************************************************************************
int SonagramPlugin::execute(QStrList *params)
{
    return 0;
}

//***************************************************************************
const char *FFT_Sizes[] =
	{"64", "128", "256", "512", "1024", "2048", "4096", 0};

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif


//***************************************************************************
//***************************************************************************
