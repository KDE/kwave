#ifndef _CURVE_WIDGET_H_
#define _CURVE_WIDGET_H_ 1

#include <qwidget.h>
#include <qpainter.h>

class Curve;
class QMouseEvent;
class QPaintEvent;
class QPopupMenu;
class Point;
class QPixmap;
class QDir;
class QStrList;

class CurveWidget : public QWidget
{
  Q_OBJECT

 public:

  CurveWidget	(QWidget *parent=0,const char *init=0,int=false);
  ~CurveWidget	 ();

  inline const char* getCommand ();
         void        setCurve   (const char *);
         void        addPoint   (double,double);
         Point*      findPoint  (int,int);

 public slots:

  void  setType (int);
  void  scaleFit ();
  void  VFlip ();
  void  HFlip ();
  void  deleteLast   ();
  void  deleteSecond ();
  void  firstHalf    ();
  void  secondHalf   ();
  void  savePreset   ();
  void  loadPreset   (int);

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

  Curve         *points; 	//Points set by User
  QPopupMenu    *menu;
  Point         *act;
  Point         *last;	        //last Point clicked remembered for deleting
  QPainter 	p;
  QPixmap	*pixmap;	//pixmap to be blitted to screen
  QDir          *presetDir;     //directory for Presets
  QStrList      *namelist;
};
//***********************************************************************
#endif // _CURVE_WIDGET_H_
