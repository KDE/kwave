#ifndef _CURVE_H_
#define _CURVE_H_ 1

#include <qdir.h>
#include <qapp.h>
#include <qpushbt.h>
#include <qstring.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <kapp.h>
#include <kslider.h>
#include <kselect.h>
#include <ktopwidget.h>
#include <kmenubar.h>
#include <kbuttonbox.h>
#include <kstatusbar.h>  
#include "gsl_fft.h"

struct CPoint
{
 double x,y;
};

class CurveWidget : public QWidget
{
  Q_OBJECT

    public:

  CurveWidget	(QWidget *parent=0,const char *name=0,QList<CPoint>*d=0,int=false);
  ~CurveWidget	();
  QList<CPoint> *getPoints ();
  int getType ();

  public slots:

  void  setType (int);
  void  scaleFit ();
  void  VFlip ();
  void  HFlip ();
  void  deleteLast ();
  void  savePreset ();
  void  loadPreset (int);

 signals:

 protected:

  void	mousePressEvent		(QMouseEvent * );
  void	mouseReleaseEvent	(QMouseEvent * );  
  void	mouseMoveEvent		(QMouseEvent * );
  void	paintEvent              (QPaintEvent *);
 
 private:
  
  int	width,height;		//of widget
  int   interpolationtype;      //type of interpolation
  int   keepborder;             //flag denying acces to first and last point...e
  double x[7],y[7];		//buffer for polynomial coefficients

  QList<CPoint> *points; 	//Points set by User
  QPopupMenu    *menu;
  CPoint        *act;
  CPoint *last;	                //last Point clicked remembered for deleting
  QPainter 	p;	
  QPixmap	*pixmap;	//pixmap to be blitted to screen
  QDir          *presetDir;     //directory for Presets
  QStrList      *namelist;
};
//***********************************************************************
#endif //curvewidget.h
