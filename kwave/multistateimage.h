#ifndef _MULTISTATEIMAGE_H_
#define _MULTISTATEIMAGE_H_ 1

#include <stdlib.h>
#include <qapp.h>
#include <qwidget.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <qmlined.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>

class MultiStateWidget : public QWidget
{
 Q_OBJECT

 public:

 	MultiStateWidget 	(QWidget *parent=NULL,int=0,int=2);
 	~MultiStateWidget 	();
 int	addPixmap               (char *);
 void   setStates               (int *newstates);
 void   setState                (int newstate);
 void   nextState                ();
 signals:

 void clicked	(int);

 public slots:

 protected:

 void	mouseReleaseEvent		(QMouseEvent * );
 void   paintEvent(QPaintEvent *); 

 private:

 QPainter p;
 int *states;  // maps states to pixmap-list
 int act;      // current states
 int count;    // number of states
 int number;   // number of channels this object represents... used for signals
};

#endif  /* multistateimage.h */   


