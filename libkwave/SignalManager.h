/***************************************************************************
         SignalManager.h -  manager class for multi channel signals
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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QString>

#include <kdemacros.h>

#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/Selection.h"
#include "libkwave/Signal.h"
#include "libkwave/undo/UndoManager.h"

class QBitmap;
class QFile;
class KUrl;

#define NEW_FILENAME i18n("New File")

namespace Kwave
{

    class UndoAction;
    class UndoInsertAction;
    class UndoTransaction;
    class UndoTransactionGuard;
    class MultiTrackWriter;
    class SampleReader;
    class SignalWidget;
    class Track;
    class Writer;

    /**
     * The SignalManager class manages multi channel signals.
     */
    class KDE_EXPORT SignalManager : public QObject
    {
	Q_OBJECT

    public:
	/** Default constructor. */
	SignalManager(QWidget *parent);

	/** Default destructor */
	virtual ~SignalManager();

    public:

	/**
	 * Closes the current signal and loads a new file.
	 * @param url URL of the file to be loaded
	 * @return 0 if succeeded or error code < 0
	 */
	int loadFile(const KUrl &url);

	/**
	 * Closes the current signal and creates a new empty signal.
	 * @param samples number of samples per track
	 * @param rate sample rate
	 * @param bits number of bits per sample
	 * @param tracks number of tracks
	 */
	void newSignal(sample_index_t samples, double rate,
	               unsigned int bits, unsigned int tracks);

	/**
	* Closes the current signal.
	*/
	void close();

	/** Returns true if the signal is closed */
	inline bool isClosed() { return m_closed; }

	/** Returns true if the signal is empty. */
	inline bool isEmpty() { return m_empty; }

	/** Returns true if the signal is modified */
	inline bool isModified() { return m_modified; }

	/** Returns a reference to the playback controller. */
	Kwave::PlaybackController &playbackController();

	/**
	* Execute a Kwave text command
	 * @param command a text command
	 * @return zero if succeeded or negative error code if failed
	 * @retval -ENOSYS is returned if the command is unknown in this component
	 */
	int executeCommand(const QString &command);

	/**
	 * Returns a reference to the current name of the signal. If no signal is
	 * loaded the string is zero-length.
	 */
	QString signalName();

	/**
	 * Returns the current sample resolution in bits per sample
	 */
	inline unsigned int bits() const {
	    return Kwave::FileInfo(m_meta_data).bits();
	}

	/** Returns the current sample rate in samples per second */
	inline double rate() const {
	    return Kwave::FileInfo(m_meta_data).rate();
	}

	/** Returns the current number of tracks */
	inline unsigned int tracks() { return m_signal.tracks(); }

	/**
	 * Returns the number of samples in the current signal. Will be
	 * zero if no signal is loaded.
	 */
	inline sample_index_t length() { return m_signal.length(); }

	/** Returns a reference to the current selection */
	inline Kwave::Selection &selection() { return m_selection; }

	/**
	 * Returns true if a given track is selected. If the track does
	 * not exist or is not selected the return value is false.
	 */
	inline bool trackSelected(unsigned int track) {
	    return (m_signal.trackSelected(track));
	}

	/**
	 * Returns an array of indices of currently selected tracks.
	 */
	const QList<unsigned int> selectedTracks();

	/**
	 * Returns an array of indices of all present tracks.
	 */
	const QList<unsigned int> allTracks();

	/**
	 * Saves the signal to a file with a given resolution. If the file
	 * already exists, it will be overwritten.
	 * @param url URL with the name of the file to be saved.
	 * @param selection if true, only the selected range will be saved
	 * @return zero if succeeded or negative error code
	 */
	int save(const KUrl &url, bool selection);

	/**
	 * Deletes a range of samples and creates an undo action.
	 * @param offset index of the first sample
	 * @param length number of samples
	 * @param track_list a list of tracks to be affected
	 * @return true if successful or nothing to do, false if not enough
	 *         memory for undo
	 */
	bool deleteRange(sample_index_t offset, sample_index_t length,
	                 const QList<unsigned int> &track_list);

	/**
	 * Deletes a range of samples and creates an undo action. Same as
	 * above, but affects all tracks.
	 * @param offset index of the first sample
	 * @param length number of samples
	 * @return true if successful or nothing to do, false if not enough
	 *         memory for undo
	 */
	bool deleteRange(sample_index_t offset, sample_index_t length);

	/**
	 * Inserts space at a given position and creates an undo action.
	 * @param offset position of the first sample to insert
	 * @param length number of samples
	 * @param track_list a list of tracks to be affected
	 * @return true if successful or nothing to do, false if not enough
	 *         memory for undo
	 */
	bool insertSpace(sample_index_t offset, sample_index_t length,
	                 const QList<unsigned int> &track_list);

	/**
	 * Sets the current start and length of the selection to new values.
	 * @param offset index of the first sample
	 * @param length number of samples
	 */
	void selectRange(sample_index_t offset, sample_index_t length);

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
	void selectTracks(QList<unsigned int> &track_list);

	/**
	 * Sets the selection flag of a track.
	 * @param track index of the track [0..N-1]
	 * @param select if true, the track will be enabled,
	 *               if false it will be disabled
	 */
	void selectTrack(unsigned int track, bool select);

	/**
	 * Opens an output stream for a track, starting at a specified sample
	 * position.
	 * @param mode specifies where and how to insert
	 * @param track index of the track. If the track does not exist, this
	 *        function will fail and return 0
	 * @param left start of the input (only useful in insert and
	 *             overwrite mode)
	 * @param right end of the input (only useful with overwrite mode)
	 * @see InsertMode
	 */
	inline Kwave::Writer *openWriter(Kwave::InsertMode mode,
	    unsigned int track,
	    sample_index_t left = 0, sample_index_t right = 0)
	{
	    return m_signal.openWriter(mode, track, left, right);
	}

	/**
	 * Opens a stream for reading samples. If the the last position
	 * is omitted, the value UINT_MAX will be used.
	 * @param mode a reader mode, see SampleReader::Mode
	 * @param track index of the track. If the track does not exist, this
	 *        function will fail and return 0
	 * @param left first offset to be read (default = 0)
	 * @param right last position to read (default = UINT_MAX)
	 */
	inline Kwave::SampleReader *openReader(Kwave::ReaderMode mode,
	    unsigned int track,
	    sample_index_t left = 0, sample_index_t right = SAMPLE_INDEX_MAX)
	{
	    return m_signal.openReader(mode, track, left, right);
	}


	/**
	 * Get a list of stripes that matches a given range of samples
	 * @param track_list list with indices of tracks for selecting
	 * @param left  offset of the first sample
	 * @param right offset of the last sample
	 * @return a list with lists of stripes that cover the given range
	 *         between left and right
	 */
	QList<Kwave::Stripe::List> stripes(
	    const QList<unsigned int> &track_list,
	    sample_index_t left = 0,
	    sample_index_t right = SAMPLE_INDEX_MAX);

	/**
	 * Merge a list of stripes into the signal.
	 * @note this operation works without undo!
	 * @param stripes list of stripe list (multi track)
	 * @param track_list list with indices of tracks for selecting
	 * @return true if succeeded, false if failed
	 */
	bool mergeStripes(const QList<Kwave::Stripe::List> &stripes,
	                  const QList<unsigned int> &track_list);

	/** Returns a reference to the undo manager */
	inline Kwave::UndoManager &undoManager() { return m_undo_manager; }

	/** Returns true if undo/redo is currently enabled */
	inline bool undoEnabled() const { return m_undo_enabled; }

	/** Return true if undo is possible */
	inline bool canUndo() const {
	    return !m_undo_buffer.isEmpty() && undoEnabled();
	}

	/** Return true if redo is possible */
	inline bool canRedo() const {
	    return !m_redo_buffer.isEmpty() && undoEnabled();
	}

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
	 * Sets a complete set of file infos, including undo information
	 * @param new_info a new FileInfo
	 * @param with_undo if true, store undo information
	 */
	void setFileInfo(Kwave::FileInfo &new_info, bool with_undo);

	/**
	 * add a new label, without undo
	 * @param pos position of the label [samples]
	 * @param name the name of the label
	 * @return pointer to the new created label
	 */
	Kwave::Label addLabel(sample_index_t pos, const QString &name);

	/**
	 * delete an existing label
	 * @param index the index of the label [0...N-1]
	 * @param with_undo if true, create undo info
	 */
	void deleteLabel(int index, bool with_undo);

	/**
	 * modify an existing label at a given index
	 * (always without undo)
	 * @param index the index of the label [0...N-1]
	 * @param pos position of the label [samples]
	 * @param name the name of the label
	 * @return true if succeeded, false if the index is out of range or if
	 *         the new position is already occupied by an existing label
	 */
	bool modifyLabel(int index, sample_index_t pos, const QString &name);

	/**
	 * Returns the index of a label, counting from zero
	 * @param label reference to a Label
	 * @return index [0...N-1] or -1 if label is a null pointer or not found
	 */
	int labelIndex(const Kwave::Label &label) const;

	/**
	 * returns the label at a given exact position
	 * @param pos position of the label [samples]
	 * @return valid label at the position or null label if not found
	 */
	Kwave::Label findLabel(sample_index_t pos);

	/**
	 * Retrieves the list of meta data objects, mutable
	 * @return list with all MetaData objects
	 */
	Kwave::MetaDataList &metaData() { return m_meta_data; };

	/**
	 * Retrieves the list of meta data objects, const
	 * @return reference to the list of all MetaData objects
	 */
	const Kwave::MetaDataList &metaData() const { return m_meta_data; }

	/**
	 * Returns the uuid of a track
	 * @param track index of the track [0...tracks-1]
	 * @return the QUuid of the track or a "null" uuid if the track
	 *         does not exist
	 */
	QUuid uuidOfTrack(unsigned int track) {
	    return m_signal.uuidOfTrack(track);
	}

    signals:

	/**
	 * Signals a change in the range of selected samples.
	 * @param offset index of the first selected sample
	 * @param length number of selected samples
	 */
	void sigSelectionChanged(sample_index_t offset, sample_index_t length);

	/**
	 * Signals that a track has been inserted.
	 * @param index position of the new track [0...tracks()-1]
	 * @param track reference to the new track
	 */
	void sigTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Signals that a track has been deleted.
	 * @param index position of the deleted track [0...tracks()-1]
	 * @param track reference to the new track
	 */
	void sigTrackDeleted(unsigned int index, Kwave::Track *track);

	/**
	 * Signals that the selection of one of the tracks has changed
	 * @param enabled state of the track, true=selected
	 */
	void sigTrackSelectionChanged(bool enabled);

	/**
	 * Emitted if samples have been inserted into a track. This implies
	 * a modification of the inserted data, so no extra sigSamplesModified
	 * is emitted.
	 * @param track index of the track
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see sigSamplesModified
	 */
	void sigSamplesInserted(unsigned int track, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Emitted if samples have been removed from a track.
	 * @param track index of the track
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 */
	void sigSamplesDeleted(unsigned int track, sample_index_t offset,
	                       sample_index_t length);

	/**
	 * Emitted if samples within a track have been modified.
	 * @param track index of the track
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 */
	void sigSamplesModified(unsigned int track, sample_index_t offset,
	                        sample_index_t length);

	/**
	* Emitted whenever meta data has changed, after some operation
	 * @param meta the current meta data
	 */
	void sigMetaDataChanged(Kwave::MetaDataList meta);

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
	void slotTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Connected to the signal's sigTrackInserted.
	 * @param index numeric index of the inserted track
	 * @param track reference to the track that has been deleted
	 * @see Signal::sigTrackDeleted
	 * @internal
	 */
	void slotTrackDeleted(unsigned int index, Kwave::Track *track);

	/**
	 * Connected to the signal's sigSamplesInserted.
	 * @param track index of the source track [0...tracks-1]
	 * @param offset position from which the data was inserted
	 * @param length number of samples inserted
	 * @see Signal::sigSamplesInserted
	 * @internal
	 */
	void slotSamplesInserted(unsigned int track, sample_index_t offset,
	                         sample_index_t length);

	/**
	 * Connected to the signal's sigSamplesDeleted.
	 * @param track index of the source track [0...tracks-1]
	 * @param offset position from which the data was removed
	 * @param length number of samples deleted
	 * @see Signal::sigSamplesDeleted
	 * @internal
	 */
	void slotSamplesDeleted(unsigned int track, sample_index_t offset,
	                        sample_index_t length);

	/**
	 * Connected to the signal's sigSamplesModified
	 * @param track index of the source track [0...tracks-1]
	 * @param offset position from which the data was modified
	 * @param length number of samples modified
	 * @see Signal::sigSamplesModified
	 * @internal
	 */
	void slotSamplesModified(unsigned int track, sample_index_t offset,
	                         sample_index_t length);

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

	friend class Kwave::UndoInsertAction;

	/**
	 * Deletes a range of samples.
	 * @note only for internal usage in the UndoInsertAction!
	 * @param track index of the track
	 * @param offset index of the first sample
	 * @param length number of samples
	 * @see Signal::deleteRange
	 */
	inline void deleteRange(unsigned int track, sample_index_t offset,
	                        sample_index_t length)
	{
	    m_signal.deleteRange(track, offset, length);
	}

    protected:

	friend class MultiTrackWriter;
	friend class PlaybackController;
	friend class PluginManager;
	friend class MainWidget;
	friend class UndoTransactionGuard;

	/**
	 * Tries to free memory for a new undo action and stores all needed
	 * data if successful.
	 * @param action UndoAction to that is to be registered
	 * @return true if the action is allowed, false if the user has
	 *         chosen to abort the operation if the memory limit of
	 *         the undo buffer would be exceeded. The return value
	 *         will also be false if the action is null.
	 * @note If undo is currently not enabled, the passed UndoAction
	 *       will be ignored and not freed, the return value will
	 *       be false. So it is safer not to call this function if
	 *       undo is not enabled.
	 */
	bool registerUndoAction(Kwave::UndoAction *action);

	/**
	 * Saves undo data for deleting a range of samples from a list
	 * of tracks.
	 * @param track_list list of indices of tracks
	 * @param offset first sample position to delete
	 * @param length number of samples to delete
	 * @return true if successful, false if out of memory or aborted
	 */
	bool saveUndoDelete(QList<unsigned int> &track_list,
	                    sample_index_t offset, sample_index_t length);

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
	void startUndoTransaction(const QString &name = QString());

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

	/** returns the associated parent widget, to be used for messages */
	QWidget *parentWidget() const { return m_parent_widget; }

    private:

	/**
	 * Ask the user if he wants to continue without undo, maybe
	 * registering an undo action has failed due to out-of-memory.
	 * @return true if it is ok, false if the user doesn't want to.
	 */
	bool continueWithoutUndo();

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

	/** saves the current sample and track selection */
	void rememberCurrentSelection();

	/**
	 * Check whether the selection has changed since the start of
	 * the last undo and create a new undo action if the selection
	 * has been modified (e.g. manually)
	 */
	void checkSelectionChange();

    private:

	/** Parent widget, used for showing messages */
	QWidget *m_parent_widget;

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
	Kwave::Signal m_signal;

	/** the current selection */
	Kwave::Selection m_selection;

	/** the last selection (stored in undo) */
	Kwave::Selection m_last_selection;

	/** the last track selection (stored in undo) */
	QList <unsigned int> m_last_track_selection;

	/**
	 * Last known length of the signal. This will be used if a track is
	 * added to an empty signal and prevents from the creation of a
	 * completely empty new signal.
	 */
	sample_index_t m_last_length;

	/** the controller for handling of playback */
	Kwave::PlaybackController m_playback_controller;

	/** flag for "undo enabled" */
	bool m_undo_enabled;

	/** fifo used for storing all undo transactions */
	QList<Kwave::UndoTransaction *> m_undo_buffer;

	/** fifo for storing all redo transactions */
	QList<Kwave::UndoTransaction *> m_redo_buffer;

	/** the current undo transaction */
	Kwave::UndoTransaction *m_undo_transaction;

	/** level of nested undo transactions */
	unsigned int m_undo_transaction_level;

	/** mutex for locking undo transactions */
	QMutex m_undo_transaction_lock;

	/** Manager for undo/redo actions */
	Kwave::UndoManager m_undo_manager;

	/**
	 * meta data of the signal
	 * @see class MetaData
	 */
	Kwave::MetaDataList m_meta_data;

    };
}

#endif  /* _SIGNAL_MANAGER_H_ */

//***************************************************************************
//***************************************************************************
