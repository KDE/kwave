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
void ScaleWidget::setMaxMin (int b,int a)
{
  low=a;
  high=b;
  repaint (true);
}
//**********************************************************
void ScaleWidget::paintEvent  (QPaintEvent *)
{
  int h=height();
  int w=width();
  QPainter p;
  QFont font (this->font());
  font.setPointSize (8);
  p.begin (this);
  p.setFont (font);

  if (w>h)
    {
      p.setPen (colorGroup().light());
      p.drawLine (0,0,w,0);
      p.setPen (colorGroup().dark());
      p.drawLine (0,h-1,w,h-1);
      p.drawLine (w-1,0,w-1,h-1);

      p.setPen (colorGroup().text());

      int a,x;
      double ofs;
      double t=w-1;
      int h2=h;
      
      while ((t/10>1)&&(h2>0))
      {
	for (ofs=0;ofs<w-1;ofs+=t)
	  {
	    for (a=0;a<6;a++)
	      {
		x=(int)(ofs+(t*a/5));
		p.drawLine (x,1,x,h2-2);
	      }
	  }

	h2=h2>>1;
	t/=5;
      }

      for (a=0;a<5;a++)
	{
	  char buf[16];
	  sprintf (buf,"%d %s",low+((high-low)*a/5),unittext);
	  x=(w-1)*a/5;
	  p.drawText (x+2,h-2,buf);
	}
    }
  else
    {
      p.setPen (colorGroup().light());
      p.drawLine (0,0,w,0);
      p.drawLine (0,0,0,h);
      p.setPen (colorGroup().dark());
      p.drawLine (w-1,0,w-1,h-1);
      p.setPen (colorGroup().text());

      int a,y;
      double ofs;
      double t=h-1;
      int h2=w-1;
      
      while ((t/10>1)&&(h2>0))
      {
	for (ofs=0;ofs<h-1;ofs+=t)
	  {
	    for (a=0;a<6;a++)
	      {
		y=(int)(ofs+(t*a/5));
		p.drawLine (w-h2,y,w-1,y);
	      }
	  }
	h2=h2>>1;
	t/=5;
      }
      int fh=font.pointSize()+1;
      for (a=0;a<5;a++)
	{
	  char buf[16];
	  sprintf (buf,"%d%s",low+((high-low)*a/5),unittext);
	  y=(h-1)*a/5;
	  for (unsigned int i=0;i<strlen(buf);i++)
	    if ((int)(8+i*fh)<(h)/5)
	      p.drawText (2,y+2+i*fh,(w-2)/2,fh,AlignHCenter,&buf[i],1);
	}
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


