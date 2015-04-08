/***************************************************************************
        LowPassDialog.h  -  dialog for the "lowpass" plugin
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

#ifndef _LOW_PASS_DIALOG_H_
#define _LOW_PASS_DIALOG_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtGui/QDialog>
#include <QtCore/QString>

#include "libkwave/PluginSetupDialog.h"

#include "ui_LowPassDlg.h"

class QStringList;

namespace Kwave
{

    class LowPassFilter;

    class LowPassDialog: public QDialog,
                         public Ui::LowPassDlg,
                         public Kwave::PluginSetupDialog
    {
	Q_OBJECT
    public:

	/**
	 * Constructor.
	 * @param parent parent widget
	 * @param sample_rate sample rate of the audio data to be processed,
	 *                    needed for determining the allowed range of
	 *                    the cutoff frequency
	 */
	LowPassDialog(QWidget *parent, double sample_rate);

	/** Destructor */
	virtual ~LowPassDialog();

	/** Returns the parameters as string list */
	virtual QStringList params();

	/** Sets the from a list of parameters */
	virtual void setParams(QStringList &params);

	/** retruns a pointer to this as a QDialog */
	virtual QDialog *dialog() { return this; }

    signals:

	/**
	 * Emitted whenever the frequency changes
	 * @param freq the frequency parameter in Hz
	 */
	void changed(double freq);

	/** Pre-listen mode has been started */
	void startPreListen();

	/** Pre-listen mode has been stopped */
	void stopPreListen();

    protected slots:

	/** called when the spinbox or spinbox value has changed */
	void valueChanged(int pos);

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

	/** Update the graphic display */
	void updateDisplay();

    private:

	/** the cutoff frequency in Hz */
	double m_frequency;

	/** sample rate of the signal in samples/sec */
	double m_sample_rate;

	/** filter function for calculating the frequency response */
	Kwave::LowPassFilter *m_filter;

    };
}

#endif /* _LOW_PASS_DIALOG_H_ */

//***************************************************************************
//***************************************************************************
