/***************************************************************************
		SignalManager.h  -  manager for multi-channel signals
			     -------------------
    begin                : Thu May  04 2000
    copyright            : (C) 2000 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
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

#define MAXCHANNELS 64

#define processid       0
#define stopprocess     1
#define samplepointer   2

#include <qobject.h>
#include <qlist.h>
#include <stdio.h>

class ProgressDialog;
class Signal;
class TimeOperation;

typedef struct {
    int rate;
    int channels;
    int bits_per_sample;
    const char *device;
    int bufbase;
} playback_param_t;

/**
 * The SignalManager class manages multi-channel signals.
 */
class SignalManager : public QObject
{
    Q_OBJECT

public:
    SignalManager(Signal *sig);
    SignalManager(const char *filename, int type = 0);
    SignalManager(unsigned int length, int rate,
                  unsigned int channel = 1);
    virtual ~SignalManager();

    bool executeCommand(const char *command);

    int setSoundParams(int audio, int bitspersample,
                       unsigned int channels, int rate, int bufbase);

    /**
     * Internally used for playback.
     * @param device file descriptor of the opened playback device
     * @param param parameters used for playback
     * @param buffer pointer to a sample buffer used for playback
     * @param bufsize size of the buffer in bytes
     * @param loop true: looping instead of single play
     */
    void playback(int device, playback_param_t &param,
                  unsigned char *buffer, unsigned int bufsize, bool loop);

    void getMaxMin(unsigned int channel, int &max, int& min,
                   unsigned int begin, unsigned int len);

    int getBitsPerSample();

    inline int getRate()
    {
	return rate;
    };

    inline unsigned int getChannelCount()
    {
	return channels;
    };

    unsigned int getLength();

    inline unsigned int getLMarker()
    {
	return lmarker;
    };

    inline unsigned int getRMarker()
    {
	return rmarker;
    };

    inline unsigned int getPlayPosition()
    {
	return msg[samplepointer];
    };
    inline bool isPlaying()
    {
	return msg[processid];
    };
    inline Signal *getSignal (int channel)
    {
	return signal.at(channel);
    };

    int getSingleSample(unsigned int channel, unsigned int offset);

    void playback_setOp(int);

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

    bool promoteCommand (const char *command);

    /**
     * Toggles the selection flag of a channel.
     * @param channel index of the channel [0..N-1]
     */
    void toggleChannel(const unsigned int channel);

signals:

    /**
     * Emitted if a command has to be executed by
     * the next higher instance.
     * @param command the command to be executed
     */
    void sigCommand(const char *command);

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

private slots:

    /**
     * Informs us that the last command of a plugin has completed.
     */
    void commandDone();

private:

    void initialize();

    ProgressDialog *createProgressDialog(TimeOperation *operation,
                                         const char *caption);

    void play(bool loop);

    void stopplay();

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

    char *name;

    /** list of all channels (signals) */
    QList<Signal> signal;

    unsigned int lmarker;
    unsigned int rmarker;
    unsigned int channels;
    int rate;                    //sampling rate being used

public:
    /** buffer for communication with the soundcard access functions (play) */
    unsigned int msg[4];
};

#endif  // _SIGNAL_MANAGER_H_
