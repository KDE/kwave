/***************************************************************************
          NoiseDialog.h  -  dialog for the "noise" plugin
                             -------------------
    begin                : Sat Sep 28 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#ifndef NOISE_DIALOG_H
#define NOISE_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QObject>
#include <QString>

#include "libkwave/PluginSetupDialog.h"
#include "ui_NoiseDlg.h"

class QStringList;

namespace Kwave
{

    class OverViewCache;

    class NoiseDialog: public QDialog,
                       public Kwave::PluginSetupDialog,
                       public Ui::NoiseDlg
    {
	Q_OBJECT
    public:

	/** Constructor */
	NoiseDialog(QWidget *parent, Kwave::OverViewCache *overview_cache);

	/** Destructor */
	virtual ~NoiseDialog();

	/** Returns the parameters as string list */
	QStringList params() Q_DECL_OVERRIDE;

	/** Sets the from a list of parameters */
	void setParams(QStringList &params) Q_DECL_OVERRIDE;

	/** retruns a pointer to this as a QDialog */
	QDialog *dialog() Q_DECL_OVERRIDE { return this; }

    signals:

	/**
	 * Emitted whenever the noise level changes
	 * @param level the noise level [0 .. 1.0]
	 */
	void levelChanged(double level);

	/** Pre-listen mode has been started */
	void startPreListen();

	/** Pre-listen mode has been stopped */
	void stopPreListen();

    protected slots:

	/** called when the mode radio buttons changed */
	void modeChanged(bool);

	/** called when the slider's position has changed */
	void sliderChanged(int pos);

	/** called when the spinbox value has changed */
	void spinboxChanged(int pos);

	/**
	 * called when the "Listen" button has been toggled,
	 * to start or stop the pre-listen mode
	 */
	void listenToggled(bool listen);

	/**
	 * called when the pre-listen mode stopped/aborted
	 */
	void listenStopped();

    private slots:

	/** invoke the online help */
	void invokeHelp();

    protected:

	/**
	 * Mode for amplification selection
	 * by percentage or by decibel
	 */
	typedef enum {
	    MODE_PERCENT = 0,
	    MODE_DECIBEL = 1
	} Mode;

	/** Sets a new volume selection mode */
	void setMode(Mode mode);

	/** Update the slider position and the spinbox value */
	void updateDisplay(double value);

    private:

	/** noise level, as a linear factor [0...1] */
	double m_noise;

	/**
	 * current mode for noise level selection
	 */
	Mode m_mode;

	/** if false, ignore the signals of slider and spinbox */
	bool m_enable_updates;

	/** overview cache for calculating the preview image */
	Kwave::OverViewCache *m_overview_cache;

    };
}

#endif /* NOISE_DIALOG_H */

//***************************************************************************
//***************************************************************************
