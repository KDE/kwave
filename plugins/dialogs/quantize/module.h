#ifndef _DIALOGS_QUANTISE_H_
#define _DIALOGS_QUANTISE_H 1

#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <kintegerline.h>

//*****************************************************************************
class QuantiseDialog : public Dialog
{
 Q_OBJECT

 public:

 	QuantiseDialog 	(bool=false);
 	~QuantiseDialog ();

 const char *getCommand ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KIntegerLine	*bits;
 QCheckBox      *linear;
 QLabel		*bitlabel;
 QPushButton	*ok,*cancel;
 char           *comstr;
};
#endif
