#ifndef _DIALOGS_SWEEP_H_
#define _DIALOGS_SWEEP_H 1

#include <qdialog.h>
#include <kintegerline.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../libgui/curvewidget.h"
#include "../libgui/guiitems.h"
//*****************************************************************************
class SweepDialog : public QWidget
{
 Q_OBJECT

 public:
 	     SweepDialog 	(QWidget *parent,int rate,char *name);
 	     ~SweepDialog 	();
 int         getTime();
 double      getLowFreq();
 double      getHighFreq();
 int         getInterpolationType();
 const char* getPoints ();
 void        convert (Curve *);

 public slots:

 void import       ();
 void showTime     (int);
 void showLowFreq  (int);
 void showHighFreq (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 TimeLine       *time;
 QLabel         *timelabel; 
 QComboBox      *note1,*note2;
 QLabel         *notelabel1,*notelabel2; 
 KIntegerLine   *lowfreq,*highfreq;
 CurveWidget	*curve;
 QPushButton	*load;
 int rate;
};
#endif
