#include <ktopwidget.h>
#include <kstatusbar.h>  
#include "../libgui/ScaleWidget.h"

class PitchWidget;
class PitchContainer;
//***********************************************************************
class PitchWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
 	PitchWindow	(const char *name);
 	~PitchWindow	();
 void 	setSignal	(float *,int,int);

 public slots:

 void freqRange (float,float);
 void showPitch (float);
 void showTime  (float);

 signals:

 protected:

 private:
 PitchWidget       *view;
 PitchContainer    *mainwidget;
 ScaleWidget       *xscale,*yscale;
 KStatusBar        *status;
 CornerPatchWidget *corner;
 int rate;
};
