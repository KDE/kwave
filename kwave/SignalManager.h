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

#include "config.h"
#include <limits.h>
#include <stdio.h>

#include <qobject.h>
#include <qmemarray.h>
#include <qmutex.h>
#include <qptrlist.h>
#include <qstring.h>

#include "mt/SignalProxy.h"
#include "libkwave/FileInfo.h"
#include "libkwave/Selection.h"
#include "libkwave/Signal.h"

#include "PlaybackController.h"

class QBitmap;
class QFile;
class ClipBoard;
class KURL;
class MultiTrackReader;
class MultiTrackWriter;
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

    /**
     * Closes the current signal and loads a new file.
     * @param url URL of the file to be loaded
     * @return 0 if succeeded or error code < 0
     */
    int loadFile(const KURL &url);

    /**
     * Closes the current signal and creates a new empty signal.
     * @param samples number of samples per track
     * @param rate sample rate
     * @param bits number of bits per sample
     * @param tracks number of tracks
     */
    void newSignal(unsigned int samples, double rate,
                   unsigned int bits, unsigned int tracks);

    /**
     * Closes the current signal.
     */
    void close();

    /** Returns true if the signal is closed */
    inline bool isClosed() { return m_closed; };

    /** Returns true if the signal is empty. */
    inline bool isEmpty() { return m_empty; };

    /** Returns true if the signal is modified */
    inline bool isModified() { return m_modified; };

    /** Returns a reference to the playback controller. */
    PlaybackController &playbackController();

    /** Executes a Kwave text command */
    bool executeCommand(const QString &command);

    /**
     * Returns a reference to the FileInfo object associated with the
     * currently opened file.
     */
    FileInfo &fileInfo() { return m_file_info; };

    /**
     * Returns the current sample resolution in bits per sample
     */
    inline unsigned int bits() const { return m_file_info.bits(); };

    /** Returns the current sample rate in samples per second */
    inline double rate() const { return m_file_info.rate(); };

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
     * Returns true if a given track is selected. If the track does
     * not exist or is not selected the return value is false.
     */
    inline bool trackSelected(unsigned int track) {
	return (m_signal.trackSelected(track));
    };


    /**
     * Returns an array of indices of currently selected tracks.
     */
    const QMemArray<unsigned int> selectedTracks();

    /**
     * Returns an array of indices of all present tracks.
     */
    const QMemArray<unsigned int> allTracks();

    /**
     * Saves the signal to a file with a given resolution. If the file
     * already exists, it will be overwritten.
     * @param url URL with the name of the file to be saved.
     * @param selection if true, only the selected range will be saved
     * @return zero if succeeded or negative error code
     */
    int save(const KURL &url, bool selection);

    /**
     * Exports ascii file with one sample per line and only one track.
     * @param name the name of the file to be exported
     * @return zero if succeeded or negative error code
     */
    int exportAscii(const char *name);

    /**
     * Deletes a range of samples and creates an undo action.
     * @param offset index of the first sample
     * @param length number of samples
     * @param track_list a list of tracks to be affected
     * @return true if successful or nothing to do, false if not enough
     *         memory for undo
     */
    bool deleteRange(unsigned int offset, unsigned int length,
                     const QMemArray<unsigned int> &track_list);

    /**
     * Deletes a range of samples and creates an undo action. Same as
     * above, but affects all tracks.
     * @param offset index of the first sample
     * @param length number of samples
     * @return true if successful or nothing to do, false if not enough
     *         memory for undo
     */
    bool deleteRange(unsigned int offset, unsigned int length);

    /**
     * Sets the current start and length of the selection to new values.
     * @param offset index of the first sample
     * @param length number of samples
     */
    void selectRange(unsigned int offset, unsigned int length);

    /**
     * Inserts a new track with the size of the current signal after
     * the last track. The same as <c>insertTrack(tracks())</c>.
     */
    void appendTrack();

    /**
     * Inserts a new track in the current signal.
     * @param index position at which the new track will be
     *              inserted [0...tracks()]
     */
    void insertTrack(unsigned int index);

    /**
     * Deletes a track from the current signal, including generation
     * of an undo action.
     * @param index the index of the track to be deleted [0...tracks()-1]
     */
    void deleteTrack(unsigned int index);

    /**
     * Selects multiple tracks, all other tracks will be disabled.
     * @param track_list list od track indices
     */
    void selectTracks(QMemArray<unsigned int> &track_list);

    /**
     * Sets the selection flag of a track.
     * @param track index of the track [0..N-1]
     * @param select if true, the track will be enabled,
     *               if false it will be disabled
     */
    void selectTrack(unsigned int track, bool select);

    /**
     * Opens an input stream for a track, starting at a specified sample
     * position.
     * @param track index of the track. If the track does not exist, this
     *        function will fail and return 0
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @param with_undo if true, an undo action will be created
     * @see InsertMode
     */
    SampleWriter *openSampleWriter(unsigned int track, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0,
	bool with_undo = false);

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

    /**
     * Returns a set of opened SampleReader objects for reading from
     * multiple tracks. The list of tracks may contain indices of tracks
     * in any order and even duplicate entries are allowed. One useful
     * application is passing the output of selectedTracks() in order
     * to read from only from selected tracks.
     * @param readers reference to the MultiTrackReader to be filled.
     * @note the returned vector has set "autoDelete" to true, so you
     *       don't have to care about cleaning up
     * @param track_list array of indices of tracks for reading
     * @param first index of the first sample
     * @param last index of the last sample
     * @see SampleReader
     * @see selectedTracks()
     * @see Signal::openMultiTrackWriter
     */
    void openMultiTrackReader(MultiTrackReader &readers,
	const QMemArray<unsigned int> &track_list,
	unsigned int first, unsigned int last);

    /**
     * Opens a set of SampleWriters and internally handles the creation of
     * needed undo information. This is useful for multi-channel operations.
     * @param writers reference to a vector that receives all writers.
     * @param track_list list of indices of tracks to be modified.
     * @param mode specifies where and how to insert
     * @param left start of the input (only useful in insert and
     *             overwrite mode)
     * @param right end of the input (only useful with overwrite mode)
     * @see Signal::openMultiTrackWriter
     */
    void openMultiTrackWriter(MultiTrackWriter &writers,
	const QMemArray<unsigned int> &track_list, InsertMode mode,
	unsigned int left, unsigned int right);

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

    /**
     * Deletes the current selection and inserts the content of the
     * clipboard at the position of the first selected sample.
     * If the clipboard contains more or less tracks than the selection,
     * the signal will be mixed over all tracks.
     */
    void paste(ClipBoard &clipboard, unsigned int offset, unsigned int length);

    /**
     * Sets a complete set of file infos, including undo information
     */
    void setFileInfo(FileInfo &new_info, bool with_undo = true);

signals:

    /**
     * Signals a change in the range of selected samples.
     * @param offset index of the first selected sample
     * @param length number of selected samples
     */
    void sigSelectionChanged(unsigned int offset, unsigned int length);

    /**
     * Signals that a track has been inserted.
     * @param index position of the new track [0...tracks()-1]
     * @param track reference to the new track
     */
    void sigTrackInserted(unsigned int index, Track &track);

    /**
     * Signals that a track has been deleted.
     * @param index position of the deleted track [0...tracks()-1]
     */
    void sigTrackDeleted(unsigned int index);

    /**
     * Emits status information of the signal if it has been changed
     * or become valid.
     * @param length number of samples
     * @param tracks number of tracks
     * @param rate sample rate [samples/second]
     * @param bits resolution in bits
     */
    void sigStatusInfo(unsigned int length, unsigned int tracks,
                       double rate, unsigned int bits);

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
     * Emitted if a command has to be executed by
     * the next higher instance.
     * @param command the command to be executed
     */
    void sigCommand(const QString &command);

    /**
     * Signals that a track has been selected or deselected.
     * @param track index of the track
     * @param select true if the track has been selected,
     *               false if deselected
     */
    void sigTrackSelected(unsigned int track, bool select);

    /**
     * Emitted if the state or description of undo/redo has changed. If
     * undo or redo is unavailable the description will be zero.
     * @see emitUndoRedoInfo
     */
    void sigUndoRedoInfo(const QString &undo, const QString &redo);

    /**
     * Emitted if the signal changes from non-modified to modified
     * state or vice-versa.
     * @param modified true if now modified, false if no longer
     */
    void sigModified(bool modified);

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
     * @param index numeric index of the inserted track
     * @param track reference to the track that has been inserted
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &track);

    /**
     * Connected to the signal's sigTrackInserted.
     * @param index numeric index of the inserted track
     * @see Signal::sigTrackDeleted
     * @internal
     */
    void slotTrackDeleted(unsigned int index);

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

    friend class UndoInsertAction;

    /**
     * Deletes a range of samples.
     * @note only for internal usage in the UndoInsertAction!
     * @param track index of the track
     * @param offset index of the first sample
     * @param length number of samples
     * @see Signal::deleteRange
     */
    inline void deleteRange(unsigned int track, unsigned int offset,
                            unsigned int length)
    {
	m_signal.deleteRange(track, offset, length);
    };

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
     *       will be ignored and not freed, the return value will
     *       be false. So it is safer not to call this function if
     *       undo is not enabled.
     */
    bool registerUndoAction(UndoAction *action);

    /**
     * Saves undo data for deleting a range of samples from a list
     * of tracks.
     * @param track_list list of indices of tracks
     * @param offset first sample position to delete
     * @param length number of samples to delete
     * @return true if successful, false if out of memory or aborted
     */
    bool saveUndoDelete(QMemArray<unsigned int> &track_list,
                        unsigned int offset, unsigned int length);

    /**
     * Aborts an undo transaction by deleting all of it's undo actions.
     */
    void abortUndoTransaction();

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

    /**
     * Sets the modified flag to a new value if m_modified_enabled is
     * true, otherwise it will be ignored.
     */
    void setModified(bool mod);

private:

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
     * Enables changes of the modified flag.
     * @param en new value for m_modified_enabled
     */
    void enableModifiedChange(bool en);

private:

    /** Parent widget, used for showing messages */
    QWidget *m_parent_widget;

    /** name of the signal, normally equal to the filename */
    QString m_name;

    /** true if the signal is closed */
    bool m_closed;

    /** true if the signal is completely empty */
    bool m_empty;

    /** true if the signal has been modified */
    bool m_modified;

    /**
     * If set to true, prevents the modified flag from changes. Useful
     * to prevent setting the modified flag during file load and creation,
     * or if the change to the non-modified state through undo operations
     * is no longer possible.
     */
    bool m_modified_enabled;

    /** signal with multiple tracks */
    Signal m_signal;

    /** the current selection */
    Selection m_selection;

    /**
     * Last known length of the signal. This will be used if a track is
     * added to an empty signal and prevents from the creation of a
     * completely empty new signal.
     */
    unsigned int m_last_length;

    /** the controller for handling of playback */
    PlaybackController m_playback_controller;

    /** flag for "undo enabled" */
    bool m_undo_enabled;

    /** fifo used for storing all undo transactions */
    QPtrList<UndoTransaction> m_undo_buffer;

    /** fifo for storing all redo transactions */
    QPtrList<UndoTransaction> m_redo_buffer;

    /** the current undo transaction */
    UndoTransaction *m_undo_transaction;

    /** level of nested undo transactions */
    unsigned int m_undo_transaction_level;

    /** mutex for locking undo transactions */
    QMutex m_undo_transaction_lock;

    /** SignalProxy for emitting sigUndoRedoInfo thread-safe */
    SignalProxy<void> m_spx_undo_redo;

    /** maximum memory for undo */
    unsigned int m_undo_limit;

    /** info about the file, @see class FileInfo */
    FileInfo m_file_info;
};

#endif  /* _SIGNAL_MANAGER_H_ */
