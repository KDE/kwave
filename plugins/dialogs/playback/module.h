#ifndef _PLAYBACK_DIALOGS_H_
#define _PLAYBACK_DIALOGS_H_ 1

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h> 
#include <qradiobutton.h> 
#include <qbuttongroup.h>

#include "../../../libgui/Slider.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
//*****************************************************************************
class PlayBackDialog : public Dialog
{
 Q_OBJECT

 public:

 PlayBackDialog 	(bool);
 ~PlayBackDialog 	();
 const char *getCommand ();

 public slots:

   void setBufferSize (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel		*bufferlabel;
 Slider         *buffersize;
 QLabel         *devicelabel;
 QComboBox      *devicebox;
 QCheckBox      *stereo;
 QLabel		*label1,*label2;
 QButtonGroup	*bg;
 QRadioButton	*b16,*b8;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
//*****************************************************************************
#endif
