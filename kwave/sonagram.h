#ifndef _SONAGRAM_H_
#define _SONAGRAM_H_ 1

#include "classes.h"
#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <kapp.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kstatusbar.h>  
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>
#include "gsl_fft.h"
#include "scale.h"
//***********************************************************************
class ImageView : public QWidget
{
 Q_OBJECT
 public:
 	ImageView	(QWidget *parent=0);
 	~ImageView	();
 void	mouseMoveEvent	(QMouseEvent * );
 void 	setImage	(QImage *image);
 int    getWidth        ();
 int    getOffset       ();

 public slots:

 void 	setOffset		(int);

 signals:

 void   viewInfo(int,int,int);
 void   info (double,double);

 protected:

 void	paintEvent(QPaintEvent *);

 private:
 int            height,width;
 int            offset;
 int            lh,lw;
 QImage         *image;
 QPixmap        map;
 QWidget	*parent;
};
//***********************************************************************
class SonagramContainer : public QWidget
{
 Q_OBJECT
 public:
 	SonagramContainer	(QWidget *parent);
 	~SonagramContainer	();
 void 	setObjects	(ImageView *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner,OverViewWidget *overview=0);

 public slots:

 signals:

 protected:

 void	resizeEvent	(QResizeEvent *);

 private:
 ImageView     *view;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 OverViewWidget *overview;
};
//***********************************************************************
class SonagramWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
 	SonagramWindow	();
 	~SonagramWindow	();
 void 	setSignal	(double*,int,int,int);

 public slots:

 void	save         ();
 void	load         ();
 void	toSignal     ();
 void	setInfo      (double,double);
 void   setRange     (int,int,int);
 signals:

 protected:
 void   createImage ();
 void   createPalette ();
// void	resizeEvent	(QResizeEvent *);

 private:
 KStatusBar *status;
 QImage     *image;
 ImageView  *view;
 OverViewWidget *overview;
 SonagramContainer *mainwidget;
 ScaleWidget   *xscale,*yscale;
 CornerPatchWidget *corner;
 int        points,rate,length;
 int        x,y;
 complex    **data;
 double     max;
};
#endif //sonagram.h
