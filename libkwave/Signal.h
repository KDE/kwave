
#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#define PROGRESS_SIZE 512*3*5

#include <pthread.h>

#include <qlist.h>

#include "mt/SharedLock.h"

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/WindowFunction.h"

class MultiTrackReader;
class MultiTrackWriter;
class SampleReader;
class SampleWriter;
class Track;

//**********************************************************************
class Signal: public QObject
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
    ~Signal();

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
    const QArray<unsigned int> allTracks();

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
    SampleReader *openSampleReader(unsigned int track, unsigned int left = 0,
	unsigned int right = UINT_MAX);

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
     */
    void openMultiTrackReader(MultiTrackReader &readers,
	const QArray<unsigned int> &track_list,
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
     * @see InsertMode
     */
    void openMultiTrackWriter(MultiTrackWriter &writers,
	const QArray<unsigned int> &track_list, InsertMode mode,
	unsigned int left, unsigned int right);

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

//    Signal *copyRange ();
//    Signal *cutRange ();
//    void cropRange ();
//    void deleteRange ();
//    void overwritePaste (Signal *);
//    void insertPaste (Signal *);
//    void mixPaste (Signal *);
//
//    void getMaxMin ( int& max, int& min, int begin, int len);
//    int getSingleSample (int offset);

    inline unsigned int rate() {
	return m_rate;
    };

    inline unsigned int bits() {
	return m_bits;
    };

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

    inline void setRate(const int rate) {
	m_rate = rate;
    };

    inline void setBits(const int bits) {
	m_bits = bits;
    };

//    int getChannelMaximum ();
//
//    void setMarkers (int, int);
//    void resample (const char *);   //int

signals:

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
    void slotSamplesInserted(Track &src, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to each track's sigSamplesDeleted.
     * @param src source track
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Track::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(Track &src, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to each track's sigSamplesModified
     * @param src source track
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Track::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(Track &src, unsigned int offset,
                             unsigned int length);

private:

    /**
     * Looks up the index of a trackin the track list
     * @param track reference to the trac to be looked up
     * @returns index of the track [0...tracks()-1] or tracks() if not found
     */
    unsigned int trackIndex(const Track &track);

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
    QList<Track> m_tracks;

    /** mutex for access to the track list */
    SharedLock m_lock_tracks;

    /** number of samples */
    unsigned int m_length;

    /** sample rate [samples per second] */
    unsigned int m_rate;

    /** resolution in bits per sample */
    unsigned int m_bits;

};

//**********************************************************************
#endif  /* _SIGNAL_H_ */
