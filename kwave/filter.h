#include <kmsgbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qdir.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qscrbar.h>
#include <qbutton.h>
#include <qcombo.h>
#include <qdialog.h>
#include <qbttngrp.h>
#include <qslider.h>
#include <qchkbox.h>
#include <qradiobt.h>

#include "gsl_fft.h"
#include "fftview.h"
#include "curvewidget.h"
#include <krestrictedline.h>
#include <kintegerline.h>

//*****************************************************************************
class Filter
{
 public:
  Filter ();
  ~Filter ();
  int  resize (int);
  void save   (QString *);
  void load   (QString *);

  int rate;
  int num;                //number of taps
  int fir;                //boolean if filter is fir or iir
  double *mult;           //array of coefficients, used according to
  int *offset;            //array of delay times in samples
};
//*****************************************************************************
class FilterDialog : public QDialog
{
 Q_OBJECT

 public:
 	FilterDialog 	(QWidget *parent=0,int =48000);
 	~FilterDialog 	();
 void	refreshView ();

 public slots:

 void setTaps      (const char *);
 void setOffset    (const char *);
 void setMult      (int);
 void refresh      ();
 void loadFilter   ();
 void saveFilter   ();

 struct Filter *getFilter();

 protected:

 void resizeEvent (QResizeEvent *);
 void getNTaps         (int);
 
 private:

 int w,h;
 Filter filter;

 int          oldnum;
 QLabel       *taplabel;
 KIntegerLine *taps;

 QLabel       **label;
 QSlider      **mult;
 KIntegerLine **offset;

 QPushButton	*load,*save;

 QLabel		*iirlabel,firlabel;
 QButtonGroup	*bg;
 QRadioButton	*fir,*iir;
 
 QPushButton	*ok,*cancel;
 QPushButton	*dofilter;
 FFTWidget	*filterwidget;
 FFTWidget	*phasewidget;
};
//*****************************************************************************
class MovingFilterDialog : public QDialog
{
  Q_OBJECT

    public:
  MovingFilterDialog 	(QWidget *parent=0,int num=1);
  ~MovingFilterDialog 	();
  QList<CPoint> *getPoints ();
  int   getType ();
  int   getState ();
  int   getTap();
  int   getLow();  //for getting lower bound and
  int   getHigh(); //higher bound
  public slots:

    void toggleState();
    void checkTap(const char *);

 protected:

  void resizeEvent (QResizeEvent *);

 private:

  int num;
  QCheckBox     *usecurve;
  KRestrictedLine  *low,*high;
  KIntegerLine  *tap;
  QLabel        *lowlabel,*highlabel;
  QPushButton	*ok,*cancel;
  CurveWidget	*curve;
};

