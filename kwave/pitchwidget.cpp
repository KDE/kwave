#include <qdir.h>
#include "fftview.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include <qcursor.h>
#include "dialogs.h"
#include "pitchwidget.h"
#include "sample.h"
#include "main.h"

extern KApplication *app;
extern char *mstotimec (int ms);
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
PitchWidget::~PitchWidget (QWidget *parent,const char *name)
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
//****************************************************************************
PitchContainer::PitchContainer (QWidget *parent): QWidget (parent)
{
  this->view=0;
}
//****************************************************************************
void PitchContainer::setObjects (PitchWidget *view,ScaleWidget *x,ScaleWidget *y,CornerPatchWidget *corner)
{
  this->view=view;
  this->xscale=x;
  this->yscale=y;
  this->corner=corner;
}
//****************************************************************************
PitchContainer::~PitchContainer ()
{
}
//****************************************************************************
void PitchContainer::resizeEvent (QResizeEvent *)
{
  if (view)
    {
      int bsize=(QPushButton("test",this).sizeHint()).height();
      view->setGeometry	 (bsize,0,width()-bsize,height()-bsize);  
      xscale->setGeometry(bsize,height()-bsize,width()-bsize,bsize);  
      yscale->setGeometry(0,0,bsize,height()-bsize);
      corner->setGeometry(0,height()-bsize,bsize,bsize);
    }
}
//****************************************************************************
PitchWindow::PitchWindow (QString *name) : KTopLevelWidget ()
{
  QPopupMenu *pitch=	new QPopupMenu ();
  KMenuBar   *bar=	new KMenuBar (this); 

  bar->insertItem	(klocale->translate("&Pitch"),pitch);

  status=new KStatusBar (this);
  status->insertItem    ("Time:         0 ms      ",1);
  status->insertItem    ("Frequency:          0 Hz",2);

  mainwidget=new PitchContainer (this);
  view=  new PitchWidget (mainwidget);
  xscale=new ScaleWidget (mainwidget,0,100,"ms");
  yscale=new ScaleWidget (mainwidget,20000,0,"Hz");
  corner=new CornerPatchWidget (mainwidget);
  mainwidget->setObjects (view,xscale,yscale,corner);

  setView (mainwidget);
  setStatusBar (status);
  setMenu (bar);
  
  connect (view,SIGNAL(freqRange(float,float)),this,SLOT(freqRange(float,float)));
  connect (view,SIGNAL(pitch(float)),this,SLOT(showPitch(float)));
  connect (view,SIGNAL(timeSamples(float)),this,SLOT(showTime(float)));
 
  QString *windowname=new QString (QString ("Pitch of ")+QString(name->data()));
  setCaption (windowname->data()); 
  setMinimumSize (320,200);
}
//****************************************************************************
void PitchWindow::freqRange (float min,float max)
{
  yscale->setMaxMin ((int)min,(int)max);
}
//****************************************************************************
void PitchWindow::showPitch (float freq)
{
  char buf [32];
  sprintf (buf,"Frequency : %.1f Hz\n",freq);
  status->changeItem (buf,2);
}
//****************************************************************************
void PitchWindow::showTime (float time)
{
  char buf [32];
  sprintf (buf,"Time : %s\n",mstotimec((int)((time*1000/rate))));
  status->changeItem (buf,1);
}
//****************************************************************************
void PitchWindow::setSignal (float *data,int len,int rate)
 //reaches through to class PitchWidget, same Method, last int is ommited, since only used for scales, that are managed from this class...
{
  view->setSignal (data,len);
  xscale->setMaxMin ((int)(((double)len)*10000/rate),0);
  this->rate=rate;
}
//****************************************************************************
PitchWindow::~PitchWindow ()
{
}
//****************************************************************************


