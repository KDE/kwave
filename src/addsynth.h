#ifndef _ADDSYNTH_H_
#define _ADDSYNTH_H_ 1

#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>

#include "../libgui/curvewidget.h"
#include "../libgui/curvewidget.h"
#include "../libgui/scale.h"
#include "../libgui/slider.h"

#include <kintegerline.h>

class KwaveSignal;
class TimeLine;

class AddSynthWidget : public QWidget
{
 Q_OBJECT
 public:
 	AddSynthWidget	(QWidget *parent=0);
 	~AddSynthWidget	();
 void   setSines (int,int *,int*,int *);

 public slots:

 void   setFunction (int);

 signals:

 protected:

 void	paintEvent(QPaintEvent *);

 private:

 int func;
 int *power;
 int *phase;
 int *mult;
 int count;
};
//***********************************************************************
class AddSynthDialog : public QDialog
{
  Q_OBJECT

    public:

  AddSynthDialog 	(QWidget *parent=0,int rate=48000,int time=0,char *name="Choose Signal Components :");
  ~AddSynthDialog 	();

  KwaveSignal *getSignal();

  public slots:

  void setChannels (const char *);
  void getFrequency();
  void showPower (const char *);
  void showPower (int);
  void showPhase (const char *);
  void showPhase (int);
  void showMult  (const char *);
  void showMult  (int);
  void popMenu   ();
  void oddMult   ();
  void evenMult  ();
  void primeMult ();
  void zeroPhase ();
  void sinusPhase ();
  void negatePhase ();
  void enumerateMult ();
  void randomizePower ();
  void randomizePhase ();
  void fibonacciMult ();
  void invertPower ();
  void sinusPower ();
  void maxPower ();
  void dbPower ();
 protected:
  void mousePressEvent( QMouseEvent *e);      
  void updateView ();
  void getNSlider (int,int);
  void resizeEvent (QResizeEvent *);
  int  getCount();        //get number of partials
  
 private:

  Curve             *times;

  AddSynthWidget    *view;
  ScaleWidget       *x,*y;
  CornerPatchWidget *corner;

  KIntegerLine	**mult;
  KIntegerLine	**powerlabel;
  KIntegerLine  **phaselabel;

  KwaveSlider	**power;
  KwaveSlider	**phase;
  
  KIntegerLine	*channel;
  QLabel        *channellabel;

  QPushButton   *freqbutton;
  QPushButton   *calculate;
  KwaveSignal   *test;           //for hearing not yet done...

  QComboBox	*functype;
  QPopupMenu*   menu;

  QLabel	*phaselab;
  QLabel	*powerlab;
  QLabel	*multlab;
  QPushButton	*ok,*cancel;

  int num;    //number of sines
  int rate;   //sampling rate
  int *apower,*aphase,*amult;
  bool tflag; //flag if typing in integerline
};
//*****************************************************************************
class PulseDialog : public QDialog
{
  Q_OBJECT

    public:

  PulseDialog 	(QWidget *parent=0,int rate=48000,int time=0,char *name="Choose pulse properties :");
  ~PulseDialog 	();

  KwaveSignal *getSignal();

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
  QPushButton   *hear;
  KwaveSignal   *test;           //for hearing...


  QPushButton	*ok,*cancel;
  int rate;     //sampling rate
  Curve         *times;
};
#endif
