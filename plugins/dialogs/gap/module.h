#ifndef _DIALOGS_Gap_H_
#define _DIALOGS_Gap_H 1
#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include "../../../libgui/guiitems.h"
#include <kintegerline.h>

//*****************************************************************************
class GapDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	GapDialog 	(int,bool);
 	~GapDialog 	();

	const char *getCommand ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine	*len1,*len2;
 QCheckBox      *insert;
 QLabel		*label1,*label2;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
#endif
