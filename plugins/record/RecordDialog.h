/*************************************************************************
         RecordDialog.h  -  dialog window for controlling audio recording
                             -------------------
    begin                : Wed Aug 20 2003
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

#ifndef _RECORD_DIALOG_H_
#define _RECORD_DIALOG_H_

#include "config.h"
#include <qstringlist.h>
#include <qvaluelist.h>
#include "RecordDlg.uih.h"
#include "RecordParams.h"

class QWidget;

class RecordDialog: public RecordDlg
{
Q_OBJECT

public:

    enum RecordState {
	REC_EMPTY = 0,
	REC_BUFFERING,
	REC_WAITING_FOR_TRIGGER,
	REC_RECORDING,
	REC_PAUSED,
	REC_DONE
    };

    /** Constructor */
    RecordDialog(QWidget *parent, const RecordParams &params);

    /** Destructor */
    virtual ~RecordDialog();

    /** Returns the list of record parameters, for the next time */
    virtual RecordParams params() const;

    /** selects a new record device */
    void setDevice(const QString &dev);

    /** sets the list of supported sample rates */
    void setSupportedSampleRates(const QValueList<double> &rates);

    /**
     * sets a new sample rate
     * @param new_rate the new sample rate [samples/second]
     */
    void setSampleRate(double new_rate);

signals:

    /** emitted when a new record device has been selected */
    void deviceChanged(const QString &device);

    /** emitted when a new sample rate has been selected */
    void sampleRateChanged(double rate);

private slots:

    /** updates the record buffer size */
    void sourceBufferChanged(int value);

    /** show a "file open" dialog for selecting a record device */
    void selectRecordDevice();

    /** forwards a deviceChanged signal */
    void forwardDeviceChanged(const QString &dev);

    /** called when another sample rate has been selected */
    void sampleRateChanged(const QString &);

private:

    /**
     * Convert a sample rate into a string, using current locale settings.
     * Trailing zeroes and thousands separators are removed, the precision
     * is set to three digits.
     * @param rate sample rate [samples/second]
     * @return the rate formatted as string
     */
    QString rate2string(double rate);

    /**
     * Convert a formated sample rate string back to a numeric sample
     * rate (the opposite of rate2string() )
     * @param rate the sample rate, formatted as string
     * @return the numeric sample rate [samples/second]
     */
    double string2rate(const QString &rate);

private:

    /** List of parameters */
    RecordParams m_params;

};


#endif /* _RECORD_PLUGIN_H_ */
