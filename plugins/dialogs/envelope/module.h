#ifndef _DIALOGS_ENVELOPE_H_
#define _DIALOGS_ENVELOPE_H 1

#include <qlabel.h>
#include "../../../libgui/Slider.h"
#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <kintegerline.h>
//*****************************************************************************
class EnvelopeDialog : public Dialog
{
 Q_OBJECT

 public:

 	EnvelopeDialog 	(bool);
 	~EnvelopeDialog ();

 const char *getCommand();

 public slots:

 void setTime (int);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 Slider 	*timeslider;
 QLabel		*timelabel;
 QLabel		*typelabel;
 QComboBox	*typebox;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
#endif
