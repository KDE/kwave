#include <stdio.h>
#include "scale.h"

//**********************************************************
ScaleWidget::ScaleWidget (QWidget *parent,int low,int high,char *text): QWidget (parent)
{
  this->low=low;
  this->high=high;
  this->unittext=text;
}
//**********************************************************
ScaleWidget::~ScaleWidget ()
{
}
//**********************************************************
void ScaleWidget::paintEvent  (QPaintEvent *)
{
  int h=height();
  int w=width();
  QPainter p;
  p.begin (this);

  if (w>h)
    {
      p.setPen (colorGroup().light());
      p.drawLine (0,0,w,0);
      p.setPen (colorGroup().dark());
      p.drawLine (0,h-1,w,h-1);
      p.drawLine (w-1,0,w-1,h-1);
    }
  else
    {
      p.setPen (colorGroup().light());
      p.drawLine (0,0,w,0);
      p.drawLine (0,0,0,h);
      p.setPen (colorGroup().dark());
      p.drawLine (w-1,0,w-1,h-1);
    }
  p.end ();
}
//**********************************************************
CornerPatchWidget::CornerPatchWidget (QWidget *parent,int pos): QWidget (parent)
{
  this->pos=pos;
}
//**********************************************************
CornerPatchWidget::~CornerPatchWidget ()
{
}
//**********************************************************
void CornerPatchWidget::paintEvent  (QPaintEvent *)
{
  int h=height();
  int w=width();
  QPainter p;
  p.begin (this);

  p.setPen (colorGroup().light());
  p.drawLine (0,0,0,h-1);
  p.setPen (colorGroup().dark());
  p.drawLine (0,h-1,w,h-1);
  p.end ();
}


