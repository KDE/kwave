#ifndef _KWAVECLIPBOARD_H_
#define _KWAVECLIPBOARD_H_ 1
#include "signalmanager.h"

class KwaveSignal;

class ClipBoard
{
 public:
  ClipBoard::ClipBoard  ();
  ClipBoard::ClipBoard  (SignalManager *signal);
  ClipBoard::~ClipBoard ();

  void toWindow ();
  void appendChannel  (KwaveSignal *);

  inline SignalManager *getSignal () {return signal;};
  inline int            getLength ()
    {if (signal) return signal->getLength(); else return 0;};

 private:
  SignalManager *signal;
};
#endif
