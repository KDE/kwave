/***************************************************************************
                    KwavePlugin.cpp  -  New Interface for Kwave plugins
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#include <qwidget.h>

#include <kapp.h>

#include <libgui/KwavePlugin.h>
#include <libgui/PluginContext.h>

#include "../src/TopWidget.h"
#include "../src/SignalManager.h"

//***************************************************************************
KwavePlugin::KwavePlugin(PluginContext &c)
    :context(c)
{
}

//***************************************************************************
KwavePlugin::~KwavePlugin()
{
    debug("KwavePlugin::~KwavePlugin()");

    // inform our owner that we close. This allows the plugin to
    // delete itself
    close();
    debug("KwavePlugin::~KwavePlugin(), done.");
}

//***************************************************************************
QStrList *KwavePlugin::setup(QStrList *previous_params = 0)
{
    QStrList *result = new QStrList();
    ASSERT(result);
    return result;
}

//***************************************************************************
int KwavePlugin::start(QStrList &params)
{
    return 0;
}

//***************************************************************************
int KwavePlugin::execute(QStrList &params)
{
    return run(params);
}

//***************************************************************************
int KwavePlugin::run(QStrList &params)
{
    return 0;
}

//***************************************************************************
void KwavePlugin::close()
{
    debug("void KwavePlugin::close() [slot]");
    emit sigClosed(this, true);
}

//***************************************************************************
QWidget *KwavePlugin::getParentWidget()
{
    return &(context.top_widget);
}

//***************************************************************************
const QString &KwavePlugin::getSignalName()
{
    return (context.top_widget.getSignalName());
}

//***************************************************************************
unsigned int KwavePlugin::getSelection(unsigned int *left,
                                       unsigned int *right)
{
    int l = 0;
    int r = 0;

    ASSERT(context.signal_manager);
    if (context.signal_manager) {
	l = context.signal_manager->getLMarker();
	r = context.signal_manager->getRMarker();
    }

    if (left)  *left  = l;
    if (right) *right = r;
    return r-l+1;
}

//***************************************************************************
int KwavePlugin::getRate()
{
    ASSERT(context.signal_manager);
    return context.signal_manager ? context.signal_manager->getRate() : 0;
}

//***************************************************************************
void *KwavePlugin::getHandle()
{
    return context.handle;
}

//***************************************************************************
void KwavePlugin::zoom2string(char *buf, unsigned int bufsize, double percent)
{
    if (percent < 1.0) {
	char format[128];
	int digits = (int)ceil(1.0 - log10(percent));

	snprintf(format, sizeof(format), "%%0.%df %%%%", digits);
	snprintf(buf, bufsize, format, percent);
    } else if (percent < 10.0) {
	snprintf(buf, bufsize, "%0.1f %%", percent);
    } else if (percent < 1000.0) {
	snprintf(buf, bufsize, "%0.0f %%", percent);
    } else {
	snprintf(buf, bufsize, "x %d", (int)(percent / 100.0));
    }
}

//***************************************************************************
void KwavePlugin::ms2string(char *buf, unsigned int bufsize, double ms)
{
    if (ms < 1.0) {
	char format[128];
	int digits = (int)ceil(1.0 - log10(ms));

	snprintf(format, sizeof(format), "%%0.%df ms", digits);
	snprintf(buf, bufsize, format, ms);
    } else if (ms < 1000.0) {
	snprintf(buf, bufsize, "%0.1f ms", ms);
    } else {
	int s = (int)round(ms / 1000);
	int m = (int)floor(s / 60);
	
	if (m < 1) {
	    char format[128];
	    int digits = (int)ceil(7.0 - log10(ms));
	    snprintf(format, sizeof(format), "%%0.%df s", digits);
	    snprintf(buf, bufsize, format, ms / 1000.0);
	} else {
	    snprintf(buf, bufsize, "%02d:%02d min", m, s % 60);
	}
    }
}

//***************************************************************************
//***************************************************************************
/* end of libgui/KwavePlugin.cpp */
