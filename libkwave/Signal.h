/***************************************************************************
   Signal.h - representation of a Kwave signal with multiple tracks
			     -------------------
    begin                : Sat Feb 03 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#define PROGRESS_SIZE (512 * 3 * 5)

#include "config.h"
#include <limits.h>
#include <pthread.h>

#include <QReadWriteLock>
#include <QList>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/ReaderMode.h"
#include "libkwave/Sample.h"
#include "libkwave/WindowFunction.h"

class MultiTrackReader;
class MultiTrackWriter;
class SampleReader;
class SampleWriter;
class Track;

//**********************************************************************
class KDE_EXPORT Signal: public QObject
{
    Q_OBJECT

public:

    /**
     * Default Constructor. Creates an empty signal with
     * zero-length and no tracks
     */
    Signal();

    /**
     * Constructor. Creates an empty signal with a specified
     * number of tracks and length. Each track will contain
     * only one stripe.
     */
    Signal(unsigned int tracks, unsigned int length);

    /**
     * Destructor.
     */
    virtual ~Signal();

    /**
     * Closes the signal by removing all tracks.
     */
    void close();

    /**
     * Inserts a new track to into the track list or appends it to the end.
     * @param index the position where to insert [0...tracks()]. If the
     *        position is at or after the last track, the new track will
     *        be appended to the end.
     * @param length number of samples of the new track. Optional, if omitted
     *        the track will be zero-length.
     * @return pointer to the created track. If the length is
     *         omitted or zero, the track will have zero length.
     */
    Track *insertTrack(unsigned int index, unsigned int length = 0);

    /**
     * Appends a new track to the end of the tracks list, shortcut for
     * insertTrack(tracks()-1, length)
     * @see insertTrack
     */
    Track *appendTrack(unsigned int length = 0);

    /**
     * Deletes a track.
     * @param index the index of the track to be deleted [0...tracks()-1]
     */
    void deleteTrack(unsigned int index);

    /**
     * Returns an array of indices of all present tracks.
     */
     QList<unsigned int> allTracks();

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
     * @param mode a reader mode, see Kwave::ReaderMode
     * @param track index of the track. If the track does not exist, this
     *        function will fail and return 0
     * @param left first offset to be read (default = 0)
     * @param right last position to read (default = UINT_MAX)
     */
    SampleReader *openSampleReader(Kwave::ReaderMode mode,
	unsigned int track, unsigned int left = 0,
	unsigned int right = UINT_MAX);

    /**
     * Returns the number of tracks.
     */
    unsigned int tracks();

    /**
     * Deletes a range of samples
     * @param track index of the track
     * @param offset index of the first sample
     * @param length number of samples
     */
    void deleteRange(unsigned int track, unsigned int offset,
                     unsigned int length);

    /**
     * Inserts some space at a given position
     * @param track index of the track
     * @param offset index of the first sample
     * @param length number of samples
     */
    void insertSpace(unsigned int track, unsigned int offset,
                     unsigned int length);

    /**
     * Returns the length of the signal. This is determined by
     * searching for the highest sample position of all tracks.
     */
    unsigned int length();

    /**
     * Queries if a track is selected. If the index of the track is
     * out of range, the return value will be false.
     */
    bool trackSelected(unsigned int track);

    /**
     * Sets the "selected" flag of a track.
     * @param track index of the track [0...tracks-1]
     * @param select true if the track should be selected,
     *               false for de-selecting
     */
    void selectTrack(unsigned int track, bool select);

signals:

    /**
     * Signals that a track has been inserted.
     * @param index position of the new track [0...tracks()-1]
     * @param track reference to the new track
     */
    void sigTrackInserted(unsigned int index, Track *track);

    /**
     * Signals that a track has been deleted.
     * @param index position of the deleted track [0...tracks()-1]
     */
    void sigTrackDeleted(unsigned int index);

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

private slots:

    /**
     * Connected to each track's sigSamplesInserted.
     * @param src source track
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Track::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(Track *src, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to each track's sigSamplesDeleted.
     * @param src source track
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Track::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(Track *src, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to each track's sigSamplesModified
     * @param src source track
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Track::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(Track *src, unsigned int offset,
                             unsigned int length);

private:

    /**
     * Looks up the index of a trackin the track list
     * @param track reference to the trac to be looked up
     * @returns index of the track [0...tracks()-1] or tracks() if not found
     */
    unsigned int trackIndex(const Track *track);

//    //signal modifying functions
//    void replaceStutter (int, int);
//    void delayRecursive (int, int);
//    void delay (int, int);
//    void movingFilter (Filter *filter, int tap, Curve *points, int low, int high);
//
//    //functions creating a new Object
//
//    void fft (int, bool);
//    void averageFFT (int points, window_function_t windowtype);

    /** list of tracks */
    QList<Track *> m_tracks;

    /** mutex for access to the track list */
    QReadWriteLock m_lock_tracks;

};

//**********************************************************************
#endif  /* _SIGNAL_H_ */
