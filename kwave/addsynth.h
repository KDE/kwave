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

class MSignal;

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
  void fileMode();
  void fixedMode();
  void sweepMode();
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
  int getCount();        //get number of partials
  
 private:

  QList<CPoint> *times;
  int tflag; //flag if typing in integerline
  int num;   //number of sines
  int rate;  //sampling rate
  int time;  //time in periods
  int freq;  //frequency of period in Hz

  AddSynthWidget *view;

  int            *apower,*aphase,*amult;

  KIntegerLine	**multlabel;
  KIntegerLine	**powerlabel;
  KIntegerLine  	**phaselabel;

  QSlider	**mult;
  QSlider	**power;
  QSlider	**phase;
  
  KIntegerLine	*channel;
  QLabel        *channellabel;

  QComboBox	*functype;
  QButtonGroup	*bg;
  QPopupMenu*   menu;
  QRadioButton  *fixed,*file,*sweep;

  QLabel	*phaselab;
  QLabel	*powerlab;
  QLabel	*multlab;
  QPushButton	*ok,*cancel;
};
//*****************************************************************************
#endif
