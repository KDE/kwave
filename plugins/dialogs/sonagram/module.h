#ifndef _DIALOGS_SONAGRAM_H_
#define _DIALOGS_SONAGRAM_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/Slider.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <kintegerline.h>
//*****************************************************************************
class SonagramDialog : public Dialog
{
 Q_OBJECT

 public:

 	SonagramDialog 	(bool,int,int);
 	~SonagramDialog 	();
 const  char *	getCommand   ();

 public slots:
   void setPoints (int);
   void setBoxPoints (int);
 protected:

 void resizeEvent (QResizeEvent *);

 private:

 Slider 	*pointslider;
 QLabel		*pointlabel;
 QLabel		*windowlabel;
 QLabel		*bitmaplabel;
 QLabel		*windowtypelabel;
 QComboBox	*pointbox;
 QComboBox	*windowtypebox;
 QPushButton	*ok,*cancel;
 int            length,rate;
 char           *comstr;
};
#endif
