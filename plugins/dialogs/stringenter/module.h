#ifndef _DIALOGS_STRING_H_
#define _DIALOGS_STRING_H 1

#include <qlineedit.h>
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
//*****************************************************************************
class StringEnterDialog : public Dialog
{
 Q_OBJECT

 public:

 	StringEnterDialog 	(const char *name,bool modal);
 	~StringEnterDialog 	();
 const char   *getCommand();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLineEdit	*name;
 QPushButton	*ok,*cancel;
};
#endif
