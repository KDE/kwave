/***************************************************************************
        NotchFilterPlugin.h  -  Plugin for simple notch filtering
                             -------------------
    begin                : Thu Jun 19 2003
    copyright            : (C) 2003 by Dave Flogeras
    email                : d.flogeras@unb.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NOTCH_FILTER_PLUGIN_H_
#define _NOTCH_FILTER_PLUGIN_H_

#include "config.h"
#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/KwavePluginSetupDialog.h"
#include "libkwave/KwavePlugin.h"

#include "libgui/KwaveFilterPlugin.h"

class QStringList;
namespace Kwave { class KwaveSampleSource; }

class NotchFilterPlugin: public Kwave::FilterPlugin
{
    Q_OBJECT

public:

    /** Constructor */
    NotchFilterPlugin(const PluginContext &context);

    /** Destructor */
    virtual ~NotchFilterPlugin();

    /** Creates the setup dialog and connects it's signals */
    virtual KwavePluginSetupDialog *createDialog(QWidget *parent);

    /**
     * Creates a multi-track filter with the given number of tracks
     * @param tracks number of tracks that the filter should have
     * @return pointer to the filter or null if failed
     */
    virtual Kwave::SampleSource *createFilter(unsigned int tracks);

    /**
     * Returns true if the parameters have changed during pre-listen.
     */
    virtual bool paramsChanged();

    /**
     * Update the filter with new parameters if it has changed
     * changed during the pre-listen.
     * @param filter the Kwave::SampleSource to be updated, should be the
     *               same one as created with createFilter()
     * @param force if true, even update if no settings have changed
     */
    virtual void updateFilter(Kwave::SampleSource *filter, bool force=0);

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
    void setFreqValue(double frequency);

    /**
     * set a new bandwidth
     * @bw bandwidth [Hz]
     */
    void setBwValue(double bw);

private:

    /** cutoff frequency [Hz] */
    double m_frequency;

    /** last value of m_frequency */
    double m_last_freq;

    /** bandwidth value [Hz] */
    double m_bw;

    /** last value of m_bw */
    double m_last_bw;

};

#endif /* _NOTCH_FILTER_PLUGIN_H_ */
