
#include "libgui/Dialog.h"

//*****************************************************************************

class QLabel;
class QComboBox;
class KIntegerLine;
class QPushButton;
class Curve;
class CurveWidget;

class SweepDialog : public Dialog
{
 Q_OBJECT

 public:
 	     SweepDialog 	(int rate,bool);
 	     ~SweepDialog 	();
 int         getTime();
 double      getLowFreq();
 double      getHighFreq();
 int         getInterpolationType();
 const char *getPoints ();
 const char *getCommand ();
 void        convert (Curve *);

 public slots:

 void import       ();
 void showTime     (int);
 void showLowFreq  (int);
 void showHighFreq (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 QComboBox      *note1,*note2;
 QLabel         *notelabel1,*notelabel2; 
 KIntegerLine   *lowfreq,*highfreq;
 CurveWidget	*curve;
 QPushButton	*load,*ok,*cancel;
 char           *command;
 int             rate;
};
