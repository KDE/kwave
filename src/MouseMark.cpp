#include <stdio.h>
#include <stdlib.h>
#include <qpainter.h>
#include "MouseMark.h"

MouseMark::MouseMark (QWidget *parent):QObject ()
{
  QObject::connect (this,SIGNAL(refresh()),(QObject*) parent,SLOT(refresh()));
  initial=-1;
  last=-1;
  offset=0;
  zoom=1;
}
//****************************************************************************
void MouseMark::setZoom (double zoom)
{
  this->zoom=zoom;
}
//****************************************************************************
void MouseMark::setOffset (int offset)
{
  this->offset=offset;
}
//****************************************************************************
void MouseMark::setLength (int length)
{
  this->length=length;
}
//****************************************************************************
void MouseMark::set (int l,int r)
{
  initial=(int)(l*zoom+offset);
  last=(int)(r*zoom+offset);

  emit refresh ();

  if (initial<last) emit selection (initial,last);
  else emit selection (last,initial);
}
//****************************************************************************
void MouseMark::grep (int x)
{
  if (abs((int)(last-x*zoom-offset))>abs((int)(initial-x*zoom-offset)))
    initial=last;

  last=(int)(x*zoom+offset);
  
  emit refresh ();

  if (initial<last) emit selection (initial,last);
  else emit selection (last,initial);
}
//****************************************************************************
void MouseMark::update (int x)
{
  last=(int)(x*zoom+offset);
  emit refresh ();
  if (initial<last) emit selection (initial,last);
  else emit selection (last,initial);
}
//****************************************************************************
bool MouseMark::checkPosition (int x,int tol)
  //returns if x is in the border range between selected and unselected
{
  if (((offset+x*zoom)>initial-(tol*zoom))&&((offset+x*zoom)<initial+tol*zoom))
    return true;

  if ((offset+x*zoom<last+tol*zoom)&&((offset+x*zoom)>last-tol*zoom))
    return true;

  return false;
}
//****************************************************************************
void MouseMark::drawSelection (QPainter *p,int width, int height)
{
  int x=(int) (((double)(initial-offset))/zoom);
  int w=(int) (((double)(last-initial))/zoom);

  //clip output to window width
  if ((x<0)&&(w+x>0))
    {
      w+=x;
      x=0;
    }
  if (w+x>width) w=width-x;
  if (w+x<0) w=-x;

  p->setBrush (yellow);
  p->setRasterOp (XorROP);

  if (last==initial)
    {
      p->setPen  (green);
      p->drawLine(x,0,x,height);
      last=initial;
    }
  else
    {
      p->setPen  (yellow);
      p->drawRect(x,0,w,height);
//      debug("MouseMark::drawSelection:%d...%d (width=%d, last=%d, initial=%d)",
//      	x, x+w-1, width,last,initial); // ###
    }
  p->setRasterOp (CopyROP);
}
//****************************************************************************
MouseMark::~MouseMark ()
{
}
