#ifndef MEMORY_DIALOG_ 
#define MEMORY_DIALOG_ 1

#include <qlabel.h>
#include <qlineedit.h>
#include "../../../libgui/Dialog.h"
#include "../../../lib/DialogOperation.h"
#include "../../../lib/Globals.h"
#include <kintegerline.h>
#include <ktreelist.h>

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







