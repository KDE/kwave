#ifndef _DIALOGS_DENTER_H
#define _DIALOGS_DENTER_H 1

#include <kintegerline.h>
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>

//*****************************************************************************
class DoubleEnterDialog : public KwaveDialog
{
 Q_OBJECT

 public:
 	DoubleEnterDialog 	(const char *name,bool=false);
 	~DoubleEnterDialog 	();
	double value();

 const char *getCommand         ();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 KRestrictedLine *val;
 QPushButton	 *ok,*cancel;
};
//*****************************************************************************
#endif  /* dialogs.h */   





