#ifndef _KWAVE_SIGNAL_H_
#define _KWAVE_SIGNAL_H_ 1

#define PROGRESS_SIZE 512*3*5

#include <pthread.h>

#include <qlist.h>

#include "mt/Mutex.h"

#include "libkwave/InsertMode.h"
#include "libkwave/Sample.h"
#include "libkwave/WindowFunction.h"

class SampleInputStream;
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
     * Appends a new track to the end of the tracks list
     * and returns a pointer to the created track. If the
     * length is omitted or zero, the track will not have
     * a length.
     */
    Track *appendTrack(unsigned int length = 0);

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
    SampleInputStream *openInputStream(unsigned int track, InsertMode mode,
	unsigned int left = 0, unsigned int right = 0);

    /**
     * Returns the number of tracks.
     */
    unsigned int tracks();

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

    inline int rate() {
	return m_rate;
    };

    inline int bits() {
	return m_bits;
    };

    /**
     * Returns the length of the signal. This is determined by
     * searching for the highest sample position of all tracks.
     */
    unsigned int length();

    inline unsigned int selectionStart() {
	return m_selection_start;
    };

    inline unsigned int selectionEnd() {
	return m_selection_end;
    };

    inline int isSelected() {
	return m_selected;
    };

    inline void select(const bool select) {
	m_selected = select;
    };

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
    Mutex m_lock_tracks;

    /** number of samples */
    unsigned int m_length;

    /** sample rate [samples per second] */
    unsigned int m_rate;

    /** resolution in bits per sample */
    unsigned int m_bits;

    /** selection flag (default=true) */
    bool m_selected;

    /** start of the selection */
    unsigned int m_selection_start;

    /** end of the selection */
    unsigned int m_selection_end;

};

//**********************************************************************
#endif  /* signal.h */
