#include <qwidget.h>
#include "../src/MainWidget.h" //this has to be changed sometimes
#include "OverViewWidget.h"

OverViewWidget::OverViewWidget (QWidget *parent,const char *name)
  : QWidget (parent,name)
{
  pixmap=0;

  this->mparent=0;
  grabbed=false;
  timer=0;
}
//*****************************************************************************
OverViewWidget::OverViewWidget (MainWidget *parent,const char *name)
 : QWidget (parent,name)
{
  pixmap=0;
  this->parent=parent;
  this->mparent=parent;
  grabbed=false;
  timer=0;
}
//*****************************************************************************
OverViewWidget::~OverViewWidget ()
{
  if (pixmap) delete pixmap;
}
//*****************************************************************************
void OverViewWidget::mousePressEvent( QMouseEvent *e)
{
  int x1=(int)(((double)act)*width/len);
  int x2=(int)(((double)max)*width/len);
  if (x2<8) x2=8;
  if (e->x()>x1+x2)
    {
      dir=max/2;
      timer=new QTimer (this);
      connect (timer,SIGNAL(timeout()),this,SLOT(increase()));
      timer->start( 50); 
    }
  else
  if (e->x()<x1)
    {
      dir=-max/2;
      timer=new QTimer (this);
      connect (timer,SIGNAL(timeout()),this,SLOT(increase()));
      timer->start( 50); 
    }
  else grabbed=e->x()-x1;
}
//****************************************************************************
void OverViewWidget::increase()
{
  act+=dir;
  if (act<0) act=0;
  if (act>len-max) act=len-max;
  repaint (false);
  emit valueChanged (act);
}
//****************************************************************************
void OverViewWidget::mouseReleaseEvent( QMouseEvent *)
{
  grabbed=false;
  if (timer)
    {
      timer->stop();
      delete timer;
      timer=0;
    }
}
//****************************************************************************
void OverViewWidget::mouseMoveEvent( QMouseEvent *e)
{
  if (grabbed) 
    {
      int pos=e->x()-grabbed;
      if (pos <0) pos=0;
      if (pos>width) pos=width;
      act=(int)(((double) len)*pos/width);
      if (act>len-max) act=len-max;
      repaint (false);
      emit valueChanged (act);
    }
}
//*****************************************************************************
void OverViewWidget::refresh  ()
{
  redraw=true;
  repaint (false);
}
//*****************************************************************************
void OverViewWidget::setRange  (int newval,int x,int y)
{
  if ((newval!=act)||(len!=y)||(x!=max))
    {
      if ((len==y)&&(x==max))
	{
	  act=newval;
	  repaint (false);
	}
      else
	{
	  act=newval;
	  max=x;
	  len=y;
	  refresh ();
	}
    }
}
//*****************************************************************************
void OverViewWidget::setValue  (int newval)
{
  if (act!=newval)
    {
      act=newval;
      repaint (false);
    }
}
//*****************************************************************************
void OverViewWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width)||redraw)
    {
      redraw=false;
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());

      pixmap->fill (colorGroup().background());

      p.begin (pixmap);
      p.setPen (colorGroup().midlight());

      if (mparent)
	{
	  unsigned char *overview=mparent->getOverView(width);
	  if (overview)
	    {
	      for (int i=0;i<width;i++)
		p.drawLine (i,height-(((int)overview[i])*height)/128,i,height);
	      delete overview;
	    }
	}

      p.end ();
    }
  if (pixmap) bitBlt (this,0,0,pixmap);

  p.begin (this);

  int x1=(int)(((double)act)*width/len);
  int x2=(int)(((double)max)*width/len);
  if (x2<8) x2=8;   //so there is at least something

  p.setPen (colorGroup().light());
  p.drawLine (0,0,width,0);
  p.drawLine (0,0,0,height);

  p.drawLine (x1,0,x1+x2,0);
  p.drawLine (x1,0,x1,height);
  p.drawLine (x1+1,0,x1+1,height);
  p.setBrush (colorGroup().background());
  p.drawRect (x1,0,x2,height);
  p.setPen (colorGroup().dark());
  p.drawLine (1,height-1,width,height-1);
  p.drawLine (width-1,1,width-1,height-1);

  p.drawLine (x1+1,height-2,x1+x2,height-2);
  p.drawLine (x1+x2,1,x1+x2,height);
  p.drawLine (x1+x2-1,1,x1+x2-1,height);

  p.end ();
}
//*****************************************************************************
