#ifndef _KWAVECLIPBOARD_H_
#define _KWAVECLIPBOARD_H_ 1

class MSignal;
class MenuManager;
class ClipBoard
{
 public:
  ClipBoard::ClipBoard  ();
  ClipBoard::ClipBoard  (MSignal *signal);
  ClipBoard::~ClipBoard ();
  void toWindow ();
  int  getLength ();
  void appendChannel  (MSignal *);
  void registerMenu   (MenuManager *);
  void unregisterMenu (MenuManager *);
  void setOp          (int);

  MSignal *getSignal ();

 private:
  MSignal *signal;
  bool hasmenu;
};
#endif
