#ifndef _FORMANT_DIALOG_H_
#define _FORMANT_DIALOG_H_ 1

#include <qpushbt.h>
#include <qlabel.h>
#include "../../../libgui/kwavedialog.h"
#include "../../../libgui/scale.h"
#include "../../../libgui/slider.h"
#include "../../../libgui/formantwidget.h"
#include "../../../lib/dialogoperation.h"
#include <kintegerline.h>

//***********************************************************************
class FormantDialog : public KwaveDialog
{
 Q_OBJECT

 public:
 	FormantDialog 	(bool modal,int rate);
 	~FormantDialog 	();
 int    getCurve ();
 void   getWidgets (int);
 const  char *getCommand ();

 public slots:

 void posChanged    (int);
 void setScale      (int,int);
 void widthChanged  (int);
 void numberChanged (const char *);
 void posChanged    (const char *);
 void widthChanged  (const char *);

 protected:

 void resizeEvent (QResizeEvent *);
 void refresh     ();

 private:

 ScaleWidget       *x,*y;
 CornerPatchWidget *corner;
 KIntegerLine   *num;
 KIntegerLine   **pos;
 KIntegerLine   **widths;
 KwaveSlider    **widthslider;
 KwaveSlider    **posslider;
 QLabel         *poslabel;
 QLabel         *numlabel;
 QPushButton	*ok,*cancel;
 FormantWidget	*formant;

 char           *comstr;
 int            oldnum;
 int            rate;
 bool           inwidget;
};
#endif
