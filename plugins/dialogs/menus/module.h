#ifndef MEMORY_DIALOG_ 
#define MEMORY_DIALOG_ 1

#include <qlabel.h>
#include <qlineedit.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include "../../../lib/globals.h"
#include <kintegerline.h>
#include <ktreelist.h>

//*****************************************************************************
class MenuDialog : public KwaveDialog
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







