#ifndef _DIALOGS_MARKSAVE_H_
#define _DIALOGS_MARKSAVE_H 1
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include <libkwave/globals.h>
#include <qlabel.h>
#include <qlistbox.h>

//*****************************************************************************
class MarkSaveDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	MarkSaveDialog 	(Global *,bool modal);

 	~MarkSaveDialog ();

	const char * getCommand();

 public slots:

   void selectAll ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 Global         *globals;
 QListBox	*save;
 QPushButton	*ok,*cancel,*all;
 int            maxcnt;
 char           *comstr;
 bool           selectall;
};
#endif
