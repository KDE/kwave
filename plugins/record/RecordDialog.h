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
#include <qtimer.h>
#include <qvaluelist.h>
#include "RecordController.h"
#include "RecordDlg.uih.h"
#include "RecordParams.h"
#include "RecordState.h"
#include "RecordTypesMap.h"
#include "StatusWidget.h"

#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"

class QWidget;

class RecordDialog: public RecordDlg
{
Q_OBJECT

public:

    /** Constructor */
    RecordDialog(QWidget *parent, QStringList &params,
                 RecordController *controller);

    /** Destructor */
    virtual ~RecordDialog();

    /** returns the list of record parameters, for the next time */
    RecordParams &params();

    /** selects a new recording method */
    void setMethod(record_method_t method);

    /**
     * Sets the list of supported devices, just entries
     * for the device selection combo box.
     */
    void setSupportedDevices(QStringList devices);

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
    void setSupportedBits(const QValueList<unsigned int> &bits);

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
    void setSupportedSampleFormats(const QValueList<SampleFormat> &formats);

    /**
     * sets a new sample format
     * @param sample_format format of the samples, like signed/unsigned
     * @see SampleFormat
     */
    void setSampleFormat(SampleFormat sample_format);

    /**
     * updates the progress bar with the buffer fill state.
     * @param filled number of filled/available buffers
     * @param total maximum amount of buffers
     */
    void updateBufferState(unsigned int count, unsigned int total);

    /**
     * updates all enabled visual effects
     * @param track index of the track that is updated
     * @param buffer array with Kwave sample data
     */
    void updateEffects(unsigned int track, QMemArray<sample_t> &buffer);

    /**
     * Show the "source" device tab, usually if the setup was
     * not successful.
     */
    void showDevicePage();

signals:

    /** emits changes in the currently selected record method */
    void sigMethodChanged(record_method_t method);

    /** emitted when a new record device has been selected */
    void sigDeviceChanged(const QString &device);

    /** emitted when the number of tracks has changed */
    void sigTracksChanged(unsigned int tracks);

    /** emitted when a new sample rate has been selected */
    void sampleRateChanged(double rate);

    /** emitted when the compression has changed */
    void sigCompressionChanged(int compression);

    /** emitted when the resoluton in bits per sample changed */
    void sigBitsPerSampleChanged(unsigned int bits);

    /** emitted when the sample format has changed */
    void sigSampleFormatChanged(SampleFormat sample_format);

    /** emitted when the number and/or size of buffers has changed */
    void sigBuffersChanged();

    /** emitted when the record time has been changed */
    void sigRecordTimeChanged(int limit);

    /** emitted when the record trigger has been enabled/disabled */
    void sigTriggerChanged(bool enabled);

    /** emitted when the prerecording has been enabled/disabled */
    void sigPreRecordingChanged(bool enabled);

public slots:

    /** selects a new record device */
    void setDevice(const QString &device);

    /** set the file filter used for the "Select..." dialog */
    void setFileFilter(const QString &filter);

    /** updates the number of recorded samples */
    void setRecordedSamples(unsigned int samples_recorded);

    /** show a message in the status bar */
    void message(const QString &message);

private slots:

    /**
     * called when a new recording method has been selected
     * from the combo box
     * @param index the position within the combo box
     */
    void methodSelected(int index);

    /** updates the record buffer count */
    void sourceBufferCountChanged(int value);

    /** updates the record buffer size */
    void sourceBufferSizeChanged(int value);

    /** show a "file open" dialog for selecting a record device */
    void selectRecordDevice();

    /** selection in the device list view has changed */
    void listEntrySelected(QListViewItem *item);

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

    /** sets a new state of the dialog, enable/disable controls etc... */
    void setState(RecordState state);

    /** updates the buffer progress bar */
    void updateBufferProgressBar();

    /** prerecording has been enabled/disabled */
    void preRecordingChecked(bool enabled);

    /** the prerecording time has been changed */
    void preRecordingTimeChanged(int time);

    /** record time has been enabled/disabled */
    void recordTimeChecked(bool limited);

    /** record time has been changed */
    void recordTimeChanged(int record_time);

    /** record trigger has been enabled/disabled */
    void triggerChecked(bool enabled);

    /** record trigger has changed */
    void triggerChanged(int trigger);

    /** display: level meter enabled/disabled */
    void displayLevelMeterChecked(bool enabled);

    /** invoke the online help */
    void invokeHelp();

private:

    /**
     * Convert a formated sample rate string back to a numeric sample
     * rate (the opposite of rate2string() )
     * @param rate the sample rate, formatted as string
     * @return the numeric sample rate [samples/second]
     */
    double string2rate(const QString &rate) const;

    /**
     * enabled or disables the record button by evaluating
     * m_record_enabled and m_seconds_recording
     */
    void updateRecordButton();

private:

    /** map of playback methods/types */
    RecordTypesMap m_methods_map;

    /** file filter for the "Select..." dialog (optional) */
    QString m_file_filter;

    /** map for items in the list view */
    QMap<QListViewItem *, QString> m_devices_list_map;

    /** state of the record plugin */
    RecordState m_state;

    /** list of parameters */
    RecordParams m_params;

    /** a list with supported bits per sample */
    QValueList<unsigned int> m_supported_resolutions;

    /** accumulated current buffer progress */
    unsigned int m_buffer_progress_count;

    /** accumulated total buffer progress */
    unsigned int m_buffer_progress_total;

    /** timer for slowly updating the buffer progress bar */
    QTimer m_buffer_progress_timer;

    /** determines if recording is to be enabled by the current state */
    bool m_record_enabled;

    /**
     * holds the recorded samples for comparing with the recording
     * time limit, for determining if recording should be enabled
     * or not. Only of interest if recording time is limit.
     */
    unsigned int m_samples_recorded;

    /** if false, do nothing in setDevice */
    bool m_enable_setDevice;

    /** widget with a icon in the status bar */
    StatusWidget *m_state_icon_widget;

};

#endif /* _RECORD_PLUGIN_H_ */
