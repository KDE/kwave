#include <stdio.h>
#include <qpainter.h>
#include <qpixmap.h>
#include "dialog_progress.h"

#include <libkwave/globals.h>

extern Global globals;
//uncomment this to get a rather fancy Progress indicator, which shows that I've to
//much free time
//#define FANCY

//**********************************************************
//KProgresss does flicker a lot, has bugs, that appear when using high
//numeric values, so here follows my own implementation... 
//**********************************************************
ProgressDialog::ProgressDialog (TimeOperation *operation,const char *caption): QDialog(0,caption)
  //timerbased variant, peeks at address "counter" to gain progress information
{
#ifdef FANCY  
  last=0;
  setMinimumSize (128,128);
  setMaximumSize (128,128);
  setCaption (caption);
  this->max=operation->getLength();
  act=0;
  lastx=0;
  lasty=0;
  setBackgroundColor (black);
#else
  resize (320,20);
  setCaption (caption);
  this->max=operation->getLength();
  act=0;
#endif

  this->operation=operation;
  timer=new QTimer (this);
  connect( timer, SIGNAL(timeout()),SLOT(timedProgress()));
  timer->start( 200); //5 times per second should be enough
}
//**********************************************************
ProgressDialog::ProgressDialog (int max,char *caption): QDialog(0,caption)
  //this one needs calls to get updated
{
#ifdef FANCY  
  last=0;
  setMinimumSize (128,128);
  setMaximumSize (128,128);
  setCaption (caption);
  this->max=max;
  act=0;
  lastx=0;
  lasty=0;
  setBackgroundColor (black);
#else
  resize (320,20);
  setCaption (caption);
  this->max=max;
  act=0;
#endif

  setCursor (waitCursor);
  timer=0;
  operation=0;
}
//**********************************************************
void ProgressDialog::timedProgress ()
{
  if (operation)
    {
      int prog=operation->getCounter();

      if ((prog)<0) //signal for ending...
	{
	  globals.port->putMessage ("refresh()");
	  delete this;
	}
      else
      setProgress (prog);
     }
}
//**********************************************************
void ProgressDialog::setProgress (int x)
{
  act=x;
  repaint(false);
}
//**********************************************************
void ProgressDialog::paintEvent (QPaintEvent *)
{
  QPainter p;
  int perc=(int)(((double)act)*100/max);

#ifdef FANCY
  int width=128;
  int height=128;

  p.begin (this);
  int col=(int)((sin(60*M_PI*(double)act/max)+1)*64);
  p.setPen (QPen(QColor(col,col,col),width/16));
  //  p.setRasterOp (XorROP);
  p.translate (width/2,height/2);
  
  int newx=(int)(sin(64*M_PI*(double)act/max)*width/sqrt(2)*act/max);
  int newy=(int)(cos(64*M_PI*(double)act/max)*height/sqrt(2)*act/max);
  
  p.drawLine   (lastx,lasty,newx,newy);

  if (perc>last)
    {
      char buf[10];
      sprintf (buf,"%d %%",perc);
      last=perc;

      p.setRasterOp (CopyROP);

      QFont newfont(font());

      newfont.setPointSize (width/4);
      newfont.setBold (true);

      p.setPen (black);
      p.setFont  (newfont);
      p.drawText (-width/2+3,-height/2,width,height,AlignCenter,buf,5);
      p.setFont  (newfont);
      p.drawText (-width/2-3,-height/2,width,height,AlignCenter,buf,5);
      p.setFont  (newfont);
      p.drawText (-width/2,-height/2+3,width,height,AlignCenter,buf,5);
      p.setFont  (newfont);
      p.drawText (-width/2,-height/2-3,width,height,AlignCenter,buf,5);

      newfont.setPointSize (width/4);
      newfont.setBold (true);

      p.setPen (white);
      p.setFont  (newfont);
      p.drawText (-width/2,-height/2,width,height,AlignCenter,buf,5);
    }

  lastx=newx;
  lasty=newy;
  p.end();
#else
  int col=(int)((((double)act)/max)*(512+64));
  int w=(int)((((double)act)/max)*(width()-2));
  if (w!=oldw)
    {
      QPixmap map (width(),height());
      map.fill (colorGroup().background());
      char buf[10]="         ";
      sprintf (buf,"%d %%",perc);

      p.begin (&map);
      if (col<256)
	{
	  p.setPen (QColor(255,col,0));
	  p.setBrush (QColor(255,col,0));
	}
      else
	if (col<512)
	  {
	    int x=511-col;
	    p.setPen (QColor(x,255,0));
	    p.setBrush (QColor(x,255,0));
	  }
	else
	  if (col<768)
	    {
	      int x=767-col;
	      p.setPen (QColor(0,x,0));
	      p.setBrush (QColor(0,x,0));
	    }
	  else
	    {
	      p.setPen (QColor(0,0,0));
	      p.setBrush (QColor(0,0,0));
	    }

      p.drawRect (1,1,w,height()-2);
      p.setPen (colorGroup().light());
      p.drawLine (0,height()-1,width(),height()-1);
      p.drawLine (width()-1,0,width()-1,height()-2);
      p.setPen (colorGroup().dark());
      p.drawLine (0,0,width(),0);
      p.drawLine (0,0,0,height()-1);
      p.end ();

      bitBlt (this,0,0,&map);

      p.begin (this);
      p.setPen (colorGroup().text());

      p.drawText (0,0,width(),height(),AlignCenter,buf,5);
      p.end ();
      w=oldw;
    }
#endif
}
//**********************************************************
ProgressDialog::~ProgressDialog ()
{
  if (timer) timer->stop();
  setCursor (arrowCursor);
  if (operation) delete operation;
}
