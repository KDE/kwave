#ifndef _KWAVEMOUSEMARK_H_
#define _KWAVEMOUSEMARK_H_ 1
#include <qobject.h>
class QWidget;
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
 inline int  getLeft  () {return (initial<last)?initial:last;};
 inline int  getRight () {return (last>initial)?last:initial;};

 public slots:
 void setZoom         (double);
 void setOffset       (int);
 void setLength       (int);

 signals:
 void refresh         ();
 void selection       (int,int);      //sends the current selection for updating of gui elements

 private:
 int	 initial;       //initial position of mouse
 int     last;          //last known position
 int     offset;        //display offset in signal
 int     length;        //length of signal in samples
 double  zoom;          //current zoom
};
//***********************************************************
#endif //_KWAVEMOUSEMARK_H_
