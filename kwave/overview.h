#ifndef _KWAVEOVERVIEW_H_
#define _KWAVEOVERVIEW_H_ 1

#include <qpushbt.h>
#include <qwidget.h>
#include <qcombo.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qframe.h>

#include "slider.h"

#include <kselect.h>
#include <ktopwidget.h>
#include <kbuttonbox.h>
#include <kbutton.h>

class MainWidget;
//***********************************************************
//OverviewWidget is the scrollbar in the main window
//QScrollbar has proven to be unstable with high numbers.
//this one also features a small overview of the part of the sample being
//unseen
class OverViewWidget : public QWidget
{
 Q_OBJECT
 public:
      OverViewWidget	(QWidget *parent=0,const char *name=0);
      OverViewWidget	(MainWidget *parent=0,const char *name=0);
      ~OverViewWidget	();
 void mousePressEvent	      (QMouseEvent *);
 void mouseReleaseEvent	      (QMouseEvent *);  
 void mouseMoveEvent	      (QMouseEvent *);  
 void setSignal		      (char *);
 void setValue                (int);
 void refresh                 ();

 public slots:

 void setRange		(int,int,int);
 void increase ();

 signals:

 void valueChanged (int);

 protected:

 void paintEvent(QPaintEvent *);

 private:

 int        width,height;
 int        grabbed;
 int        max;
 int        len;
 int        act;
 int        dir;        //addup for direction...
 int	    redraw;	//flag for redrawing pixmap
 MainWidget *mparent;
 QWidget    *parent;
 QTimer     *timer;     //to spare user repeated pressing of the widget...
 QPixmap    *pixmap;	//pixmap to be blitted to screen
};
#endif //_KWAVEOVERVIEW_H_
