/***************************************************************************
      NewSignalDialog.h  -  dialog for the "newsignal" plugin
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef NEW_SIGNAL_DIALOG_H
#define NEW_SIGNAL_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QTimer>
#include <QWidget>

#include "ui_NewSigDlg.h"

#include "libkwave/Sample.h"

class QString;

namespace Kwave
{
    class NewSignalDialog: public QDialog,
                           public Ui::NewSigDlg
    {
	Q_OBJECT
    public:
	/**
	 * Constructor.
	 * @param parent the parent widget the dialog belongs to
	 * @param samples default resolution in bits per sample
	 * @param rate default sample rate
	 * @param bits default resolution
	 * @param tracks default tracks
	 * @param by_time if true: select by time, if false: select by samples
	 */
	NewSignalDialog(QWidget *parent, sample_index_t samples,
	                unsigned int rate, unsigned int bits,
	                unsigned int tracks, bool by_time);

	/** Destructor */
	virtual ~NewSignalDialog() {}

	/** Returns the number of samples */
	sample_index_t samples();

	/** Returns the selected sample rate [samples/second] */
	double rate();

	/** Returns the selected resolution [bits per sample] */
	unsigned int bitsPerSample();

	/** Returns the selected number of tracks */
	unsigned int tracks();

	/** Returns true if the selection was made by time */
	bool byTime();

    private slots:

	/**
	 * Checks for modifications of the sample number edit.
	 * That stupid KIntNumInput doesn't notify us about changes :-[
	 */
	void checkNewSampleEdit();

	/** Checks for changes in the samples or time info */
	void checkTimeAndLengthInfo(int);

	/** starts/stops the sample edit's timer if rbTime has been toggled */
	void rbTimeToggled(bool);

	/** updates the number of samples if the time changed */
	void timeChanged(int);

	/** called when the sample rate has been edited or changed */
	void sampleRateChanged(const QString&);

	/** called when the sample rate has been changed */
	void tracksChanged(int);

	/** number of samples changed */
	void samplesChanged(int);

	/** updates the file size */
	void updateFileSize();

	/** called if the slider for the length has been moved */
	void setLengthPercentage(int percent);

	/** invoke the online help */
	void invokeHelp();

    private:

	/**
	 * Returns the maximum number of samples per track that can be created
	 * with the current settings, based on the maximum file size of a .wav
	 * file and the header sizes.
	 */
	sample_index_t maxSamples();

	/**
	 * Sets hours, minutes and seconds according to a given
	 * number of samples.
	 */
	void setHMS(sample_index_t &samples);

    private:

	/** Timer that checks for changes in the sample edit field */
	QTimer m_timer;

	/** flag to avoid recursion */
	bool m_recursive;
    };
}

#endif /* NEW_SIGNAL_DIALOG_H */

//***************************************************************************
//***************************************************************************
