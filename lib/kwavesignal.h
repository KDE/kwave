#ifndef _KWAVE_SIGNAL_H_
#define _KWAVE_SIGNAL_H_ 1

#define PROGRESS_SIZE 512*3*5

#include <pthread.h>

#include "../lib/interpolation.h"
#include "../lib/curve.h"
#include "timeoperation.h"
class Filter;
//**********************************************************************
class KwaveSignal
{
 public:
  KwaveSignal     (int *signal,int size,int rate);
  KwaveSignal     (int size,int rate);
  ~KwaveSignal	  ();

  KwaveSignal *copyRange     ();
  KwaveSignal *cutRange      ();
  void        cropRange      ();
  void        deleteRange    ();
  void        overwritePaste (KwaveSignal *);
  void        insertPaste    (KwaveSignal *);
  void        mixPaste       (KwaveSignal *);

  void   getMaxMin          ( int& max,int& min,int begin,int len);
  int	 getSingleSample    (int offset);

  inline int 	*getSample  ()          {return sample;};
  inline int 	getRate	    ()          {return rate;};
  inline int 	getLength   ()          {return length;};
  inline int 	getLMarker  ()          {return lmarker;};
  inline int 	getRMarker  ()          {return rmarker;};
  inline int 	*getCounter ()        	{return &counter;};
  inline void 	setRate	    (int rate)  {this->rate=rate;};

  int    getChannelMaximum   ();

  int    getLockState        ();  //returns lock-state of signal

  void  command              (TimeOperation *);
  void  setMarkers           (int,int);
  void	fadeIn     	     (const char *); //int
  void	fadeOut              (const char *); //int
  void 	quantize             (const char *); //int
  void  average              (const char *); //int
  void  resample             (const char *); //int

 protected:
 private:
  //locking information - not yet used

  void   lockRead        ();
  void   lockWrite       ();
  void   unlockRead      ();
  void   unlockWrite     ();

  //memory management

  void   noMemory        ();
  void   getridof        (int *mem);
  int    *getNewMem      (int size);

  //attribute modifying functions

  void	changeRate      (int);

  //signal modifying functions
  void   hullCurve            (int,int);
  void   replaceStutter       (int,int);
  void   delayRecursive       (int,int);
  void   delay        	      (int,int);
  void   filter               (Filter *filter);
  void   movingFilter         (Filter *filter,int tap, Curve *points,int low,int high);

  //functions creating a new Object

  void   sonagram             (int,int);
  void   fft                  (int,bool);
  void   averageFFT           (int,int);

 private: 

  int	 	*sample;               //samples, linear in memory
  int		length;                //number of samples
  int		rate;                  //sampling rate being used
  int		rmarker,lmarker;       //selection markers
  int           counter;
  int           begin,len;

  char           locked;                //boolean if sample is locked (interthread-semaphore)
  char           speaker;               //code for playback speaker (e.g. left or right), will be used in future...
  char           mapped;                //flag if memory allocated is mmapped

};
struct Member //just for listing the mapping of functions to names
{
  void (KwaveSignal::*function) ();
  const char *name;
};
//**********************************************************************
struct MemberwithParams //just for listing the mapping of functions to names
{
  void (KwaveSignal::*function) (const char *);
  const char *name;
};
//**********************************************************************
#endif  /* signal.h */   







