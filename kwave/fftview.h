#ifndef _FFTVIEW_H_
#define _FFTVIEW_H_ 1

#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  
#include <qpixmap.h>
#include <qtimer.h>
#include "scale.h"
#include "gsl_fft.h"

class FFTWidget : public QWidget
{
 Q_OBJECT
 public:
	FFTWidget       (QWidget *parent=0);
	~FFTWidget      ();
 void   mousePressEvent         (QMouseEvent * );
 void   mouseReleaseEvent       (QMouseEvent * );  
 void   mouseMoveEvent          (QMouseEvent * );  
 void   setSignal               (complex *data,int size,int rate);
 void   setPhase                (complex *data,int size,int rate);
 void   refresh                 ();
 void   setAutoDelete           (int);
 void   setFreqRange            (int,int);
 void   getMaxMin               ();

 public slots:

 void   iFFT         ();
 void   amplify      ();
 void   formant      ();
 void   smooth       ();
 void   killPhase    ();
 void   phaseMode    ();
 void   dbMode       (int);
 void   percentMode  ();
 void   togglefindPeak (bool *);
 void   findMaxPeak  ();
 void   findMinimum  ();

 signals:

 void   freqInfo  (int,int);
 void   phaseInfo (int,int);
 void   ampInfo   (int,int);
 void   dbInfo    (int,int);
 void   noteInfo  (int,int);

 protected:

 void   paintEvent              (QPaintEvent *);
 void   drawInterpolatedFFT     ();
 void   drawOverviewFFT         ();
 void   drawInterpolatedDB      ();
 void   drawOverviewDB          ();
 void   drawInterpolatedPhase   ();
 void   drawOverviewPhase       ();

 private:
 complex *data;
 int     fftsize,rate;
 int     width,height;   //of widget
 double  max;           
 double  min;
 double  zoom;           //number of samples represented by 1
			 //vertical line on the screen
 int     lmarker,rmarker;
 int     oldcursor;      //position of cursor;
 int     cursor;         //position of cursor;
 int     db;             //flag, if decibel scale is to be used
			 //if !false, range of scale in db
 bool    findLocalMax;   //if true --> show frequency and note of nearest maximum
 bool    phaseview;      //flag for displaying phase instead of power spectrum
 bool    redraw;         //flag for redrawing pixmap
 bool    redrawcursor;   //flag for fast redrawing of cursor
 bool    autodelete;     //flag if deleting data is allowed

 QPainter p;
 QPixmap  *pixmap;       //pixmap to be blitted to screen
};
//***********************************************************************
class FFTContainer : public QWidget
{
 Q_OBJECT
 public:
	FFTContainer    (QWidget *parent);
	~FFTContainer   ();
 void   setObjects      (FFTWidget *fftview,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner);

 public slots:

 signals:

 protected:

 void   resizeEvent     (QResizeEvent *);

 private:
 FFTWidget     *view;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
};
//***********************************************************************
class FFTWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
	FFTWindow       (QString *name);
	~FFTWindow      ();
 void   setSignal       (complex *,double,int,int);     //reaches through to class FFTWidget...

 public slots:

 void   setFreqInfo  (int,int);
 void   setAmpInfo   (int,int);
 void   setDBInfo    (int,int);
 void   setPhaseInfo (int,int);
 void   setNoteInfo  (int,int);
 void   phaseMode    ();
 void   dbMode       (int);
 void   percentMode  ();
 void   askFreqRange ();
 void   findPeak     ();

 signals:

 protected:

 private:
 FFTContainer  *mainwidget;
 FFTWidget     *fftview;
 QPopupMenu    *cursor;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 KStatusBar    *status;
 int           findPeakID;
};
#endif






