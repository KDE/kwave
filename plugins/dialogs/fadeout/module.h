#include "../../../libgui/kwavedialog.h"
#include "../../../libgui/faderwidget.h"
#include "../../../libgui/scale.h"
#include "../../../libgui/slider.h"
#include <libkwave/dialogoperation.h>

class FadeDialog : public KwaveDialog
{
 Q_OBJECT

 public:
 	FadeDialog 	(bool modal,int ms);
 	~FadeDialog 	();
 const char *getCommand();

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 ScaleWidget       *x,*y;
 CornerPatchWidget *corner;
 QPushButton	*ok,*cancel;
 KwaveSlider	*slider;
 FaderWidget	*fade;
 char           *comstr;
};
