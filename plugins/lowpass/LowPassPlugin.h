/***************************************************************************
        LowPassPlugin.h  -  Plugin for simple lowpass filtering
                             -------------------
    begin                : Fri Mar 07 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _LOW_PASS_PLUGIN_H_
#define _LOW_PASS_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "libkwave/KwavePluginSetupDialog.h"
#include "libkwave/KwaveFilterPlugin.h"
#include "libkwave/KwavePlugin.h"

class ArtsMultiTrackFilter;
class QStringList;

class LowPassPlugin: public KwaveFilterPlugin
{
    Q_OBJECT

public:

    /** Constructor */
    LowPassPlugin(PluginContext &context);

    /** Destructor */
    virtual ~LowPassPlugin();

    /** Creates the setup dialog and connects it's signals */
    virtual KwavePluginSetupDialog *createDialog(QWidget *parent);

    /**
     * Creates a multi-track filter with the given number of tracks
     * @param tracks number of tracks that the filter should have
     * @return pointer to the filter or null if failed
     */
    virtual ArtsMultiTrackFilter *createFilter(unsigned int tracks);

    /**
     * Returns true if the parameters have changed during pre-listen.
     */
    virtual bool paramsChanged();

    /**
     * Update the filter with new parameters if it has changed
     * changed during the pre-listen.
     * @param filter the ArtsMultiTrackFilter to be updated, should be the
     *               same one as created with createFilter()
     * @param force if true, even update if no settings have changed
     */
    virtual void updateFilter(ArtsMultiTrackFilter *filter, bool force=0);

    /**
     * Returns a verbose name of the performed action. Used for giving
     * the undo action a readable name.
     */
    virtual QString actionName();

protected:

    /** Reads values from the parameter list */
    virtual int interpreteParameters(QStringList &params);

protected slots:

    /**
     * Called when the parameter changed during pre-listen
     * @param frequency the frequency parameter in Hz
     */
    void setValue(double frequency);

private:

    /** cutoff frequency [Hz] */
    double m_frequency;

    /** last value of m_frequency */
    double m_last_freq;
};

#endif /* _LOW_PASS_PLUGIN_H_ */
