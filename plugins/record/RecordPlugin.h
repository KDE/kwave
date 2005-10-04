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

#ifndef _RECORD_PLUGIN_H_
#define _RECORD_PLUGIN_H_

#include "config.h"
#include <qcstring.h>
#include <qmemarray.h>
#include <qptrvector.h>
#include "libkwave/KwavePlugin.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "RecordController.h"
#include "RecordParams.h"
#include "RecordState.h"

class QStringList;
class RecordDevice;
class RecordDialog;
class RecordThread;
class SampleDecoder;
class SampleFIFO;

class RecordPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    RecordPlugin(const PluginContext &c);

    /** Destructor */
    virtual ~RecordPlugin();

    /** @see KwavePlugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

signals:

    /** emitted to promote the recording progress to the dialog */
    void sigRecordedSamples(unsigned int samples_recorded);

protected:

    /**
     * internal guard class for inhibiting low level recording
     * at times where it should not occur.
     */
    class InhibitRecordGuard
    {
    public:
	/** Constructor, inhibits recording */
	InhibitRecordGuard(RecordPlugin &recorder)
	    :m_recorder(recorder)
	{
	    m_recorder.enterInhibit();
	};

	/** Destructor, re-enables recording */
	virtual ~InhibitRecordGuard()
	{
	    m_recorder.leaveInhibit();
        };

    private:
	RecordPlugin &m_recorder;
    };

protected:
    friend class InhibitRecordGuard;

    /** inhibits recording, stopping the recorder if necessary */
    void enterInhibit();

    /** leave the area with recording inhibited, restart recorder if needed */
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
     * signalled with sigStarted()
     */
    void startRecording();

    /** called when the recording stopped (for detecting aborts only) */
    void recordStopped(int reason);

    /**
     * called when the recording engine has changed it's state
     */
    void stateChanged(RecordState state);

private slots:

    // setup functions

    /**
     * Change the recording method
     * @param method the new recording method
     */
    void setMethod(record_method_t method);

    /** select a new record device */
    void setDevice(const QString &device);

    /** select a new number of tracks (channels) */
    void changeTracks(unsigned int new_tracks);

    /** select a new sample rate [samples/second] */
    void changeSampleRate(double new_rate);

    /** change compression type */
    void changeCompression(int new_compression);

    /** select a new resolution [bits/sample] */
    void changeBitsPerSample(unsigned int new_bits);

    /** select a new sample format */
    void changeSampleFormat(SampleFormat new_format);

    /** process a raw audio buffer */
    void processBuffer(QByteArray buffer);

    /** restart recorder with new buffer settings */
    void buffersChanged();

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
    bool checkTrigger(unsigned int track, QMemArray<sample_t> &buffer);

    /**
     * Split off one track from a raw buffer with multiple tracks into
     * a separate buffer
     * @param raw_data the raw buffer with multiple tracks
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
                             const QMemArray<sample_t> &decoded);

    /**
     * Flush the content of the prerecording queue to the output
     * @see m_writers
     */
    void flushPrerecordingQueue();

private:

    /** last recording method */
    record_method_t m_method;

    /** last record device */
    QString m_device_name;

    /** controller for the recording engine */
    RecordController m_controller;

    /** global state of the plugin */
    RecordState m_state;

    /** device used for recording */
    RecordDevice *m_device;

    /** setup dialog */
    RecordDialog *m_dialog;

    /** the thread for recording */
    RecordThread *m_thread;

    /** decoder for converting raw data to samples */
    SampleDecoder *m_decoder;

    /**
     * set of queues for buffering prerecording data, one for each
     * track
     */
    QPtrVector<SampleFIFO> m_prerecording_queue;

    /** sink for the audio data */
    MultiTrackWriter m_writers;

    /**
     * number of recorded buffers since start or continue or the number of
     * buffers in the queue if recording stopped
     */
    unsigned int m_buffers_recorded;

    /** recursion level for inhibiting recording */
    unsigned int m_inhibit_count;

    /** buffer for trigger values */
    QMemArray<double> m_trigger_value;

};

#endif /* _RECORD_PLUGIN_H_ */
