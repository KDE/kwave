#ifndef _DIALOGS_ENVELOPE_H_
#define _DIALOGS_ENVELOPE_H 1

#include <qlabel.h>
#include "../../../libgui/slider.h"
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include <kintegerline.h>
//*****************************************************************************
class EnvelopeDialog : public KwaveDialog
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

 KwaveSlider	*timeslider;
 QLabel		*timelabel;
 QLabel		*typelabel;
 QComboBox	*typebox;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
#endif
