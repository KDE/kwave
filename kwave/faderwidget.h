#ifndef _FADERDIALOG_H_
#define _FADERDIALOG_H_ 1

#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qslider.h>
#include <qdialog.h>
#include "scale.h"

class FaderWidget : public QWidget
{
 Q_OBJECT
 public:
 	FaderWidget	(QWidget *parent=0,int dir=1);
 	~FaderWidget	();
	int getCurve ();
 public slots:

 void   setCurve (int);

 signals:

 protected:

 void	paintEvent(QPaintEvent *);
 void	drawInterpolatedFFT	();

 private:

 int    dir;
 int    curve;
 int	width,height;		//of widget
};
//***********************************************************************
class FadeDialog : public QDialog
{
 Q_OBJECT

 public:
 	FadeDialog 	(QWidget *parent=0,int dir=1,int ms=100);
 	~FadeDialog 	();
 int getCurve ();

 public slots:

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 ScaleWidget       *x,*y;
 CornerPatchWidget *corner;
 QPushButton	*ok,*cancel;
 QSlider	*slider;
 FaderWidget	*fade;

};
//*****************************************************************************
#endif
