#ifndef _DIALOGS_DISTORT_H_
#define _DIALOGS_DISTORT_H 1
#include <qlist.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include "../../../libgui/curvewidget.h"
#include "../../../libgui/scale.h"
//*****************************************************************************
class DistortDialog : public KwaveDialog
{
  Q_OBJECT

    public:
  DistortDialog 	(bool);
  ~DistortDialog 	();

  const char *getCommand ();

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  ScaleWidget   *xscale,*yscale;
  CornerPatchWidget *corner;
  QComboBox     *sym;
  QPushButton	*ok,*cancel;
  CurveWidget	*curve;
  char          *comstr;
};
#endif
