#include <qobject.h>
#include <qpainter.h>
#include <math.h>
#include <limits.h>
#include "classes.h"
#include "dialogs.h"

extern QList<MarkerType>*markertypes;

//****************************************************************************
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
  zoom=1.0;
  zoomy=1;
  down=false;
  reset=false;
  lastx=-1;
  firstx=-1;
  playpointer=-1;
  lastplaypointer=-1;
  pixmap=0;

  markers=new MarkerList;
  markers->setAutoDelete (true);

  markertype=markertypes->first();

  setBackgroundColor (black);

  connect (this,SIGNAL(channelReset()),this->parent(),SLOT(resetChannels()));
}
//****************************************************************************
SigWidget::~SigWidget (QWidget *parent,const char *name)
{
	if (pixmap==0)	delete pixmap;
	if (signal!=0)	delete signal;
	if (markers) delete markers;
}
//****************************************************************************
void SigWidget::saveSignal  (QString *filename,int bit,int selection)
{
  if (signal) signal->save (filename,bit,selection);
}
//****************************************************************************
unsigned char *SigWidget::getOverview (int size)
{
  int step,max=0,min=0;

  unsigned char *overview=new unsigned char [size];

  for (int i=0;i<size;overview[i++]=0); //clear

  if (overview&&signal)
    {
      MSignal *tmp=signal;
 
      double zoom=((double) signal->getLength())/size;
      while (tmp!=0)
	{
	  int *sam=tmp->getSample();
	  for (int i=0;i<size;i++)
	    {
	      step=((int) (((double)i)*zoom));
	      getMaxMin (&sam[step],(int) zoom+2,max,min);
	      if (-min>max) max=-min;
	      overview[i]+=max/65536;
	    }
	  tmp=tmp->getNext();
	}
    }
  return overview;
}
//****************************************************************************
void SigWidget::toggleChannel (int channel)
{
  if (signal)
  signal->toggleChannel (0,channel);
}
//****************************************************************************
void SigWidget::setRangeOp (int op)
{
  if (signal)
    {
      signal->doRangeOp (op);

      if (op>=SELECTMARK&&op<SELECTMARK+20) markertype=markertypes->at(op-SELECTMARK); //set active Markertype

      if ((op>=SAVEBLOCKS)&&(op<=SAVEBLOCKS+24)) saveBlocks (op-SAVEBLOCKS);
      switch (op)
	{
	case ZOOMIN:
	  zoomIn ();
	  break;
	case ZOOMOUT:
	  zoomOut ();
	  break;
	case ZOOMRANGE:
	  zoomRange ();
	  break;
	case SCROLLRIGHT:
	  if (signal)
	    {
	      offset+=int (zoom*width)/10;
	      if (offset>(int)(signal->getLength()-width*zoom)) offset=(int)(signal->getLength()-width*zoom);
	      refresh();
	    }
	  break;
	case NEXTPAGE:
	  if (signal)
	    {
	      offset+=int (zoom*width);
	      if (offset>(int)(signal->getLength()-width*zoom)) offset=(int)(signal->getLength()-width*zoom);
	      refresh();
	    }
	  break;
	case PREVPAGE:
	  if (signal)
	    {
	      offset-=int (zoom*width);
	      if (offset<0) offset=0;
	      refresh();
	    }
	  break;
	case SCROLLLEFT:
	  if (signal)
	    {
	      offset-=int (zoom*width)/10;
	      if (offset<0) offset=0;
	      refresh();
	    }
	  break;
	case DELETEMARK:
	  deleteMarks ();
	  break;
	case APPENDMARK:
	  appendMarks ();
	  break;
	case LOADMARK:
	  loadMarks ();
	  break;
	case SAVEMARK:
	  saveMarks ();
	  break;
	case ADDMARK:
	  addMark ();
	  break;
	case MARKSIGNAL:
	  markSignal ();
	  break;
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
	case DELETECHANNEL:
	  {
	    MSignal *next=signal->getNext();
	    int channels=signal->getChannels();
	    signal->detachChannels();
	    delete signal;
	    signal=next;
	    if (signal)
	      {
		signal->setChannels (channels-1);  //set number of channels to new 

		connect (signal,SIGNAL(channelReset()),this->parent(),SLOT(resetChannels())); //and reconnect ...
		connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));

		emit channelReset();
		refresh ();
	      }
	    break;
	  }
	case SELECTALL:
	  setRange (0,signal->getLength());
	  break;
	case SELECTNEXT:
	  {
	    int r=signal->getRMarker();
	    int l=signal->getLMarker();

	    setRange (r+1,r+1+(r-l));
	    break;
	  }
	case SELECTPREV:
	  {
	    int r=signal->getRMarker();
	    int l=signal->getLMarker();

	    setRange (l-(r-l)-1,l-1);
	    break;
	  }
	case SELECTVISIBLE:
	  setRange	(offset,(int)((double)width*zoom));
	  break;
	case SELECTNONE:
	  setRange	(offset,offset);
	  break;
	case SELECTRANGE:
	  {
	    int l=signal->getLMarker();
	    TimeDialog dialog (this,signal->getRate(),"Select Range");
	    if (dialog.exec())
	      {
		int len=dialog.getLength()*signal->getRate()/1000;
		setRange	(l,l+len);
	      }
	    break;
	  }
	}
    }
  else if (op==NEW)
    {
      NewSampleDialog *dialog=new NewSampleDialog (this);
      if (dialog->exec())
	{
	  int rate=dialog->getRate();
	  int numsamples=dialog->getLength();

	  signal=new MSignal (this,numsamples,rate);

	  if ((signal)&&(signal->getLength()))
	    {
	      markers->clear ();
	      offset=0;
	      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
	      connect (signal,SIGNAL(signalinserted(int,int)),this,SLOT(signalinserted(int,int)));
	      connect (signal,SIGNAL(signaldeleted(int,int)),this,SLOT(signaldeleted(int,int)));
	      connect (signal,SIGNAL(channelReset()),parent(),SLOT(resetChannels()));

	      emit	channelReset	();
	      emit	rateInfo	(signal->getRate());
	      emit	lengthInfo	(signal->getLength());

	      calcTimeInfo ();
	      setZoom (100);
	      refresh ();
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

  if ((firstx<width)&&(nextx>0)) repaint (false);

  down=false;
  lastx=nextx;

  if (signal)
   {
    signal->setMarkers	(l,r);
    emit selectedtimeInfo((r-l)*10000/signal->getRate());
   }
}
//****************************************************************************
void SigWidget::setSignal  (MSignal *sig)
{
  if (signal) delete signal;
  markers->clear ();
  signal=sig;
  signal->setParent (this);
  offset=0;
  if ((signal)&&(signal->getLength()))
    {
      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
      connect (signal,SIGNAL(signalinserted(int,int)),this,SLOT(signalinserted(int,int)));
      connect (signal,SIGNAL(signaldeleted(int,int)),this,SLOT(signaldeleted(int,int)));
      connect (signal,SIGNAL(channelReset()),parent(),SLOT(resetChannels()));

      emit	rateInfo	(signal->getRate());
      emit	lengthInfo	(signal->getLength());
      emit	channelReset	();

      calcTimeInfo ();
    }
}
//****************************************************************************
void SigWidget::setSignal  (QString *filename)
{
  if (signal) delete signal;
  signal=new MSignal (this,filename);
  markers->clear ();

  if ((signal)&&(signal->getLength()))
    {
      offset=0;
      signal->setMarkers (0,0);

      connect (signal,SIGNAL(sampleChanged()),this,SLOT(refresh()));
      connect (signal,SIGNAL(signalinserted(int,int)),this,SLOT(signalinserted(int,int)));
      connect (signal,SIGNAL(signaldeleted(int,int)),this,SLOT(signaldeleted(int,int)));
      connect (signal,SIGNAL(channelReset()),parent(),SLOT(resetChannels()));

      emit	rateInfo	(signal->getRate());
      emit	lengthInfo	(signal->getLength());
      emit	channelReset	();

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
      len*=1000;			//timeinfo needs scale of ms instead of seconds..

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
  else this->zoom=1.0;
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
	int r=signal->getRMarker();
	int l=signal->getLMarker();

	emit selectedtimeInfo((r-l)*10000/signal->getRate());

	emit    channelInfo     (signal->getChannels());
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
      if (e->button()==LeftButton)
	{
	  down	= TRUE;
	  reset	= TRUE;
	  repaint(false);
	  reset	= false;
	  firstx =e->pos().x();
	  lastx	=-1;
	  nextx	=-1;
	}
      if (e->button()==RightButton) lasty=e->pos().y();
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
	      signal->setMarkers	((int)(((double)lastx)*zoom)+offset,(int)(((double)firstx)*zoom)+offset);
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
  if (e->state()==RightButton)
    {
	double old=zoomy;

      zoomy+=(double (e->pos().y()-lasty))*2/height;

      if (zoomy<1) zoomy=1;
      if (zoomy>10) zoomy=10;


      lasty=e->pos().y();
      
	if (zoomy!=old)
	 {
	      redraw	= TRUE;
	      repaint();
	 }
    }
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
void SigWidget::drawSignal (int *sam,int begin,int height)
{
  int lx,x;
  int div=65536;
  div=(int)((double)div/zoomy);

  lx=begin+(sam[offset]/256)*height/div;
  for (int i=1;((i<signal->getLength())&&(i<width));i++)
    {
      x=begin+(sam[offset+i]/256)*height/div;
      p.drawLine (i-1,lx,i,x);
      lx=x;
    }
}
//****************************************************************************
void SigWidget::drawOverviewSignal (int *sam,int begin, int height)
{
  int step,max=0,min=0;
  int div=65536;
  div=(int)((double)div/zoomy);

  for (int i=0;i<width;i++)
    {
      step=((int) (((double)i)*zoom))+offset;
      getMaxMin (&sam[step],(int) zoom+2,max,min);
      max=(max/256)*height/div;
      min=(min/256)*height/div;
      p.drawLine (i,begin+max,i,begin+min);
    }
}
//****************************************************************************
void SigWidget::drawInterpolatedSignal (int *sam,int begin, int height)
{
  double	f;
  int 	j,lx,x,x1,x2;
  int div=65536;

  div=(int)((double)div/zoomy);

  lx=begin+(sam[offset]/256)*height/div;
  for (int i=1;i<width;i++)
    {
      j=(int) floor((double) i*zoom);
      f=(i*zoom)-j;
			
      x1=(sam[offset+j]/256)*height/div;
      x2=(sam[offset+j+1]/256)*height/div;

      x=begin+(int)(x2*f+x1*(1-f));

      p.drawLine (i-1,lx,i,x);
      lx=x;
    }
}
//****************************************************************************
void SigWidget::paintEvent  (QPaintEvent *event)
{
  int updateall=false;
  int update[2]={-1,-1};

  ///if pixmap has to be resized ... or is not allocated ...
  if ((rect().height()!=height)||(rect().width()!=width)||(pixmap==0))
    {
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());
      pixmap->fill (this,0,0);
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
	  int chanheight=height/signal->getChannels ();

	  int 	l=signal->getLMarker();
	  int 	r=signal->getRMarker();

	  //if zoom is to big find maximum zoom
	  if (((zoom*width)>signal->getLength())||(zoom<=0))
	    zoom=((double)signal->getLength())/width;

	  //if zoom is to big for part of the signal to be displayed
	  //show more of the signal by changing offset...
	  if (((int)(((double)width)*zoom))>signal->getLength()-offset)
	    offset=signal->getLength()-((int)((((double)width)*zoom)+.5));
	  //seems to get in here, even when using double
	  MSignal *tmp=signal;

	  int begin=-height/2+chanheight/2;

	  while (tmp!=0)
	    {
	      int *sam=tmp->getSample();

	      if (sam)
		{
		  if (zoom<1)        drawInterpolatedSignal(sam,begin,chanheight);
		  else 	if (zoom>1)  drawOverviewSignal(sam,begin,chanheight);
		  else	             drawSignal (sam,begin,chanheight);
		  p.setPen (green);
		  p.drawLine (0,begin,width,begin);
		  p.setPen (white);
		  tmp=tmp->getNext();
		  begin+=chanheight;
		}
	    }


	  // show selected range ...
	  if (lastx!=-1)
	    {
	      firstx	=(int)((double)(l-offset)/zoom);
	      lastx	=(int)((double)(r-l)/zoom);

	      if (lastx!=0)	firstx++;

	      if ((firstx<width)&&(firstx+lastx>0))  //just another error check
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


	  p.setRasterOp (CopyROP);
	  // show the labels
	  Marker *act;
	  int lastpos=(int)(offset+width*zoom);
	  for (act=markers->first();act;act=markers->next())
	    {
	      if (((int)act->pos>=offset)&&((int)act->pos<lastpos))
		{
		  int x=(int)((act->pos-offset)/zoom);
		  p.setPen (*(act->type->color));
		  p.drawLine (x,-height/2,x,height/2);
		  lastx=-1;

		  if (act->name)
		    {
		      int w=p.fontMetrics().width (act->name->data());
		      int h=p.fontMetrics().height();

		      p.fillRect (x-w/2-1,-height/2+1,w+2,h+2,QBrush(gray));
		      p.setPen (white);
		      p.drawLine (x-w/2-2,-height/2+1,x+w/2+1,-height/2+1);
		      p.drawLine (x-w/2-2,(-height/2)+1,x-w/2-2,(-height/2)+1+h);
		      p.setPen (black);
		      p.drawLine (x+w/2+1,(-height/2)+1,x+w/2+1,(-height/2)+1+h);
		      p.drawText (x-w/2,-height/2+3,w,h,AlignCenter,act->name->data());
		    }
		  
		}
	    }
    }
  else
    {	//no full repaint needed ... here only the playing marker or the range markers gets updated
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
	{
	  int maxofs=((int) (((double)width)*zoom+.5));
	  emit viewInfo	(offset,maxofs,signal->getLength());
	}
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



