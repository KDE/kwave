#ifndef _MULTISTATEIMAGE_H_
#define _MULTISTATEIMAGE_H_ 1

#include <qwidget.h>
#include <qpainter.h>

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
