#ifndef _PITCHWIDGET_H_
#define _PITCHWIDGET_H_ 1

#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "scale.h"

#include <kapp.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  

class PitchWidget : public QWidget
{
 Q_OBJECT
 public:
 	PitchWidget	(QWidget *parent=0);
 	~PitchWidget	();
 void	mousePressEvent		(QMouseEvent * );
 void	mouseReleaseEvent	(QMouseEvent * );  
 void	mouseMoveEvent		(QMouseEvent * );  
 void 	setSignal		(float *,int);
 void   refresh                 ();

 public slots:

 signals:
 
 void freqRange  (float,float);
 void pitch      (float);
 void timeSamples(float);

 protected:

 void   getMaxMin               ();
 void	paintEvent              (QPaintEvent *);

 private:

 float   *data;
 float   max,min;
 int     len;
 int	 width,height;	 //of widget
 bool    redraw;

 QPixmap  *pixmap;	 //pixmap to be blitted to screen
};
//***********************************************************************
class PitchContainer : public QWidget
{
 Q_OBJECT
 public:
 	PitchContainer	(QWidget *parent);
 	~PitchContainer	();
	void 	setObjects	(PitchWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner);

 public slots:

 signals:

 protected:

 void	resizeEvent	(QResizeEvent *);

 private:
 PitchWidget       *view;
 ScaleWidget       *xscale,*yscale;
 CornerPatchWidget *corner;
};
//***********************************************************************
class PitchWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
 	PitchWindow	(QString *name);
 	~PitchWindow	();
 void 	setSignal	(float *,int,int);

 public slots:

 void freqRange (float,float);
 void showPitch (float);
 void showTime  (float);

 signals:

 protected:

 private:
 PitchWidget     *view;
 PitchContainer  *mainwidget;
 ScaleWidget     *xscale,*yscale;
 KStatusBar      *status;
 CornerPatchWidget *corner;
 int rate;
};
#endif






