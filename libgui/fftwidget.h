#ifndef _FFT_WIDGET_H_
#define _FFT_WIDGET_H_ 1

#include "../lib/gsl_fft.h"
#include <qwidget.h>
#include <qpainter.h>

//***********************************************************************
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
#endif
