#ifndef _KWAVECLIPBOARD_H_
#define _KWAVECLIPBOARD_H_ 1

class SignalManager;
class Signal;

class ClipBoard
{
public:
  ClipBoard::ClipBoard  ();
  ClipBoard::ClipBoard  (SignalManager *signal);
  ClipBoard::~ClipBoard ();

  void toWindow ();
  void appendChannel  (Signal *);

  SignalManager *getSignal();
  int            getLength();

private:
  SignalManager *signal;
};
#endif
