#include "PitchWidget.h"
#include <qpainter.h>

//****************************************************************************
PitchWidget::PitchWidget (QWidget *parent)
 : QWidget (parent)
{
  data=0;
  len=0;
  height=-1;
  pixmap=0;
  setCursor (crossCursor);
  setBackgroundColor (QColor(black) );
}
//****************************************************************************
PitchWidget::~PitchWidget ()
{
  if (pixmap==0) delete pixmap;
}
//****************************************************************************
void PitchWidget::setSignal  (float *data,int len)
{
  this->data=data;
  this->len=len;
  getMaxMin ();
}
//****************************************************************************
void PitchWidget::refresh()
{
  redraw=true;
  repaint ();
}
//****************************************************************************
void PitchWidget::mousePressEvent( QMouseEvent *)
{
}
//****************************************************************************
void PitchWidget::mouseReleaseEvent( QMouseEvent *)
{
}
//****************************************************************************
void PitchWidget::mouseMoveEvent( QMouseEvent *e )
{
  int x=e->pos().x();
  if ((x<width)&&(x>=0))
    {
      int y=e->pos().y();
      int x=e->pos().x();
      emit pitch (((float)height-y)/height*(max-min)+min);
      emit timeSamples (((float)x)*len/width);
    }
}
//****************************************************************************
void PitchWidget::getMaxMin  ()
{
  if (data)
    {
      max=0;
      min=10000000;

      for (int i=0;i<len;i++)
	{
	  if (data[i]>max) max=data[i];
	  if (data[i]<min) min=data[i];
	}
      emit freqRange (min,max);
    }
}
//****************************************************************************
void PitchWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  /// if pixmap has to be resized ...
    if ((rect().height()!=height)||(rect().width()!=width)||redraw)
      {
	redraw=false;
	height=rect().height();
	width=rect().width();

	if (pixmap) delete pixmap;
	pixmap=new QPixmap (size());

	pixmap->fill (black);

	if (data)
	  {
	    p.begin (pixmap);
	    p.translate (0,height);

	    p.setPen (white);

	    for (int i=0;i<width-1;i++)
	      {
		int ofs=(int) ((double)i*len/width);
		int ofs2=(int) ((double)(i+1)*len/width);
		for (int j=ofs;j<ofs2;j++)
		  {
		    int y=(int)(data[j]/(max-min)*height);
		    p.drawPoint (i,-y);
		  }
	      }
	    p.end();
	  }
      }
    //blit pixmap to window
    if (pixmap) bitBlt (this,0,0,pixmap);
}



