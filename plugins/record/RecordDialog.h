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
    RecordParams params() const;

    /** selects a new record device */
    void setDevice(const QString &dev);

    /** sets the range of supported tracks */
    void setSupportedTracks(unsigned int min, unsigned int max);

    /** sets a new number of tracks */
    void setTracks(unsigned int tracks);

    /** sets the list of supported sample rates */
    void setSupportedSampleRates(const QValueList<double> &rates);

    /**
     * sets a new sample rate
     * @param new_rate the new sample rate [samples/second]
     */
    void setSampleRate(double new_rate);

    /**
     * sets the list of supported compressions
     * @param comps list of supported compressions, can be empty
     */
    void setSupportedCompressions(const QValueList<int> &comps);

    /**
     * sets a new compression type
     * @param compression type of the compression, numeric
     * @see CompressionType
     */
    void setCompression(int compression);

    /** sets a list of supported number of bits per sample */
    void setSupportedBitsPerSample(const QValueList<unsigned int> &bits);

    /** sets a new resolution in bits per sample */
    void setBitsPerSample(unsigned int bits);

    /**
     * Convert a sample rate into a string, using current locale settings.
     * Trailing zeroes and thousands separators are removed, the precision
     * is set to three digits.
     * @param rate sample rate [samples/second]
     * @return the rate formatted as string
     */
    QString rate2string(double rate) const;

    /**
     * sets the list of supported sample formats
     * @param comps list of supported sample formats, must not be empty
     */
    void setSupportedSampleFormats(const QValueList<int> &formats);

    /**
     * sets a new sample format
     * @param sample_format format of the samples, like signed/unsigned
     * @see SampleFormat
     */
    void setSampleFormat(int sample_format);

signals:

    /** emitted when a new record device has been selected */
    void deviceChanged(const QString &device);

    /** emitted when the number of tracks has changed */
    void sigTracksChanged(unsigned int tracks);

    /** emitted when a new sample rate has been selected */
    void sampleRateChanged(double rate);

    /** emitted when the compression has changed */
    void sigCompressionChanged(int compression);

    /** emitted when the resoluton in bits per sample changed */
    void sigBitsPerSampleChanged(unsigned int bits);

    /** emitted when the sample format has changed */
    void sigSampleFormatChanged(int sample_format);

private slots:

    /** updates the record buffer size */
    void sourceBufferChanged(int value);

    /** show a "file open" dialog for selecting a record device */
    void selectRecordDevice();

    /** forwards a deviceChanged signal */
    void forwardDeviceChanged(const QString &dev);

    /** forwards a sigTracksChanged signal */
    void tracksChanged(int tracks);

    /** called when another sample rate has been selected */
    void sampleRateChanged(const QString &rate);

    /** called when a new compression type has been set */
    void compressionChanged(const QString &name);

    /** called when the resolution in bits per sample has changed */
    void bitsPerSampleChanged(int bits);

    /** called when a new sample format has been selected */
    void sampleFormatChanged(const QString &name);

private:

    /**
     * Convert a formated sample rate string back to a numeric sample
     * rate (the opposite of rate2string() )
     * @param rate the sample rate, formatted as string
     * @return the numeric sample rate [samples/second]
     */
    double string2rate(const QString &rate) const;

private:

    /** list of parameters */
    RecordParams m_params;

    /** a list with supported bits per sample */
    QValueList<unsigned int> m_supported_resolutions;
};


#endif /* _RECORD_PLUGIN_H_ */
