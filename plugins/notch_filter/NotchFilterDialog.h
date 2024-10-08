/***************************************************************************
    NotchFilterDialog.h  -  dialog for the "notch_filter" plugin
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

#ifndef NOTCH_FILTER_DIALOG_H
#define NOTCH_FILTER_DIALOG_H

#include "config.h"
#include <QObject>
#include <QString>

#include "libkwave/PluginSetupDialog.h"

#include "ui_NotchFilterDlg.h"

namespace Kwave
{

    class NotchFilter;

    class NotchFilterDialog: public QDialog,
                             public Kwave::PluginSetupDialog,
                             public Ui::NotchFilterDlg
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
        NotchFilterDialog(QWidget *parent, double sample_rate);

        /** Destructor */
        ~NotchFilterDialog() override;

        /** Returns the parameters as string list */
        QStringList params() override;

        /** Sets the from a list of parameters */
        void setParams(QStringList &params) override;

        /** retruns a pointer to this as a QDialog */
        QDialog *dialog() override { return this; }

    signals:

        /**
         * Emitted whenever the frequency changes
         * @param freq the frequency parameter in Hz
         */
        void freqChanged(double freq);

        /**
         * Emitted when the bandwidth selectio has changed
         * @param bw the bandwidth parameter in Hz
         */
        void bwChanged(double bw);

        /** Pre-listen mode has been started */
        void startPreListen();

        /** Pre-listen mode has been stopped */
        void stopPreListen();

    protected slots:

        /** called when the freq spinbox or slider value has changed */
        void freqValueChanged(int pos);

        /** called when the bw spinbox or slider value has changed */
        void bwValueChanged(int pos);

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

        /** the bw in Hz */
        double m_bw;

        /** sample rate of the signal in samples/sec */
        double m_sample_rate;

        /** filter function for calculating the frequency response */
        Kwave::NotchFilter *m_filter;

    };
}

#endif /* NOTCH_FILTER_DIALOG_H */

//***************************************************************************
//***************************************************************************
