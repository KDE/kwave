#ifndef _SCALE_H_
#define _SCALE_H_ 1

#include <stdlib.h>
#include <qapp.h>
#include <qwidget.h>
#include <qpainter.h>

class ScaleWidget : public QWidget
{
 Q_OBJECT

 public:

 	ScaleWidget 	(QWidget *parent=0,int=0,int=100,char *unittext="%");
 	~ScaleWidget 	();

 signals:

 public slots:

 protected:

 void   paintEvent(QPaintEvent *); 

 private:

 int  low,high;    //range of display
 char *unittext;   //string containing the name of the unit
};
//*****************************************************
class CornerPatchWidget : public QWidget
{
 Q_OBJECT

 public:

 	CornerPatchWidget 	(QWidget *parent=0,int=0);
 	~CornerPatchWidget 	();

 signals:

 public slots:

 protected:

 void   paintEvent(QPaintEvent *); 

 private:

 int  pos;         //
};


#endif  /* Scale.h */   


