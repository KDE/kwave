/***************************************************************************
          SignalManager.h -  manager class for multi-channel signals
			     -------------------
    begin                : Sun Oct 15 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _SIGNAL_MANAGER_H_
#define _SIGNAL_MANAGER_H_ 1

#define processid       0
#define stopprocess     1
#define samplepointer   2

#include <stdio.h>

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>
#include <qstring.h>

#include "mt/SignalProxy.h"

#include "libkwave/Signal.h"

class ProgressDialog;
class QBitmap;
class TimeOperation;

typedef struct {
    int rate;
    int channels;
    int bits_per_sample;
    QString device;
    int bufbase;
} playback_param_t;

/**
 * The SignalManager class manages multi-channel signals.
 */
class SignalManager : public QObject
{
    Q_OBJECT

public:
    /** Default constructor. */
    SignalManager();

    virtual ~SignalManager();

    void loadFile(const QString &filename, int type = 0);

    bool executeCommand(const QString &command);

    int setSoundParams(int audio, int bitspersample,
                       unsigned int channels, int rate, int bufbase);

    /**
     * Internally used for playback.
     * @param device file descriptor of the opened playback device
     * @param param parameters used for playback
     * @param buffer pointer to a sample buffer used for playback
     * @param bufsize size of the buffer in bytes
     * @param start position where playback should start
     * @param loop true: looping instead of single play
     */
    void playback(int device, playback_param_t &param,
                  unsigned char *buffer, unsigned int bufsize,
                  unsigned int start, bool loop);

    /**
     * Determines the maximum and minimum values of a given range
     * of samples.
     * @param channel index of the channel
     * @param max receives the highest sample value (most positive)
     * @param min receives the lowest sample value (most negative)
     * @param begin start of the sample range (inclusive)
     * @param len number of samples to inspect
     */
    void getMaxMin(unsigned int channel, int &max, int& min,
                   unsigned int begin, unsigned int len);

    /**
     * Returns a QBitmap with an overview of all currently present
     * signals.
     * @param width width of the resulting bitmap in pixels
     * @param height height of the resutling bitmap in pixels
     * @param offset index of the first sample
     * @param length number of samples
     */
    QBitmap *overview(unsigned int width, unsigned int height,
                      unsigned int offset, unsigned int length);

    /**
     * Returns the current sample resolution in bits per sample
     */
    int getBitsPerSample();

    /** Returns the current sample rate in samples per second */
    inline int getRate()
    {
	return rate;
    };

    /** Returns the current number of channels */
    inline unsigned int channels()
    {
	return m_channels;
    };

    /**
     * Returns the number of samples in the current signal. Will be
     * zero if no signal is loaded.
     */
    unsigned int getLength();

    /**
     * Returns the start of the selection (inclusive).
     */
    inline unsigned int getLMarker()
    {
	return lmarker;
    };

    /**
     * Returns the end of the selection (inclusive).
     */
    inline unsigned int getRMarker()
    {
	return rmarker;
    };

    inline Signal *getSignal (int channel)
    {
	return 0; // ###
    };

    /**
     * Returns an array of indices of currently selected channels.
     */
    const QArray<unsigned int> selectedChannels();

    /**
     * Returns the value of one single sample of a specified channel.
     * If the channel does not exist or the index of the sample is
     * out of range the return value will be zero.
     * @param channel index if the channel [0...N-1]
     * @param offset sample offset [0...length-1]
     * @return value of the sample
     */
    int singleSample(unsigned int channel, unsigned int offset);

    /**
     * Returns the value of one single sample averaged over all active channels.
     * If no channel do exist or the index of the sample is out of range the
     * return value will be zero. If the optional list of channels is omitted,
     * the sample will be averaged over all currently selected channels.
     * @param offset sample offset [0...length-1]
     * @param channels an array of channel numbers, optional
     * @return value of the sample
     */
    int averageSample(unsigned int offset,
                      const QArray<unsigned int> *channels = 0);

    void save(const char *filename, int bits, bool selection);

    /**
     * Exports ascii file with one sample per line and only one channel.
     */
    void exportAscii(const char *name);

    void appendChannel(Signal *);

    /**
     * Sets the internal markers and promotes them to all channels.
     * @param l left marker [0...length-1]
     * @param r right marker [0...length-1]
     */
    void setRange(unsigned int l, unsigned int r);

    bool promoteCommand (const QString &command);

    /**
     * Toggles the selection flag of a channel.
     * @param channel index of the channel [0..N-1]
     */
    void toggleChannel(const unsigned int channel);

public slots:

    /**
     * Starts playback.
     * @param start position where playback should start
     * @param loop true: looping instead of single play
     */
    void startplay(unsigned int start, bool loop);

    /**
     * Stops playback.
     */
    void stopplay();

signals:

    /**
     * Emitted if a command has to be executed by
     * the next higher instance.
     * @param command the command to be executed
     */
    void sigCommand(const QString &command);

    /**
     * Indicates that the signal data within a range
     * has changed.
     * @param lmarker leftmost sample or -1 for "up to the left"
     * @param rmarker rightmost sample or -1 for "up to the right"
     */
    void signalChanged(int lmarker, int rmarker);

    /**
     * Signals that a channel has been added/inserted. The channels
     * at and after this position (if any) have moved to channel+1.
     * @param channel index of the new channel [0...N-1]
     */
    void sigChannelAdded(unsigned int channel);

    /**
     * Signals that a channel has been deleted. All following channels
     * are shifted one channel down.
     * @param channel index of the deleted channel [0...N-1]
     */
    void sigChannelDeleted(unsigned int channel);

    /**
     * Indicates a change in the position of the playback pointer
     * during playback.
     */
    void sigPlaybackPos(unsigned int pos);

    /**
     * Indicates that playback is done.
     */
    void sigPlaybackDone();

private slots:

    /**
     * Informs us that the last command of a plugin has completed.
     */
    void commandDone();

    /**
     * Called from the playback thread to notify about a new
     * playback pointer.
     * \internal
     */
    void updatePlaybackPos();

    /**
     * emits sigPlaybackDone() at end of playback
     */
    void forwardPlaybackDone();

private:

    void initialize();

    ProgressDialog *createProgressDialog(TimeOperation *operation,
                                         const char *caption);

    /**
     * Searches for the wav data chunk in a wav file.
     * @param sigin pointer to the already opened file
     * @return the position of the data or 0 if failed
     */
    int findWavChunk(FILE *sigin);

    /**
     * Imports ascii file with one sample per line and only one
     * channel. Everything that cannot be parsed by strod will be ignored.
     * @return 0 if succeeded or error number if failed
     */
    int loadAscii();

    /**
     * Loads a .wav-File.
     * @return 0 if succeeded or a negative error number if failed:
     *           -ENOENT if file does not exist,
     *           -ENODATA if the file has no data chunk or is zero-length,
     *           -EMEDIUMTYPE if the file has an invalid/unsupported format
     */
    int loadWav();

    /**
     * Appends a new empty channel to the current signal.
     * It has the same length as the first channel.
     */
    void addChannel();

    /**
     * Removes a channel from the signal.
     */
    void deleteChannel(unsigned int channel);

private:

    /**
     * Reads in the wav data chunk from a .wav-file. It creates
     * a new empty Signal for each channel and fills it with
     * data read from an opened file. The file's actual position
     * must already be set to the correct position.
     * @param sigin pointer to the already opened file
     * @param length number of samples to be read
     * @param channels number of channels [1..n]
     * @param number of bits per sample [8,16,24,...]
     * @return 0 if succeeded or error number if failed
     */
    int loadWavChunk(FILE *sigin, unsigned int length,
                     unsigned int channels, int bits);

    /**
     * Writes the chunk with the signal to a .wav file (not including
     * the header!).
     * @param sigout pointer to the already opened file
     * @param length number of samples to be written
     * @param channels number of channels [1..n]
     * @param number of bits per sample [8,16,24,...]
     * @return 0 if succeeded or error number if failed
     */
    int writeWavChunk(FILE *sigout, unsigned int begin, unsigned int length,
                      int bits);

    /** name of the signal, normally equal to the filename */
    QString m_name;

    /** signal with multiple tracks */
    Signal m_signal;

    unsigned int lmarker;
    unsigned int rmarker;
    unsigned int m_channels;
    int rate;                    //sampling rate being used

    /**
     * Signal proxy that brings the current playback position
     * out of the playback thread.
     */
    SignalProxy1<unsigned int> m_spx_playback_pos;

    /** error string from the playback thread */
    const char *m_playback_error;

public:

    /**
     * Signal proxy that signals the end of playback out
     * of the playback thread.
     */
    SignalProxy<void> m_spx_playback_done;

    /** buffer for communication with the soundcard access functions (play) */
    unsigned int msg[4];
};

#endif  // _SIGNAL_MANAGER_H_
