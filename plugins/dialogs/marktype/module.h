#ifndef _DIALOGS_MARKTYPE_H_
#define _DIALOGS_MARKTYPE_H 1

#include <qlabel.h>
#include <qcheckbox.h>
#include <kintegerline.h>
#include <kcolordlg.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include "../../../lib/color.h"

//*****************************************************************************
class MarkerTypeDialog : public KwaveDialog
{
 Q_OBJECT

 public:
 	MarkerTypeDialog 	(bool modal);
 	~MarkerTypeDialog 	();
	const char *getCommand();

 public slots:

   void setColor(const Color &col);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KColorCombo    *color;
 QCheckBox      *individual;
 QLabel		*namelabel;
 QLineEdit	*name;
 QPushButton	*ok,*cancel;
 Color          col;
 char           *comstr;
};
#endif
