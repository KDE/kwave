#include <qpainter.h>
#include <qkeycode.h>
#include <math.h>
#include "FormantWidget.h"
#include "FFTWidget.h"

//Formant modell according to G. Fant, acoustic theory of speech production,
//published by Moutom & Co,Netherlands, 1960, p. 48ff

//****************************************************************************
FormantWidget::FormantWidget (QWidget *parent,int rate) : QWidget
(parent)
{
  height=-1;
  this->setBackgroundColor (QColor(black));    
  this->rate=rate;
  num=0;
  pos=0;
  widths=0;
  points=0;
}
//****************************************************************************
FormantWidget::~FormantWidget ()
{
  if (pos) delete pos;
  if (widths) delete widths;
}
//****************************************************************************
double *FormantWidget::getPoints (int psize)
{
  double x=0;
  double freq; //input frequency
  double p1;
  double p2;
  double pos2;
  double q;
  int i;

  points=new double [psize];

  if (points)
    {
      for (i=0;i<psize;i++)
	{
	  freq=(double)(5000*i+.1)/psize;
	  x=freq/(330/4);      //carelessly assumed constant
	 
	  freq*=freq;
	  for (int j=0;j<num;j++) //add every formant
	    {
	      q=pos[j]/widths[j];
	      pos2=(pos[j]*pos[j]);

	      p1=1-(freq/pos2);
	      p1*=p1;
	      p2=freq/(pos2*q*q);
	      x+=10*log10(p1+p2);
	    }
	  points[i]=x/num;
	}
      //eliminate function error below 50 Hz
      int border=psize*50/5000;
      for (i=0;i<border;i++) points[i]=points[border];

    }
  return points;
}
//****************************************************************************
void FormantWidget::setFormants (int num, int *pos,int *widths)
{
  if (this->pos) delete this->pos;
  if (this->widths) delete this->widths;
  this->num=num;
  this->pos=pos;
  this->widths=widths;

  repaint ();
}
//****************************************************************************
void FormantWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  double *points;

  height=rect().height();
  width=rect().width();

  p.begin (this);
  p.setPen (white);

  if (num)
    {
      int i;
      double max=0;
      double min=10000;

      points=getPoints (width); //get function points

      if (points) 
	{
	  for (i=0;i<width;i++) //find max
	    {
	      if (max<points[i]) max=points[i];
	      if (min>points[i]) min=points[i];
	    }

	  //scale for display
	  for (i=0;i<width;i++) points[i]=(points[i]-min)/(max-min);

	  //set display of scale-range
	  emit dbscale((int) min,(int) max); 

	  for (i=0;i<width;i++)
	    p.drawLine (i,(int)(points[i]*height),i+1,(int)(points[i+1]*height));
	  delete points;
	  points=0;
	}
    }
  p.end();
}
