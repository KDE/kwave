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
 	FFTWidget	(QWidget *parent=0);
 	~FFTWidget	();
 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void 	setSignal		(complex *data,int size,int rate);
 void 	setPhase		(complex *data,int size,int rate);
 void   refresh                 ();
 void   setAutoDelete           (int);
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

 signals:

 void   freqInfo  (int,int);
 void   phaseInfo (int,int);
 void   ampInfo   (int,int);
 void   dbInfo    (int,int);

 protected:

 void	paintEvent              (QPaintEvent *);
 void	drawInterpolatedFFT	();
 void	drawOverviewFFT	        ();
 void	drawInterpolatedDB	();
 void	drawOverviewDB          ();
 void	drawInterpolatedPhase	();
 void	drawOverviewPhase	();

 private:
 double  max;
 double  min;

 int     autodelete;     //flag if deleting data is correct
 complex *data;
 int	 fftsize,rate;
 int	 width,height;	 //of widget
 double	 zoom;		 //number of samples represented by 1
			 //vertical line on the screen
 int     db;             //flag, if decibel scale is to be used
                         //if !false, range of scale in db
 bool    phaseview;      //flag for displaying phase instead of power spectrum
 bool	 redraw;	 //flag for redrawing pixmap

 QPainter p;
 QPixmap  *pixmap;	 //pixmap to be blitted to screen
};
//***********************************************************************
class FFTContainer : public QWidget
{
 Q_OBJECT
 public:
 	FFTContainer	(QWidget *parent);
 	~FFTContainer	();
 void 	setObjects	(FFTWidget *fftview,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner);

 public slots:

 signals:

 protected:

 void	resizeEvent	(QResizeEvent *);

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
 	FFTWindow	(QString *name);
 	~FFTWindow	();
 void 	setSignal	(complex *,double,int,int);	//reaches through to class FFTWidget...

 public slots:

 void	setFreqInfo  (int,int);
 void	setAmpInfo   (int,int);
 void	setDBInfo    (int,int);
 void	setPhaseInfo (int,int);
 void   phaseMode    ();
 void   dbMode       (int);
 void   percentMode  ();

 signals:

 protected:

 private:
 FFTContainer  *mainwidget;
 FFTWidget     *fftview;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 KStatusBar    *status;
};
#endif






