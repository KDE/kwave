#include "../../../libgui/Dialog.h"
#include "libkwave/DialogOperation.h"

class Curve;
class ScaleWidget;
class CurveWidget;
class CornerPatchWidget;
class TimeLine;
class Signal;
class QLabel;
class QPushButton;

class PulseDialog : public Dialog
{
  Q_OBJECT

    public:

  PulseDialog 	(int rate,int time,bool modal);

  ~PulseDialog 	();

  const char *getCommand ();
  Signal *getSignal();

  public slots:

  void getFrequency();

 protected:

  void resizeEvent (QResizeEvent *);
  
 private:

  CurveWidget       *pulse;
  ScaleWidget       *x,*y;
  CornerPatchWidget *corner;

  TimeLine      *pulselength;
  QLabel        *pulselabel;
  QLabel        *channel;

  QPushButton   *freqbutton;
  QPushButton	*ok,*cancel;
  int rate;     //sampling rate
  Curve         *times;
};
