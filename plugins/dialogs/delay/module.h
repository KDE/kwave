#ifndef _DIALOGS_DELAY_H_
#define _DIALOGS_DELAY_H_ 1

#include <qlabel.h>
#include <qcheckbox.h>
#include "../../../libgui/slider.h"
#include "../../../libgui/guiitems.h"
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>

//****************************************************************************
class DelayDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	DelayDialog 	(int rate,bool modal);
 	~DelayDialog 	();

const char *getCommand();

 public slots:

 void setAmpl (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 TimeLine       *delay;
 QLabel		*delaylabel;
 KwaveSlider	*amplslider;
 QLabel		*ampllabel;
 QCheckBox	*recursive;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
//****************************************************************************
#endif
