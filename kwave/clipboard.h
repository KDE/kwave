#ifndef _KWAVECLIPBOARD_H_
#define _KWAVECLIPBOARD_H_ 1
#include "sample.h"

class ClipBoard
{
 public:
  ClipBoard::ClipBoard (MSignal *signal);
  ClipBoard::~ClipBoard ();
  void toWindow ();
  int getLength ();
  void appendChannel (MSignal *);
  MSignal *getSignal ();

 private:
  MSignal *signal;
};
#endif
