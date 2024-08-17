/*************************************************************************
         RecordPlugin.h  -  plugin for recording audio data
                             -------------------
    begin                : Wed Jul 09 2003
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

#ifndef RECORD_PLUGIN_H
#define RECORD_PLUGIN_H

#include "config.h"

#include <QByteArray>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"
#include "libkwave/SampleFIFO.h"
#include "libkwave/SampleFormat.h"

#include "RecordController.h"
#include "RecordParams.h"
#include "RecordState.h"

namespace Kwave
{

    class RecordDevice;
    class RecordDialog;
    class RecordThread;
    class SampleDecoder;

    class RecordPlugin: public Kwave::Plugin
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        RecordPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~RecordPlugin() override;

        /** @see Kwave::Plugin::setup() */
        virtual QStringList *setup(QStringList &previous_params)
            override;

    signals:

        /** emitted to promote the recording progress to the dialog */
        void sigRecordedSamples(sample_index_t samples_recorded);

    protected:

        /**
         * internal guard class for inhibiting low level recording
         * at times where it should not occur.
         */
        class InhibitRecordGuard
        {
        public:
            /** Constructor, inhibits recording */
            explicit InhibitRecordGuard(Kwave::RecordPlugin &recorder)
                :m_recorder(recorder)
            {
                m_recorder.enterInhibit();
            }

            /** Destructor, re-enables recording */
            virtual ~InhibitRecordGuard()
            {
                m_recorder.leaveInhibit();
            }

        private:
            Kwave::RecordPlugin &m_recorder;
        };

    protected:
        friend class InhibitRecordGuard;

        /** inhibits recording, stopping the recorder if necessary */
        void enterInhibit();

        /**
         * leave the area with recording inhibited, restart recorder
         * if needed
         */
        void leaveInhibit();

    protected slots:

        /**
         * command for resetting all recorded stuff for starting again
         * @param accepted bool variable that will show if the action was
         *        performed or aborted (not accepted)
         */
        void resetRecording(bool &accepted);

        /**
         * command for starting the recording, completion is
         * signaled with sigStarted()
         */
        void startRecording();

        /** called when the recording stopped (for detecting aborts only) */
        void recordStopped(int reason);

        /**
         * called when the recording engine has changed it's state
         */
        void stateChanged(Kwave::RecordState state);

    private slots:

        // setup functions

        /**
        * Change the recording method
        * @param method the new recording method
        */
        void setMethod(Kwave::record_method_t method);

        /** select a new record device */
        void setDevice(const QString &device);

        /** select a new number of tracks (channels) */
        void changeTracks(unsigned int new_tracks);

        /** select a new sample rate [samples/second] */
        void changeSampleRate(double new_rate);

        /** change compression type */
        void changeCompression(Kwave::Compression::Type new_compression);

        /** select a new resolution [bits/sample] */
        void changeBitsPerSample(unsigned int new_bits);

        /** select a new sample format */
        void changeSampleFormat(Kwave::SampleFormat::Format new_format);

        /** process a raw audio buffer */
        void processBuffer();

        /** restart recorder with new buffer settings */
        void buffersChanged();

        /** the prerecording checkbox has changed */
        void prerecordingChanged(bool enable);

        /** try to open the record device, in case it was busy before */
        void retryOpen();

    private:

        /** close m_device and delete it */
        void closeDevice();

        /**
         * show a short notice which disappears automatically,
         * e.g. if something is not supported and has been substituted
         * @param message the notice that should pop up
         */
        void notice(QString message);

        /** set up the recorder thread and record device (again) */
        void setupRecordThread();

        /** update the buffer progress bar */
        void updateBufferProgressBar();

        /**
         * check if the trigger level has been reached
         * @param track index of the track that is checked
         * @param buffer array with Kwave sample data
         * @return true if trigger reached or no trigger set
         */
        bool checkTrigger(unsigned int track, const Kwave::SampleArray &buffer);

        /**
         * Split off one track from a raw buffer with multiple tracks into
         * a separate buffer
         * @param raw_data the raw buffer with multiple tracks
         * @param dest byte array that receives the data of the specified track
         * @param bytes_per_sample number of bytes for each sample
         * @param track index of the track to split off [1...n-1]
         * @param tracks number of total tracks
         */
        void split(QByteArray &raw_data, QByteArray &dest,
                   unsigned int bytes_per_sample,
                   unsigned int track,
                   unsigned int tracks);

        /**
         * Enqueue a buffer with decoded samples into a prerecording
         * buffer of the corresponding track.
         *
         * @param track index of the track [0...tracks-1]
         * @param decoded array with decoded samples, in Kwave's internal format
         */
        void enqueuePrerecording(unsigned int track,
                                 const Kwave::SampleArray &decoded);

        /**
         * Flush the content of the prerecording queue to the output
         * @see m_writers
         */
        void flushPrerecordingQueue();

        /**
         * Returns true if all parameters are valid and the recording
         * (thread) could be started.
         */
        bool paramsValid();

    private:

        /** last recording method */
        Kwave::record_method_t m_method;

        /** last record device */
        QString m_device_name;

        /** controller for the recording engine */
        Kwave::RecordController m_controller;

        /** global state of the plugin */
        Kwave::RecordState m_state;

        /** device used for recording */
        Kwave::RecordDevice *m_device;

        /** setup dialog */
        QPointer<Kwave::RecordDialog> m_dialog;

        /** the thread for recording */
        Kwave::RecordThread *m_thread;

        /** decoder for converting raw data to samples */
        Kwave::SampleDecoder *m_decoder;

        /**
         * set of queues for buffering prerecording data, one for each
         * track
         */
        QVector<Kwave::SampleFIFO> m_prerecording_queue;

        /** sink for the audio data */
        Kwave::MultiTrackWriter *m_writers;

        /**
         * number of recorded buffers since start or continue or the number of
         * buffers in the queue if recording stopped
         */
        unsigned int m_buffers_recorded;

        /** recursion level for inhibiting recording */
        unsigned int m_inhibit_count;

        /** buffer for trigger values */
        QVector<float> m_trigger_value;

        /** timer for retrying "open" */
        QTimer m_retry_timer;

    };
}

#endif /* RECORD_PLUGIN_H */

//***************************************************************************
//***************************************************************************
