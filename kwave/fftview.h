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
#include "gsl_fft.h"

class FFTWidget : public QWidget
{
 Q_OBJECT
 public:
 	FFTWidget	(QWidget *parent=0,const char *name=0);
 	~FFTWidget	();
 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void 	setSignal		(complex *data,int size,int rate);

 public slots:

 signals:

 void   freqInfo (int,int);
 void   phaseInfo (int,int);
 void   ampInfo (int,int);

 protected:

 void	paintEvent(QPaintEvent *);
 void	drawFFT ();
 void	drawInterpolatedFFT	();
 void	drawOverviewFFT	();

 private:
 complex *data;
 int	fftsize,rate;
 int	width,height;		//of widget
 double	zoom;			//number of samples represented by 1
				//vertical line on the screen
 QPainter 	p;
 int		redraw;		//flag for redrawing pixmap
 QPixmap	*pixmap;	//pixmap to be blitted to screen
};
//***********************************************************************
class FFTWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
 	FFTWindow	(const char *name=0);
 	~FFTWindow	();
 void 	setSignal	(complex *,int,int);	//reaches through to class FFTWidget...

 public slots:

 void	setFreqInfo (int,int);
 void	setAmpInfo (int,int);
 void	setPhaseInfo (int,int);

 signals:

 protected:

// void	resizeEvent	(QResizeEvent *);

 private:
 FFTWidget *fftview;
 KStatusBar *status;
};
