#ifndef MEMORY_DIALOG_ 
#define MEMORY_DIALOG_ 1

#include "libgui/Dialog.h"

class MenuDialog;
class KTreeList;
class QPushButton;
class Global;

//*****************************************************************************
class MenuDialog : public Dialog
{
 Q_OBJECT

 public:

 MenuDialog 	  (Global*,bool modal);
 ~MenuDialog 	  ();
 const char *getCommand ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KTreeList      *source;
 KTreeList      *dest;
 QPushButton	*ok,*cancel;
 Global         *globals;
};
#endif







