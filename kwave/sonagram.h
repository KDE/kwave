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
#include <qimage.h>
#include <qtimer.h>
#include "gsl_fft.h"

//***********************************************************************
class ImageView : public QWidget
{
 Q_OBJECT
 public:
 	ImageView	(QWidget *parent=0);
 	~ImageView	();
 void	mouseMoveEvent		(QMouseEvent * );
 void 	setImage		(QImage *image);

 public slots:

 signals:

 void   info (double,double);

 protected:

 void	paintEvent(QPaintEvent *);

 private:
 int            x,y;
 QImage         *image;
 QWidget	*parent;
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

 signals:

 protected:
 void   createImage ();
 void   createPalette ();
// void	resizeEvent	(QResizeEvent *);

 private:
 KStatusBar *status;
 QImage     *image;
 ImageView  *view;
 int        points,rate,length;
 int        x,y;
 complex    **data;
 double     max;

};

