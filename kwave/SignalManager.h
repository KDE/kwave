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
#define _SIGNAL_MANAGER_H_

#include <stdio.h>

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>
#include <qstring.h>

#include "mt/Mutex.h"
#include "mt/SignalProxy.h"

#include "libkwave/Selection.h"
#include "libkwave/Signal.h"

#include "PlaybackController.h"

class QBitmap;
class QFile;
class Track;
class UndoAction;
class UndoTransaction;
class UndoTransactionGuard;

/**
 * The SignalManager class manages multi-channel signals.
 */
class SignalManager : public QObject
{
    Q_OBJECT

public:
    /** Default constructor. */
    SignalManager(QWidget *parent);

    /** Default destructor */
    virtual ~SignalManager();

    void loadFile(const QString &filename, int type = 0);

    /**
     * Closes the current signal.
     */
    void close();

    /** Returns true if the signal is closed */
    inline bool isClosed() { return m_closed; };

    /** Returns true if the signal is empty. */
    inline bool isEmpty() { return m_empty; };

    /** Returns a reference to the playback controller. */
    PlaybackController &playbackController();

    /** */
    bool executeCommand(const QString &command);

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
    inline unsigned int bits() { return m_signal.bits(); };

    /** Returns the current sample rate in samples per second */
    inline int rate() { return m_rate; };

    /** Returns the current number of tracks */
    inline unsigned int tracks() { return m_signal.tracks(); };

    /**
     * Returns the number of samples in the current signal. Will be
     * zero if no signal is loaded.
     */
    inline unsigned int length() { return m_signal.length(); };

    /** Returns a reference to the current selection */
    inline Selection &selection() { return m_selection; };

    /**
     * Returns an array of indices of currently selected channels.
     */
    const QArray<unsigned int> selectedTracks();

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

    /**
     * Saves the signal to a file with a given resolution. If the file
     * already exists, it will be overwritten.
     * @param filename name of the file to be saved.
     * @param bits number of bits per sample
     * @param selection if true, only the selected range will be saved
     */
    void save(const QString &filename, unsigned int bits, bool selection);

    /**
     * Exports ascii file with one sample per line and only one channel.
     */
    void exportAscii(const char *name);

    /**
     * Sets the current start and length of the selection to new values.
     * @param offset index of the first sample
     * @param length number of samples
     */
    void selectRange(unsigned int offset, unsigned int length);

    /**
     * Toggles the selection flag of a channel.
     * @param channel index of the channel [0..N-1]
     */
    void toggleChannel(const unsigned int channel);

    /**
     * Opens an input stream for a track, starting at a specified sample
     * position.
     * @param track index of the track. If the track does not exist, this
     *        function will fail and return 0
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see InsertMode
     */
    SampleWriter *openSampleWriter(unsigned int track, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Opens a stream for reading samples. If the the last position
     * is omitted, the value UINT_MAX will be used.
     * @param track index of the track. If the track does not exist, this
     *        function will fail and return 0
     * @param left first offset to be read (default = 0)
     * @param right last position to read (default = UINT_MAX)
     */
    inline SampleReader *openSampleReader(unsigned int track,
	unsigned int left = 0, unsigned int right = UINT_MAX)
    {
	return m_signal.openSampleReader(track, left, right);
    };

    /** Returns true if undo/redo is currently enabled */
    inline bool undoEnabled() { return m_undo_enabled; };

    /**
     * Enables undo and redo. If undo/redo is already enabled, nothing
     * will be done.
     */
    void enableUndo();

    /**
     * Disables undo and redo. If undo/redo was enabled, all undo data
     * will be discarded in order to avoid trouble when modifications
     * are done while undo is of.
     * @note No modifications should be performed while undo is off!
     */
    void disableUndo();

signals:

    /**
     * Signals that a track has been inserted.
     * @param index position of the new track [0...tracks()-1]
     * @param track reference to the new track
     */
    void sigTrackInserted(unsigned int index, Track &track);

    /**
     * Emits status information of the signal if it has been changed
     * or become valid.
     * @param length number of samples
     * @param tracks number of tracks
     * @param rate sample rate [samples/second]
     * @param bits resolution in bits
     */
    void sigStatusInfo(unsigned int length, unsigned int tracks,
                       unsigned int rate, unsigned int bits);

    /**
     * Emitted if samples have been inserted into a track. This implies
     * a modification of the inserted data, so no extra sigSamplesModified
     * is emitted.
     * @param track index of the track
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see sigSamplesModified
     */
    void sigSamplesInserted(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted if samples have been removed from a track.
     * @param track index of the track
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     */
    void sigSamplesDeleted(unsigned int track, unsigned int offset,
                           unsigned int length);

    /**
     * Emitted if samples within a track have been modified.
     * @param track index of the track
     * @param offset position from which the data was modified
     * @param length number of samples modified
     */
    void sigSamplesModified(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Emitted if the length of the signal has changed.
     * @param length new length of the signal
     */
    void sigLengthChanged(unsigned int length);

    /**
     * Emitted if a command has to be executed by
     * the next higher instance.
     * @param command the command to be executed
     */
    void sigCommand(const QString &command);

    /**
     * Indicates a change in the position of the playback pointer
     * during playback.
     */
    void sigPlaybackPos(unsigned int pos);

    /**
     * Indicates that playback is done.
     */
    void sigPlaybackDone();

    /**
     * Signals that a track has been inserted.
     * @param track index of the new track [0...tracks()-1]
     */
    void sigTrackInserted(unsigned int track);

    /**
     * Emitted if the state or description of undo/redo has changed. If
     * undo or redo is unavailable the description will be zero.
     * @see emitUndoRedoInfo
     */
    void sigUndoRedoInfo(const QString &undo, const QString &redo);

public slots:

    /**
     * Un-does the last action if possible.
     */
    void undo();

    /**
     * re-does the last undone action.
     */
    void redo();

private slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::trackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &track);

    /**
     * Connected to the signal's sigSamplesInserted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Signal::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(unsigned int track, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, unsigned int offset,
                             unsigned int length);

    /**
     * Closes an undo transaction or recurses the current recursion level
     * of nested undo transactions.
     */
    void closeUndoTransaction();

    /**
     * Determines the description of undo and redo actions and emits
     * a sigUndoRedoInfo. If undo or redo is currently not available,
     * the descriptions will be zero-length. If an action is available
     * but does not have a description, the description will be set
     * to "last action".
     * @see sigUndoRedoInfo
     */
    void emitUndoRedoInfo();

protected:

    friend class UndoTransactionGuard;
    friend class PluginManager;

    /**
     * Tries to free memory for a new undo action and stores all needed
     * data if successful.
     * @param action the UndoAction to that is to be registered
     * @return true if the action is allowed, false if the user has
     *         choosen to abort the operation if the memory limit of
     *         the undo buffer would be exceeded. The return value
     *         will also be false if the action is null.
     * @note If undo is currently not enabled, the passed UndoAction
     *       will be immediately deleted and the return value would
     *       be true. So it is safer not to call this function if
     *       undo is not enabled.
     */
    bool registerUndoAction(UndoAction *action);

    /**
     * Starts an undo transaction or enters a currently running transaction
     * recursively.
     * @param name the name of the transaction. Will be ignored if there
     *        already is an active transaction (optional)
     */
    void startUndoTransaction(const QString &name = 0);

    /**
     * Removes all undo and redo transactions.
     * @note must not be called if an undo transaction is currently active!
     */
    void flushUndoBuffers();

    /**
     * Removes all redo transactions.
     */
    void flushRedoBuffer();

private:

    /** Built-in command Erases a range of samples */
    void builtinCmdErase(unsigned int offset, unsigned int length);

    /** Shortcut for emitting a sigStatusInfo */
    void emitStatusInfo();

    /**
     * Returns the amount of memory currently used for undo + redo.
     */
    unsigned int usedUndoRedoMemory();

    /**
     * Makes sure that enough memory for a following undo (or redo) action
     * is available. If necessary, it deletes old undo transactions and if
     * still no enough, it also removes old redo transactions.
     * @param needed the amount of memory that should be free afterwards
     */
    void freeUndoMemory(unsigned int needed);

    /**
     * Try to find a chunk within a RIFF file. If the chunk
     * was found, the current position will be at the start
     * of the chunk's data.
     * @param sigfile file to read from
     * @param chunk name of the chunk
     * @param offset the file offset for start of the search
     * @return the size of the chunk, 0 if not found
     */
    __uint32_t findChunk(QFile &sigfile, const char *chunk,
	__uint32_t offset = 12);

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
     * Reads in the wav data chunk from a .wav-file. It creates
     * a new empty Signal for each channel and fills it with
     * data read from an opened file. The file's actual position
     * must already be set to the correct position.
     * @param sigin reference to the already opened file
     * @param length number of samples to be read
     * @param channels number of channels [1..n]
     * @param number of bits per sample [8,16,24,...]
     * @return 0 if succeeded or error number if failed
     */
    int loadWavChunk(QFile &sigin, unsigned int length,
                     unsigned int channels, int bits);

    /**
     * Writes the chunk with the signal to a .wav file (not including
     * the header!).
     * @param sigout reference to the already opened file
     * @param offset start position from where to save
     * @param length number of samples to be written
     * @param bits number of bits per sample [8,16,24,...]
     * @return 0 if succeeded or error number if failed
     */
    int writeWavChunk(QFile &sigout, unsigned int offset, unsigned int length,
                      unsigned int bits);

private:

    /** Parent widget, used for showing messages */
    QWidget *m_parent_widget;

    /** name of the signal, normally equal to the filename */
    QString m_name;

    /** true if the signal is closed */
    bool m_closed;

    /** true if the signal is completely empty */
    bool m_empty;

    /** signal with multiple tracks */
    Signal m_signal;

    /** the current selection */
    Selection m_selection;

    /** sampling rate of the current signal */
    int m_rate;

    /** the controller for handling of playback */
    PlaybackController m_playback_controller;

    /** flag for "undo enabled" */
    bool m_undo_enabled;

    /** fifo used for storing all undo transactions */
    QList<UndoTransaction> m_undo_buffer;

    /** fifo for storing all redo transactions */
    QList<UndoTransaction> m_redo_buffer;

    /** the current undo transaction */
    UndoTransaction *m_undo_transaction;

    /** level of nested undo transactions */
    unsigned int m_undo_transaction_level;

    /** mutex for locking undo transactions */
    Mutex m_undo_transaction_lock;

    /** SignalProxy for emitting sigUndoRedoInfo thread-safe */
    SignalProxy<void> m_spx_undo_redo;

    /** maximum memory for undo */
    unsigned int m_undo_limit;

};

#endif  /* _SIGNAL_MANAGER_H_ */
