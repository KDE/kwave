#ifndef _KWAVE_SIGNAL_H_
#define _KWAVE_SIGNAL_H_ 1

#define PROGRESS_SIZE 512*3*5

#include <pthread.h>

#include "libkwave/WindowFunction.h"

class Curve;
class Filter;
class Interpolation;

//**********************************************************************
class Signal {
public:
    Signal (int *signal, int size, int rate);
    Signal (int size, int rate);
    ~Signal ();

    Signal *copyRange ();
    Signal *cutRange ();
    void cropRange ();
    void deleteRange ();
    void overwritePaste (Signal *);
    void insertPaste (Signal *);
    void mixPaste (Signal *);

    void getMaxMin ( int& max, int& min, int begin, int len);
    int getSingleSample (int offset);

    inline int *getSample() {
	return sample;
    };
    inline int getRate() {
	return rate;
    };
    inline int getBits() {
	return bits;
    };
    inline int getLength() {
	return length;
    };
    inline int getLMarker() {
	return lmarker;
    };
    inline int getRMarker() {
	return rmarker;
    };
    inline int isSelected() {
	return selected;
    };
    inline void select(const bool select) {
	selected = select;
    }
    inline void setRate(const int rate) {
	this->rate = rate;
    };
    inline void setBits(const int bits) {
	this->bits = bits;
    };

    int getChannelMaximum ();

    int getLockState ();    //returns lock-state of signal

////    bool command (TimeOperation *);
    void setMarkers (int, int);
    void resample (const char *);   //int

protected:
private:
    //locking information - not yet used

    void lockRead ();
    void lockWrite ();
    void unlockRead ();
    void unlockWrite ();

    //memory management

    void noMemory ();
    void getridof (int *mem);
    int *getNewMem (int size);

    //attribute modifying functions

    void changeRate (int);

    //signal modifying functions
    void replaceStutter (int, int);
    void delayRecursive (int, int);
    void delay (int, int);
    void movingFilter (Filter *filter, int tap, Curve *points, int low, int high);

    //functions creating a new Object

    void fft (int, bool);
    void averageFFT (int points, window_function_t windowtype);

private:

    int *sample;                 //samples, linear in memory
    int length;                  //number of samples
    int rate;                    //sampling rate being used
    int bits;                    //bits per sample
    int rmarker, lmarker;         //selection markers
    bool selected;                // selection flag (default=true)
    int counter;
    int begin, len;

    char locked;                  //boolean if sample is locked (interthread-semaphore)
    char speaker;                 //code for playback speaker (e.g. left or right), will be used in future...
    char mapped;                  //flag if memory allocated is mmapped

}
;

struct Member //just for listing the mapping of functions to names
{
    void (Signal::*function) ();
    const char *name;
};

//**********************************************************************
struct MemberwithParams //just for listing the mapping of functions to names
{
    void (Signal::*function) (const char *);
    const char *name;
};

//**********************************************************************
#endif  /* signal.h */
