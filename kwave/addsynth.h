#ifndef _ADDSYNTH_H_
#define _ADDSYNTH_H_ 1

#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include "curvewidget.h"
#include "dialogs.h"
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>
#include <kintegerline.h>
#include "dialogs.h"
#include "curvewidget.h"
#include "scale.h"

class MSignal;
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
 QPainter 	p;
};
//***********************************************************************
class AddSynthDialog : public QDialog
{
  Q_OBJECT

    public:

  AddSynthDialog 	(QWidget *parent=0,int rate=48000,int time=0,char *name="Choose Signal Components :");
  ~AddSynthDialog 	();

  MSignal *getSignal();

  public slots:

  void setChannels (const char *);
  void getFrequency();
  void showPower (const char *);
  void showPower (int);
  void showPhase (const char *);
  void showPhase (int);
  void showMult  (const char *);
  void showMult  (int);

 protected:
  void mousePressEvent( QMouseEvent *e);      
  void updateView ();
  void getNSlider (int,int);
  void resizeEvent (QResizeEvent *);
  int  getCount();        //get number of partials
  
 private:

  QList<CPoint> *times;
  int tflag; //flag if typing in integerline
  int num;   //number of sines
  int rate;  //sampling rate
  int *apower,*aphase,*amult;

  AddSynthWidget    *view;
  ScaleWidget       *x,*y;
  CornerPatchWidget *corner;

  KIntegerLine	**mult;
  KIntegerLine	**powerlabel;
  KIntegerLine  **phaselabel;

  QSlider	**power;
  QSlider	**phase;
  
  KIntegerLine	*channel;
  QLabel        *channellabel;

  QPushButton   *freqbutton;
  QPushButton   *hear;
  MSignal       *test;           //for hearing...

  QComboBox	*functype;
  QPopupMenu*   menu;

  QLabel	*phaselab;
  QLabel	*powerlab;
  QLabel	*multlab;
  QPushButton	*ok,*cancel;
};
//*****************************************************************************
class PulseDialog : public QDialog
{
  Q_OBJECT

    public:

  PulseDialog 	(QWidget *parent=0,int rate=48000,int time=0,char *name="Choose pulse properties :");
  ~PulseDialog 	();

  MSignal *getSignal();

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
  MSignal       *test;           //for hearing...


  QPushButton	*ok,*cancel;
  int rate;     //sampling rate
  QList<CPoint> *times;
};
#endif
