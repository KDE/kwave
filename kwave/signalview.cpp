#include "classes.h"
#include <qpainter.h>
#include <math.h>
#include <limits.h>

__inline void  getMaxMin (int *sample,int len,int &max,int &min)
{
  int c;
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      c=sample[i];
      if (c>max) max=c;
      if (c<min) min=c;
    }
}
//****************************************************************************
SigWidget::SigWidget (QWidget *parent,const char *name) : QWidget (parent,name)
{
  playing=false;
  redraw=false;

  timer=0;
  signal=0;
  offset=0;
  zoom=1;
  down=false;
  reset=false;
  lastx=-1;
  firstx=-1;
  playpointer=-1;
  lastplaypointer=-1;
}
//****************************************************************************
SigWidget::~SigWidget (QWidget *parent,const char *name)
{
	if (pixmap==0)	delete pixmap;
	if (signal!=0)	delete signal;
}
//****************************************************************************
void SigWidget::saveSignal  (QString *filename)
{
  if (signal) signal->save16Bit (filename);
}
//****************************************************************************
void SigWidget::setRangeOp (int op)
{
  if (signal)
    {
      signal->doRangeOp (op);

      switch (op)
	{
	case PLAY:
	case LOOP:
	  if (timer==0)
	    {
	      timer=new QTimer (this);
	      connect (timer,SIGNAL(timeout()),this,SLOT(time()));
	    }
	  playing=true;
	  timer->start (20);
	  break;
	case PSTOP:
	  timer->stop ();
	  playing=false;
	  playpointer=-1;

	
	  if (lastplaypointer >=0) repaint (false);
	  break;
	case SELECTALL:
	  setRange (0,signal->getLength());
	  break;
	case SELECTVISIBLE:
	  setRange	(offset,(int)(width*zoom));
	  break;
	case SELECTNONE:
	  setRange	(offset,offset);
	  break;
	}
    }
  else if (op==NEW)
    {
      NewSampleDialog *dialog=new NewSampleDialog (this);
      if (dialog->exec())
	{
	  int 	rate=dialog->getRate();
	  int     numsamples=(int) (((long long)(dialog->getLength()))*rate/1000);

	  signal=new MSignal (this,numsamples,rate);

	  if ((signal)&&(signal->getLength()))
	    {
	      offset=0;
	      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
	      emit	rateInfo	(signal->getRate());
	      emit	lengthInfo	(signal->getLength());

	 	calcTimeInfo ();
	    }
	}
    }
}
//****************************************************************************
void SigWidget::setRange  (int l,int r)
{
  down=true;
  reset=true;
  repaint(false);
  reset=false;

  firstx	=(int)((l-offset)/zoom);
  nextx	=(int)((r-offset)/zoom);

  if ((firstx<width)&&(nextx>0))
    repaint (false);

  down=false;
  lastx=nextx;

  if (signal)
   {
    signal->setMarkers	(l,r);
    emit selectedtimeInfo((int)((lastx-firstx)*zoom*10000/signal->getRate()));
   }
}
//****************************************************************************
void SigWidget::setSignal  (MSignal *sig)
{
  signal=sig;
  offset=0;
  if ((signal)&&(signal->getLength()))
    {
      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
      emit	rateInfo	(signal->getRate());
      emit	lengthInfo	(signal->getLength());

	calcTimeInfo ();
    }
}
//****************************************************************************
void SigWidget::setSignal  (QString *filename)
{
  signal=new MSignal (this,filename);

  if ((signal)&&(signal->getLength()))
    {
      offset=0;
      signal->setMarkers (0,0);

      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
      emit	rateInfo	(signal->getRate());
      emit	lengthInfo	(signal->getLength());

	calcTimeInfo ();
    }
}
//****************************************************************************
void SigWidget::calcTimeInfo ()
{
	if (signal)
	 {
		double len=signal->getLength();	//get the number of samples
		len/=signal->getRate();		//divide through sampling rate
		len*=1000;			//timeinfo needs scale of ms

		emit timeInfo ((int)len);
	 }
}
//****************************************************************************
void SigWidget::time ()
{
  int scr=signal->getPlayPosition()-offset;

  if ((scr<(int)(width*zoom))&&(scr>0))
    {
      int pointer=(int)(scr/zoom);
	if (pointer!=lastplaypointer)
	 {
		playpointer=pointer;
	      repaint (false);
	 }
    }
  else
    if (lastplaypointer >=0) repaint (false);

  if (signal->getPlayPosition()==0)
    emit playingfinished();
}
//****************************************************************************
void SigWidget::setZoom (double zoom)
{
  if (signal)
    this->zoom=(((double) signal->getLength())/width)*zoom/100;

  refresh();
}
//****************************************************************************
void SigWidget::zoomNormal	()
{
  zoom=1;
  refresh();
}
//****************************************************************************
void SigWidget::zoomOut		()
{
  offset-=int (zoom*width);
  if (offset<0) offset=0;
  zoom*=3;
  refresh();
}
//****************************************************************************
void SigWidget::zoomIn		()
{
  offset+=int (zoom*width/2);
  zoom/=3;
  offset-=int (zoom*width/2);
  refresh();
}
//****************************************************************************
void SigWidget::zoomRange	()
{
  if (signal)
    {
      int lmarker=signal->getLMarker();
      int rmarker=signal->getRMarker();
	if (lmarker!=rmarker)
	 {
	      offset=lmarker;
	      zoom=(((double)(rmarker-lmarker))/width);
	      refresh();
	 }
    }
}
//****************************************************************************
void SigWidget::refresh	()
{
  redraw=true;
  repaint (false);
  if (signal)
    if (signal->getLength())
      {
	emit	rateInfo	(signal->getRate());
	emit	lengthInfo	(signal->getLength());
	
	calcTimeInfo ();
      }
};
//****************************************************************************
void SigWidget::setOffset	(int no)
{
  if (signal!=0)
    {
      offset=no;
      redraw=true;
      repaint (false);
    }
}
//****************************************************************************
void SigWidget::mousePressEvent( QMouseEvent *e)
{
  if (!playing)
    {
      down	= TRUE;
      reset	= TRUE;
      repaint(false);
      reset	= false;
      firstx =e->pos().x();
      lastx	=-1;
      nextx	=-1;
    }
}
//****************************************************************************
void SigWidget::mouseReleaseEvent( QMouseEvent *e)
{
  if (down)
    {
      lastx=nextx;
      nextx=e->pos().x();             // add point
      if (nextx<0) nextx=0;
      if (nextx>width) nextx=width;

      repaint(false);
      if (nextx==firstx) lastx=firstx;
	
      down = FALSE;			// done recording markers
      if (signal)
	{
	  if (firstx>lastx)
	    {
	      signal->setMarkers ((int)(lastx*zoom)+offset,(int)(firstx*zoom)+offset);
	      emit selectedtimeInfo((int)((firstx-lastx)*zoom*10000/signal->getRate()));
	    }
	  else
	    {
	      signal->setMarkers ((int)(firstx*zoom)+offset,(int)(lastx*zoom)+offset);
	      emit selectedtimeInfo((int)((lastx-firstx)*zoom*10000/signal->getRate()));
	    }
	}
    }
}
//****************************************************************************
void SigWidget::mouseMoveEvent( QMouseEvent *e )
{
  if (down) {
    lastx=nextx;
    nextx=e->pos().x();             // add point
    if (nextx<0) nextx=0;
    if (nextx>width) nextx=width;

    repaint (false);

    if (signal)
      {
	if (firstx>lastx)
	  emit	selectedtimeInfo((int)((firstx-lastx)*zoom*10000/signal->getRate()));
	else
	  emit	selectedtimeInfo((int)((lastx-firstx)*zoom*10000/signal->getRate()));
      }
  }
}
//****************************************************************************
void SigWidget::deleteLastRange  ()
{
  if (lastx>=0)
    if (lastx==firstx)	//is it a single line ?
      {
	p.setPen (green);
	p.drawLine (firstx,-height/2,firstx,height);
	p.setPen (yellow);
      }
    else
      p.drawRect (firstx,-height/2,lastx-firstx,height);
}
//****************************************************************************
void SigWidget::drawRange  ()
{

  if (firstx>=0)
    if (nextx==firstx)
      {
	p.setPen (green);
	p.drawLine (firstx,-height/2,firstx,height);
      }
    else
      {
	if (nextx>=0)
	  p.drawRect (firstx,-height/2,nextx-firstx,height);
      }
}
//****************************************************************************
void SigWidget::drawSignal ()
{
  int	*sam=0;
  if (signal)	sam=signal->getSample();
  if (sam!=0)
    {
      int lx,x;
      lx=(sam[offset]/256)*height/65536;
      for (int i=1;((i<signal->getLength())&&(i<width));i++)
	{
	  x=(sam[offset+i]/256)*height/65536;
	  p.drawLine (i-1,lx,i,x);
	  lx=x;
	}
    }
}
//****************************************************************************
void SigWidget::drawOverviewSignal ()
{
	
  int	*sam=0;
  if (signal)	sam=signal->getSample();
  if (sam)
    {
      int step,max=0,min=0;

      for (int i=0;i<width;i++)
	{
	  step=((int) zoom*i)+offset;
	  getMaxMin (&sam[step],(int) (zoom)+1,max,min);
	  max=(max/256)*height/65536;
	  min=(min/256)*height/65536;
	  p.drawLine (i,max,i,min);
	}
    }
}
//****************************************************************************
void SigWidget::drawInterpolatedSignal ()
{
  int	*sam=0;
  if (signal)	sam=signal->getSample();
  if (sam!=0)
    {
      double	f;
      int 	j,lx,x,x1,x2;

      lx=(sam[offset]/256)*height/65536;
      for (int i=1;i<width;i++)
	{
	  j=(int) floor(i*zoom);
	  f=(i*zoom)-j;
			
	  x1=(sam[offset+j]/256)*height/65536;
	  x2=(sam[offset+j+1]/256)*height/65536;

	  x=(int)(x2*f+x1*(1-f));

	  p.drawLine (i-1,lx,i,x);
	  lx=x;
	}
    }
}
//****************************************************************************
void SigWidget::paintEvent  (QPaintEvent *event)
{
  int updateall=false;
  int update[2]={-1,-1};
 
  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width))
    {
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());
      updateall=true;
    }
  p.begin (pixmap);
  p.setPen (QPen(NoPen));
  p.translate (0,height/2);


  if (updateall||redraw)
    {
      if (redraw)
	{
	  p.fillRect	(0,-height/2,width,height,black);
	  redraw=false;
	}	

      p.setPen (white);

      if ((signal)&&(signal->getLength()))
	{
	  int 	l=signal->getLMarker();
	  int 	r=signal->getRMarker();

	  //if zoom is to big find maximum zoom
	  if (((int)width*zoom)>signal->getLength())
	    zoom=signal->getLength()/width;

	  //if zoom is to big for part of the signal to be displayed
	  //show more of the signal...
	  if (((int)width*zoom)>signal->getLength()-offset)
	    offset=signal->getLength()-((int)(width*zoom));

	  if (zoom<1) drawInterpolatedSignal();
	  else 	if (zoom>1)	drawOverviewSignal();
	  else	drawSignal();

	  if (lastx!=-1)
	    {
	      firstx	=(int)((l-offset)/zoom);
	      lastx	=(int)((r-l)/zoom);

	      if ((firstx<width)&&(firstx+lastx>0))
		{
		  if (firstx<0)
		    {
		      lastx+=firstx;
		      firstx=0;
		    }
		  if (lastx>width) lastx=width;

		  p.setBrush (yellow);
		  p.setRasterOp (XorROP);

		  if (r==l)
		    {
		      p.setPen 	(green);
		      p.drawLine	(firstx,-height/2,firstx,height);
		      lastx=firstx;
		    }
		  else
		    {
		      p.setPen	(yellow);
		      p.drawRect	(firstx,-height/2,lastx,height);
		      lastx=firstx+lastx;
		    }
		  nextx=-1;
		}
	    }
	  updateall=true;
	}
      lastplaypointer=-1;
    }
  else
    {	//no full repaint needed ... here only RangeMarkers get updated
      if ((down)&&(!playing))
	{
	  p.setBrush (yellow);
	  p.setPen (yellow);
	  p.setRasterOp (XorROP);

	  if (reset)			//initial redraw after click
	    {				
	      if (lastx!=-1) deleteLastRange ();
	      lastx=-1;
	    }
	  else 
	    {
	      if (lastx!=nextx);
	      {
		if (lastx>=0) deleteLastRange ();
		drawRange ();
	      }
	    }
	  updateall=true;
	}

    }
  p.setPen (gray);
  p.drawLine (0,0,width,0);

  if (playpointer>=0)
    {
      p.setRasterOp (XorROP);
      p.setPen (green);

      if (lastplaypointer >=0)
	p.drawLine (lastplaypointer,-height/2,lastplaypointer,height);

      p.drawLine (playpointer,-height/2,playpointer,height);
      update[1]=lastplaypointer;
      lastplaypointer=playpointer;
      update[0]=playpointer;
      playpointer=-1;
    }
  else
    if (lastplaypointer >=0)
      {
	p.setRasterOp (XorROP);
	p.setPen (green);
	p.drawLine (lastplaypointer,-height/2,lastplaypointer,height);
	update[0]=lastplaypointer;
	lastplaypointer=-1;
      }

  p.end();

  if (updateall)
    {
      if (signal)
	emit viewInfo	(offset,(int)(width*zoom),signal->getLength()-(int) (width*zoom));
      bitBlt (this,0,0,pixmap);
    }
  else
    if (update[0]<0)
      {
	QRect	pos=event->rect();
	bitBlt (this,pos.topLeft(),pixmap,pos);
      }

  if (update[0]!=-1) bitBlt (this,update[0],0,pixmap,update[0],0,1,height);
  if (update[1]!=-1) bitBlt (this,update[1],0,pixmap,update[1],0,1,height);
}
