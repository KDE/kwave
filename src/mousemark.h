#ifndef _KWAVEMOUSEMARK_H_
#define _KWAVEMOUSEMARK_H_ 1

#include <qwidget.h>

class MouseMark:public QObject
{
 Q_OBJECT
 public:
 MouseMark::MouseMark (QWidget *);
 MouseMark::~MouseMark();
 bool isSelected      ();             //returns wether a selection was made
 void unselect        ();             //sets state to unselected
 void set             (int,int);      //sets the selection
 void update          (int);          //for continous update of mouse movement
 void grep            (int);          //regetting a already existing selection
 bool checkPosition   (int,int);      //checks for bound to reget the selection
 void drawSelection   (QPainter *,int,int);

 public slots:
 void setZoom         (double);
 void setOffset       (int);
 void setLength       (int);

 signals:
 void refresh         ();
 void selection       (int,int);

 private:
 int	 initial;       //initial position of mouse
 int     last;          //last known position
 int     offset;
 int     length;
 double  zoom;
};
//***********************************************************
#endif //_KWAVEMOUSEMARK_H_
