#ifndef _PITCHWIDGET_H_
#define _PITCHWIDGET_H_ 1

#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "../libgui/ScaleWidget.h"

#include <kapp.h>
#include <kselect.h>
#include <kmenubar.h>
#include <kbuttonbox.h>

//***********************************************************************
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
#endif






