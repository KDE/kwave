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
class Signal
{
public:
    /**
     * Default Constructor. Creates an empty signal with
     * zero-length and no tracks
     */
    Signal();

    /**
     * Copy constructor.
     */
//    Signal(const Signal &sig);

    /**
     * Constructor. Creates an empty signal with a specified
     * number of tracks and length. Each track will contain
     * only one stripe.
     */
    Signal(unsigned int tracks, unsigned int length);

    /**
     * Destructor.
     */
    ~Signal ();

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

    inline unsigned int length() {
	return m_length;
    };

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

private:

//    //memory management
//
//    void noMemory ();
//    void getridof (int *mem);
//    int *getNewMem (int size);
//
//    //attribute modifying functions
//
//    void changeRate (int);
//
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

private:

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
