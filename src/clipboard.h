#ifndef _KWAVECLIPBOARD_H_
#define _KWAVECLIPBOARD_H_ 1

#include "signalmanager.h"

class KwaveSignal;
class MenuManager;

class ClipBoard
{
 public:
  ClipBoard::ClipBoard  ();
  ClipBoard::ClipBoard  (SignalManager *signal);
  ClipBoard::~ClipBoard ();

  void toWindow ();
  int  getLength ();
  void appendChannel  (KwaveSignal *);
  void registerMenu   (MenuManager *);
  void unregisterMenu (MenuManager *);
  void setOp          (const char *);

  SignalManager *getSignal ();

 private:
  SignalManager *signal;
  bool hasmenu;
};
#endif
