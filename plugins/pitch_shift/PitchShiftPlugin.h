/***************************************************************************
     PitchShiftPlugin.h  -  plugin for modifying the "pitch_shift"
                             -------------------
    begin                : Sun Mar 23 2003
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

#ifndef PITCH_SHIFT_PLUGIN_H
#define PITCH_SHIFT_PLUGIN_H

#include "config.h"
#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/PluginSetupDialog.h"

#include "libgui/FilterPlugin.h"

class QStringList;
namespace Kwave
{

    class KwaveSampleSource;

    class PitchShiftPlugin: public Kwave::FilterPlugin
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	PitchShiftPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~PitchShiftPlugin();

	/** Creates the setup dialog and connects it's signals */
	Kwave::PluginSetupDialog *createDialog(QWidget *parent) Q_DECL_OVERRIDE;

	/**
	 * Creates a multi-track filter with the given number of tracks
	 * @param tracks number of tracks that the filter should have
	 * @return pointer to the filter or null if failed
	 */
	Kwave::SampleSource *createFilter(unsigned int tracks) Q_DECL_OVERRIDE;

	/**
	 * Returns true if the parameters have changed during pre-listen.
	 */
	bool paramsChanged() Q_DECL_OVERRIDE;

	/**
	 * Update the filter with new parameters if it has changed
	 * changed during the pre-listen.
	 * @param filter the Kwave::SampleSource to be updated, should be the
	 *               same one as created with createFilter()
	 * @param force if true, even update if no settings have changed
	 */
	void updateFilter(Kwave::SampleSource *filter,
	                  bool force = false) Q_DECL_OVERRIDE;

	/**
	 * Returns a verbose name of the performed action. Used for giving
	 * the undo action a readable name.
	 */
	QString actionName() Q_DECL_OVERRIDE;

    protected:

	/** Reads values from the parameter list */
	int interpreteParameters(QStringList &params) Q_DECL_OVERRIDE;

    protected slots:

	/**
	 * Called when the parameters of the pre-listen have changed
	 * @param speed the speed factor, floating point
	 * @param frequency the frequency parameter in Hz
	 */
	void setValues(double speed, double frequency);

    private:

	/** speed factor */
	double m_speed;

	/** base frequency */
	double m_frequency;

	/** mode for selecting speed (factor or percentage) */
	bool m_percentage_mode;

	/** last value of m_speed */
	double m_last_speed;

	/** last value of m_frequency */
	double m_last_freq;
    };
}

#endif /* PITCH_SHIFT_PLUGIN_H */

//***************************************************************************
//***************************************************************************
