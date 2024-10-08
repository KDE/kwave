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

#ifndef RECORD_DIALOG_H
#define RECORD_DIALOG_H

#include "config.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "libkwave/Compression.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SampleFormat.h"

#include "RecordController.h"
#include "RecordParams.h"
#include "RecordState.h"
#include "RecordTypesMap.h"
#include "StatusWidget.h"

#include "ui_RecordDlg.h"

class QDateTime;
class QLabel;
class QTreeWidgetItem;
class QWidget;

namespace Kwave
{
    class RecordDialog: public QDialog,
                        public Ui::RecordDlg
    {
    Q_OBJECT

    public:
        typedef enum {
            SETTINGS_DEFAULT = 0, /**< default page (setup, tab 0) */
            SETTINGS_FORMAT,      /**< format settings             */
            SETTINGS_SOURCE,      /**< source settings             */
            START_RECORDING       /**< directly start recording!   */
        } Mode;

        /** Constructor */
        RecordDialog(QWidget *parent, QStringList &params,
                     Kwave::RecordController *controller,
                     Mode mode);

        /** Destructor */
        ~RecordDialog() override;

        /** returns the list of record parameters, for the next time */
        Kwave::RecordParams &params();

        /** selects a new recording method */
        void setMethod(Kwave::record_method_t method);

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
        void setSupportedSampleRates(const QList<double> &rates);

        /**
         * sets a new sample rate
         * @param new_rate the new sample rate [samples/second]
         */
        void setSampleRate(double new_rate);

        /**
         * sets the list of supported compressions
         * @param comps list of supported compressions, can be empty
         */
        void setSupportedCompressions(
            const QList<Kwave::Compression::Type> &comps
        );

        /**
         * sets a new compression type
         * @param compression type of the compression, numeric
         * @see Kwave::Compression
         */
        void setCompression(int compression);

        /** sets a list of supported number of bits per sample */
        void setSupportedBits(const QList<unsigned int> &bits);

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
         * @param formats list of supported sample formats, must not be empty
         */
        void setSupportedSampleFormats(
            const QList<Kwave::SampleFormat::Format> &formats
        );

        /**
         * sets a new sample format
         * @param sample_format format of the samples, like signed/unsigned
         * @see SampleFormat
         */
        void setSampleFormat(Kwave::SampleFormat::Format sample_format);

        /**
         * updates the progress bar with the buffer fill state.
         * @param count number of filled/available buffers
         * @param total maximum amount of buffers
         */
        void updateBufferState(unsigned int count, unsigned int total);

        /**
         * updates all enabled visual effects
         * @param track index of the track that is updated
         * @param buffer array with Kwave sample data
         */
        void updateEffects(unsigned int track, Kwave::SampleArray &buffer);

        /**
         * Show the "source" device tab, usually if the setup was
         * not successful.
         */
        void showDevicePage();

    signals:

        /** emits changes in the currently selected record method */
        void sigMethodChanged(Kwave::record_method_t method);

        /** emitted when a new record device has been selected */
        void sigDeviceChanged(const QString &device);

        /** emitted when the number of tracks has changed */
        void sigTracksChanged(unsigned int tracks);

        /** emitted when a new sample rate has been selected */
        void sampleRateChanged(double rate);

        /** emitted when the compression has changed */
        void sigCompressionChanged(Kwave::Compression::Type compression);

        /** emitted when the resoluton in bits per sample changed */
        void sigBitsPerSampleChanged(unsigned int bits);

        /** emitted when the sample format has changed */
        void sigSampleFormatChanged(Kwave::SampleFormat::Format sample_format);

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
        void setRecordedSamples(sample_index_t samples_recorded);

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
        void listEntrySelected(QTreeWidgetItem *current,
                            QTreeWidgetItem *previous);

        /** selection in the device list view has changed */
        void listItemExpanded(QTreeWidgetItem *item);

        /**
         * updates/fixes the device selection when the tree view has
         * lost focus, to avoid that nothing is selected
         */
        void updateListSelection();

        /** forwards a sigTracksChanged signal */
        void tracksChanged(int tracks);

        /** called when another sample rate has been selected */
        void sampleRateChanged(const QString &rate);

        /** called when a new compression type has been set */
        void compressionChanged(int index);

        /** called when the resolution in bits per sample has changed */
        void bitsPerSampleChanged(int bits);

        /** called when a new sample format has been selected */
        void sampleFormatChanged(int index);

        /** sets a new state of the dialog, enable/disable controls etc... */
        void setState(Kwave::RecordState state);

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

        /** start time has been enabled/disabled */
        void startTimeChecked(bool enabled);

        /** start time has been changed */
        void startTimeChanged(const QDateTime &datetime);

        /** record trigger has been enabled/disabled */
        void triggerChecked(bool enabled);

        /** record trigger has changed */
        void triggerChanged(int trigger);

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
        Kwave::RecordTypesMap m_methods_map;

        /** file filter for the "Select..." dialog (optional) */
        QString m_file_filter;

        /** map for items in the list view */
        QMap<QTreeWidgetItem *, QString> m_devices_list_map;

        /** state of the record plugin */
        Kwave::RecordState m_state;

        /** list of parameters */
        Kwave::RecordParams m_params;

        /** a list with supported bits per sample */
        QList<unsigned int> m_supported_resolutions;

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
         * or not. Also used in status bar for displaying the recorded time.
         */
        sample_index_t m_samples_recorded;

        /** if false, do nothing in setDevice */
        bool m_enable_setDevice;

        /** widget with a icon in the status bar */
        Kwave::StatusWidget *m_state_icon_widget;

        struct {
            QLabel *m_state;           /**< status bar id: state text */
            QLabel *m_time;            /**< status bar id: recorded time */
            QLabel *m_sample_rate;     /**< status bar id: sample rate */
            QLabel *m_bits_per_sample; /**< status bar id: number of tracks */
            QLabel *m_tracks;          /**< status bar id: number of tracks */
        } m_status_bar;
    };
}

#endif /* RECORD_DIALOG_H */

//***************************************************************************
//***************************************************************************
