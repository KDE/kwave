#ifndef _DIALOGS_AMPLIFY_H_
#define _DIALOGS_AMPLIFY_H 1

#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/curvewidget.h" 
#include "../../../libgui/scale.h" 
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"
#include <kintegerline.h>
//*****************************************************************************
class AmplifyCurveDialog : public KwaveDialog
{
  Q_OBJECT

    public:
  AmplifyCurveDialog 	(int time,bool modal);
  ~AmplifyCurveDialog 	();
  const char *getCommand ();

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  CurveWidget	    *curve;
  ScaleWidget       *xscale,*yscale;
  CornerPatchWidget *corner;
  QPushButton	    *ok,*cancel;
  char              *comstr;
};
#endif



