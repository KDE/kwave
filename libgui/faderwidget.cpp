#include <qkeycode.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include "faderwidget.h"
#include "fftview.h"

extern const char *OK;
extern const char *CANCEL;
//****************************************************************************
FaderWidget::FaderWidget (QWidget *parent,int dir) : QWidget
(parent)
{
  this->dir=dir;
  height=-1;
  curve=0;
  this->setBackgroundColor (QColor(black));    
}
//****************************************************************************
FaderWidget::~FaderWidget (QWidget *parent,const char *name)
{
}
//****************************************************************************
void FaderWidget::setCurve (int c)
{
  curve=c*c*c;
  repaint ();
}
//****************************************************************************
int FaderWidget::getCurve ()
{
  return curve;
}
//****************************************************************************
void FaderWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  height=rect().height();
  width=rect().width();

  p.begin (this);
  p.setPen (white);

  int lx=(dir==1)?0:width;
  int ly=height;

  if (curve==0)
    {
      for (int i=1;i<width;i++)
	{
	  if (dir==1)
	    {
	      p.drawLine (lx,ly,i,(width-i)*height/width);
	      lx=i;
	    }
	  else
	    {
	      p.drawLine (lx,ly,width-i,(width-i)*height/width);
	      lx=width-i;
	    }

	  ly=(width-i)*height/width;
	}
    }
  else
    if (curve<0)
      for (int i=1;i<width;i++)
	{
	  int y=height-(int)(log10(1+(-curve*((double)i)/width))*height/log10(1-curve));
	  if (dir==1)
	    {
	      p.drawLine (lx,ly,i,y);
	      lx=i;
	    }
	  else
	    {
	      p.drawLine (lx,ly,width-i,y);
	      lx=width-i;
	    }

	  ly=y;
	}
    else
      for (int i=1;i<width;i++)
	{
	  int y=(int)(log10(1+(curve*((double)(width-i))/width))*height/log10(1+curve));

	  if (dir==1)
	    {
	      p.drawLine (lx,ly,i,y);
	      lx=i;
	    }
	  else
	    {
	      p.drawLine (lx,ly,width-i,y);
	  
	      lx=width-i;
	    }

	  ly=y;
	}

  p.end();
}
//****************************************************************************
FadeDialog::FadeDialog (QWidget *par,int dir,int ms): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Choose Curve :"));

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);
  slider =new KwaveSlider (-100,100,1,0,KwaveSlider::Horizontal,this);     
  fade   =new FaderWidget (this,dir);

  x=new ScaleWidget (this,0,ms,"ms");
  y=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(slider	,SIGNAL(valueChanged(int)),fade,SLOT (setCurve(int)));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int FadeDialog::getCurve ()
{
  return (fade->getCurve());
}
//**********************************************************
void FadeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int half=height()-bsize*7/2;

 fade   ->setGeometry	(8+bsize,0,width()-16-bsize,half-bsize);  
 y      ->setGeometry	(8,0,bsize,half-bsize);
 x      ->setGeometry	(8+bsize,half-bsize,width()-16,bsize);
 corner ->setGeometry   (8,half-bsize,bsize,bsize);

 slider->setGeometry	(width()/20,height()-bsize*3,width()*18/20,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FadeDialog::~FadeDialog ()
{
}
