#ifndef MEMORY_DIALOG_ 
#define MEMORY_DIALOG_ 1

#include <qlabel.h>
#include <qlineedit.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include <kintegerline.h>
//*****************************************************************************
class MemoryDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 MemoryDialog 	  (bool modal);
 ~MemoryDialog 	  ();
 const char *getCommand ();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*dirlabel;
 QLineEdit	*dir;
 QPushButton	*browse;
 QLabel		*memlabel;
 KIntegerLine   *mem;

 QPushButton	*ok,*cancel;
 char           *comstr;
};
#endif







