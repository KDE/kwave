#ifndef _SONAGRAM_WINDOW_H_
#define _SONAGRAM_WINDOW_H_ 1

#include <qtimer.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>

#include "../libgui/OverViewWidget.h"
#include "../libgui/ScaleWidget.h"

#include <kapp.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kstatusbar.h>  

#include <libkwave/gsl_fft.h>
class ImageView;
class SonagramContainer;
//***********************************************************************
class SonagramWindow : public KTopLevelWidget
{
 Q_OBJECT
 public:
 	SonagramWindow	(QString *);
 	~SonagramWindow	();
 void 	setSignal	(double*,int,int,int,int);

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
