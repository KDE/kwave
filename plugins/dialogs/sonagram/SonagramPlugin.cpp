#include <stdio.h>
#include <stdlib.h>

#include <libkwave/WindowFunction.h>
#include <libgui/KwavePlugin.h>
#include <libgui/PluginContext.h>
#include <kapp.h>

#include "SonagramPlugin.h"
#include "SonagramDialog.h"
#include "SonagramWindow.h"

KWAVE_PLUGIN(SonagramPlugin,"sonagram","Martin Wilz");

//***************************************************************************
SonagramPlugin::SonagramPlugin(PluginContext &c)
    :KwavePlugin(c)
{
    sonagram_window = 0;
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
int SonagramPlugin::start(QStrList &params)
{
    debug("--- SonagramPlugin::start() ---");

    QString n("sonagram of something...");
    sonagram_window = new SonagramWindow(&n);
    ASSERT(sonagram_window);
    if (sonagram_window) {
	sonagram_window->show();
	// delete sono;
    }

    debug("### SonagramPlugin::run() done. ###");
    return 0;
}

//***************************************************************************
void SonagramPlugin::close()
{
    debug("void SonagramPlugin::close()");
    if (sonagram_window) delete sonagram_window;
    sonagram_window = 0;
}

//***************************************************************************
//***************************************************************************
