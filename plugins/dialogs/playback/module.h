#ifndef _PLAYBACK_DIALOGS_H_
#define _PLAYBACK_DIALOGS_H_ 1

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h> 
#include <qradiobutton.h> 
#include <qbuttongroup.h>

#include "../../../libgui/slider.h"
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
//*****************************************************************************
class PlayBackDialog : public KwaveDialog
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
 KwaveSlider    *buffersize;
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
