#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
SonagramPlugin::~SonagramPlugin()
{
    if (sonagram_window) delete sonagram_window;
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
	if (result) dlg->getParameters(*result);
    };

    delete dlg;
    return result;
}

//***************************************************************************
int SonagramPlugin::start(QStrList &params)
{
    debug("--- SonagramPlugin::start() ---");

    sonagram_window = new SonagramWindow(getSignalName());
    ASSERT(sonagram_window);
    if (sonagram_window) {

	// connect all needed signals
	connect(sonagram_window, SIGNAL(destroyed()),
	        this, SLOT(windowClosed()));

        QObject::connect((QObject*)&(getManager()),
	    SIGNAL(sigSignalNameChanged(const QString &)),
	    sonagram_window, SLOT(setName(const QString &)));

	// evaluate the parameter list
	if (params.count() != 2) return -EINVAL;
	
	bool ok;
	QString param;
	
	param = params.at(0);
	unsigned int points = param.toUInt(&ok);
	if (!ok) return -EINVAL;
	
	param = params.at(1);
	unsigned int type = param.toUInt(&ok);
	if (!ok) return -EINVAL;

	unsigned int l;
	unsigned int r;
	unsigned int length = getSelection(&l, &r);
	if (l == r) {
	    length = getSignalLength()-1;
	    l = 0;
	    r = length-1;
	}
	
	int len=( (length/(points/2)) * (points/2) ) + (points/2) ;
	debug("SonagramPlugin::start: filling input data length=%d, len=%d",
	length,len);
	
	double *input;
	input = new double[len];
	ASSERT(input);
	if (!input) return -ENOMEM;

        int i;
	for (i=0;i<length;i++)
	    input[i] = (double)getSingleSample(0, l+i)/(double)(1<<23);

	debug("padding...");
	for (;i<len;i++) input[i]=(double)0.0; //pad with zeros...
	debug("padding done.");

	debug("SonagramPlugin::start: setting signal");
	
	// start the process
	sonagram_window->setSignal(input, length, points, type, getSignalRate());
	sonagram_window->show();
	
	delete[] input;
    }

    debug("SonagramPlugin::start() done.");

    return 0;
}

//***************************************************************************
void SonagramPlugin::windowClosed()
{
    // the SonagramWindow closes itself !
    sonagram_window = 0;

    // close the plugin too
    close();
}

//***************************************************************************
//***************************************************************************
