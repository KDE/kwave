#ifndef _DIALOGS_NEWSAMPLE_H_
#define _DIALOGS_NEWSAMPLE_H 1
#include <stdlib.h>
#include <qcombobox.h>
#include <qlabel.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include "../../../libgui/guiitems.h"
#include <kintegerline.h>
//*****************************************************************************
class NewSampleDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	NewSampleDialog 	(bool modal);
 	~NewSampleDialog 	();
	const char *getCommand  ();

 public slots:

 void setRate   (const char *);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine	*time;
 QLabel		*timelabel;
 QLabel		*ratelabel;
 QComboBox	*ratefield;
 QPushButton	*ok,*cancel;
 char *          comstr;
};
#endif
