#ifndef _DIALOGS_SONAGRAM_H_
#define _DIALOGS_SONAGRAM_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/slider.h"
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include <kintegerline.h>
//*****************************************************************************
class SonagramDialog : public KwaveDialog
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

 KwaveSlider	*pointslider;
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
