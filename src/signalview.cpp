//methods for SignalWidget the view for MSignal objects.
//methods concerning markers may be found in markers.cpp
#include <qobject.h>
#include <qpainter.h>
#include <qcursor.h>
#include <math.h>
#include <limits.h>
#include "signalview.h"
#include "signalmanager.h"
#include "../lib/dialogoperation.h"
#include "../lib/parser.h"
#include "../lib/globals.h"
#include "../lib/dynamicloader.h"
#include "../libgui/kwavemenu.h"
#include "../libgui/kwavedialog.h"


#define SCROLLLEFT	 8000
#define SCROLLRIGHT	 8001
#define NEXTPAGE	 8002
#define PREVPAGE	 8003
#define ZOOMIN	         8004
#define ZOOMOUT  	 8005
#define ZOOMRANGE	 8006
#define ZOOMFULL	 8007

extern Global globals;
//****************************************************************************
SignalWidget::SignalWidget (QWidget *parent,MenuManager *manage) : QWidget (parent)
{
  playing=false;
  redraw=false;

  this->manage=manage;

  manage->addNumberedMenu ("MarkerTypes");

  for (MarkerType *tmp=globals.markertypes.first();tmp;tmp=globals.markertypes.next())
    manage->addNumberedMenuEntry ("MarkerTypes",tmp->name->data());

  timer=0;
  signalmanage=0;
  offset=0;
  zoom=1.0;
  zoomy=1;
  down=false;

  select=new MouseMark (this);

  playpointer=-1;
  lastplaypointer=-1;
  pixmap=0;

  markers=new MarkerList;
  markers->setAutoDelete (true);

  markertype=globals.markertypes.first();

  setBackgroundColor (black);

  setMouseTracking (true);

  connect (this,SIGNAL(channelReset()),this->parent(),SLOT(resetChannels()));
}
//****************************************************************************
SignalWidget::~SignalWidget ()
{
	if (pixmap==0)	delete pixmap;
	if (signalmanage)	delete signalmanage;
	if (markers)    delete markers;
}
//****************************************************************************
void SignalWidget::saveSignal  (QString *filename,int bit,int selection)
{
  if (signalmanage) signalmanage->save (filename,bit,selection);
}
//****************************************************************************
unsigned char *SignalWidget::getOverview (int size)
{
  int step,max=0,min=0;

  unsigned char *overview=new unsigned char [size];

  for (int i=0;i<size;overview[i++]=0); //clear

  if (overview&&signalmanage)
    {
      double zoom=((double) signalmanage->getLength())/size;
      int channels=signalmanage->getChannels();

      for (int c=0;c<channels;c++)
      	{
	  for (int i=0;i<size;i++)
	    {
	      step=((int) (((double)i)*zoom));
	      signalmanage->getMaxMin (c,max,min,step,(int) zoom+2);
	      if (-min>max) max=-min;
	      overview[i]+=max/65536;
	    }
	}
    }
  return overview;
}
//****************************************************************************
void SignalWidget::toggleChannel (int channel)
{
  if (signalmanage) signalmanage->toggleChannel (channel);
}
//****************************************************************************
int SignalWidget::doCommand (const char *str)
{
  return true;
}
//****************************************************************************
void SignalWidget::setOp (int op)
  //this one hopefully catches all functions from mainwidget and topwidget,
  //that should not be delivered to SignalManage
{
  if (signalmanage)
    {
      signalmanage->setOp (op);

      if (op>=SELECTMARK&&op<SELECTMARK+MENUMAX)
	markertype=globals.markertypes.at(op-SELECTMARK);
      //set active Markertype

      if ((op>=SAVEBLOCKS)&&(op<=SAVEBLOCKS+24)) saveBlocks (op-SAVEBLOCKS);

      switch (op)
	{
	case CUT:
	  setRange (signalmanage->getLMarker(),signalmanage->getLMarker());
	  break;
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
	  if (signalmanage)
	    {
	      offset+=int (zoom*width)/10;
	      if (offset>(int)(signalmanage->getLength()-width*zoom)) offset=(int)(signalmanage->getLength()-width*zoom);
	      refresh();
	    }
	  break;
	case NEXTPAGE:
	  if (signalmanage)
	    {
	      offset+=int (zoom*width);
	      if (offset>(int)(signalmanage->getLength()-width*zoom)) offset=(int)(signalmanage->getLength()-width*zoom);
	      refresh();
	    }
	  break;
	case PREVPAGE:
	  if (signalmanage)
	    {
	      offset-=int (zoom*width);
	      if (offset<0) offset=0;
	      refresh();
	    }
	  break;
	case SCROLLLEFT:
	  if (signalmanage)
	    {
	      offset-=int (zoom*width)/10;
	      if (offset<0) offset=0;
	      refresh();
	    }
	  break;
	case TOPITCH:
	  convertMarkstoPitch ();
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
	case ADDMARKTYPE:
	  addMarkType ();
	  break;
	case JUMPTOLABEL:
	  jumptoLabel ();
	  break;
	case MARKSIGNAL:
	  markSignal ();
	  break;
	case MARKPERIOD:
	  markPeriods ();
	  break;
	case SAVEPERIODS:
	  savePeriods ();
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
	case PHALT: //halt by Gerhard Zint
          {
            timer->stop ();
            playing=false;
            int lmarker=signalmanage->getLMarker(), rmarker=signalmanage->getRMarker();
            lmarker = signalmanage->getPlayPosition();
            if (rmarker<lmarker) rmarker = lmarker;
            setRange(lmarker, rmarker);
            playpointer=-1;
            break;
          }
	case SELECTALL:
	  setRange (0,signalmanage->getLength());
	  break;
	case SELECTNEXT:
	  {
	    int r=signalmanage->getRMarker();
	    int l=signalmanage->getLMarker();

	    setRange (r+1,r+1+(r-l));
	    break;
	  }
	case SELECTPREV:
	  {
	    int r=signalmanage->getRMarker();
	    int l=signalmanage->getLMarker();

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
	    selectRange ();
	    break;
	  }
	}
    }
  if (op==NEW) createSignal ();
}
//****************************************************************************
void SignalWidget::selectRange ()
{
  if (signalmanage)
    {
      int rate=signalmanage->getRate();

      KwaveDialog *dialog = DynamicLoader::getDialog
	("time",new DialogOperation (rate,true));
      if ((dialog)&&(!dialog->exec()))
	{
	  int l=signalmanage->getLMarker();

	  KwaveParser parser (dialog->getCommand());
  
	  double ms=parser.toDouble ();
	  int len=(int)(ms*rate/1000);
	  int siglen=signalmanage->getLength();
	  if ((l+len)>siglen) setRange (l,siglen); //overflow check
	  else
	    setRange (l,l+len);
	}
    }
}
//****************************************************************************
void SignalWidget::connectSignal ()
{
  connect (signalmanage,SIGNAL(sampleChanged()),
	   this,SLOT(refresh()));
  connect (signalmanage,SIGNAL(signalinserted(int,int)),
	   this,SLOT(signalinserted(int,int)));
  connect (signalmanage,SIGNAL(signaldeleted(int,int)),this,
	   SLOT(signaldeleted(int,int)));
  connect (signalmanage,SIGNAL(channelReset()),
	   parent(),SLOT(resetChannels()));
  connect (signalmanage,SIGNAL(selectedTimeInfo(int)),
	   parent(),SLOT(setSelectedTimeInfo(int)));

  connect (signalmanage,SIGNAL(channelInfo(int)),
	   parent(),SLOT(getChannelInfo(int)));

  connect (signalmanage,SIGNAL(rateInfo(int)),parent(),
	   SLOT(setRateInfo(int)));
  connect (signalmanage,SIGNAL(lengthInfo(int)),parent(),
	   SLOT(setLengthInfo(int)));
  connect (signalmanage,SIGNAL(timeInfo(int)),parent(),
	   SLOT(setTimeInfo( int)));
  connect (select,SIGNAL(selection(int,int)),signalmanage,
	   SLOT(setMarkers( int,int)));

  signalmanage->info ();
}
//****************************************************************************
void SignalWidget::createSignal ()
{
  KwaveDialog *dialog =
    DynamicLoader::getDialog ("newsample",new DialogOperation ((int)0,true));

  if ((dialog)&&(!dialog->exec()))
    {
      KwaveParser parser (dialog->getCommand());

      int rate=parser.toInt();
      double ms=parser.toDouble();

      int numsamples=(int)(ms*rate/1000);

      if (signalmanage) delete signalmanage;
      markers->clear ();	  
	  
      signalmanage=new SignalManager (this,numsamples,rate,1);

      if (signalmanage) 
	{

	  connectSignal ();

	  emit	channelReset	();
	  setZoom (100);

	  refresh ();
	}
    }
  else debug ("could not find newsample dialog\n");
}
//****************************************************************************
void SignalWidget::setRange  (int l,int r)
{
  select->set (((l-offset)/zoom),((r-offset)/zoom));
  if (signalmanage) signalmanage->setMarkers (l,r);
}
//****************************************************************************
void SignalWidget::setSignal  (SignalManager *sigs)
{
  if (signalmanage) delete signalmanage;
  markers->clear ();
  signalmanage=sigs;
  signalmanage->setParent (this);
  offset=0;
  if ((signalmanage)&&(signalmanage->getLength()))
    {
      connectSignal ();
      emit channelReset	();
    }
}
//****************************************************************************
void SignalWidget::setSignal  (QString *filename,int type)
{
  if (signalmanage) delete signalmanage;  //get rid of old signal
  signalmanage=new SignalManager (this,filename,1,type);
  markers->clear ();

  if (signalmanage)
    {
      offset=0;

      connectSignal ();
      signalmanage->setMarkers (0,0);

      emit channelReset	();
    }
}
//****************************************************************************
void SignalWidget::time ()
{
  int scr=signalmanage->getPlayPosition()-offset;

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

  if (signalmanage->getPlayPosition()==0)
    emit playingfinished();
}
//****************************************************************************
void SignalWidget::setZoom (double zoom)
{
  if (signalmanage) this->zoom=(((double) signalmanage->getLength())/width)*zoom/100;
  else this->zoom=1.0;
  
  refresh();
}
//****************************************************************************
void SignalWidget::zoomNormal	()
{
  zoom=1;
  refresh();
}
//****************************************************************************
void SignalWidget::zoomOut		()
{
  offset-=int (zoom*width);
  if (offset<0) offset=0;
  zoom*=3;

  if (offset+zoom*width>signalmanage->getLength())
    zoom=((double)signalmanage->getLength()-offset)/width;

  refresh();
}
//****************************************************************************
void SignalWidget::zoomIn		()
{
  offset+=int (zoom*width/2);
  zoom/=3;
  offset-=int (zoom*width/2);
  refresh();
}
//****************************************************************************
void SignalWidget::zoomRange()
{
  if (signalmanage)
    {
      int lmarker=signalmanage->getLMarker();
      int rmarker=signalmanage->getRMarker();

      if (lmarker!=rmarker)
	{
	  offset=lmarker;
	  zoom=(((double)(rmarker-lmarker))/width);
	  refresh();
	}
    }
}
//****************************************************************************
void SignalWidget::refresh()
{
  if (signalmanage)
    {
      select->setOffset (offset);
      select->setLength (signalmanage->getLength());
      select->setZoom (zoom);
    }
  redraw=true;

  repaint (false);
};
//****************************************************************************
void SignalWidget::setOffset	(int no)
{
  offset=no;
  refresh ();
}
//****************************************************************************
int SignalWidget::checkPosition (int x)
  //returns given x coordinates is within bonds of area,
  //in which labels may be reset, else false;
{
  if (signalmanage)
    return select->checkPosition (x,width/50);      //2 % of width tolerance

  return false;
}
//****************************************************************************
void SignalWidget::mousePressEvent(QMouseEvent *e)
{
  if (!playing)
    {
      if (e->button()==LeftButton)
	{
	  int x=e->pos().x();
	  if (checkPosition (x))
	    {
	      down=true;
	      select->grep (x);
	    }
	  else
	    {
	      down	= true;
	      select->set (x,x);
	    }
	}
    }
  if (e->button()==RightButton) lasty=e->pos().y();
}
//****************************************************************************
void SignalWidget::mouseReleaseEvent( QMouseEvent *e)
{
  if (down)
    {
      int x=e->pos().x();
      if (x>=width) x=width; //check for some bounds
      if (x<0) x=0;
      select->update (x);
      down=false;
    }
}
//****************************************************************************
void SignalWidget::mouseMoveEvent( QMouseEvent *e )
{
  if (e->state()==RightButton)
    {
      //zooming on y axis... not very useful, will perhaps be replaced by
      //more useful funcitonality...
      //also very time consuming, because the hole viewable range of signal
      //has to be redisplayed with every mousemove...
      double old=zoomy;

      zoomy+=(double (e->pos().y()-lasty))*2/height;

      if (zoomy<1) zoomy=1;
      if (zoomy>10) zoomy=10;

      lasty=e->pos().y();
      
      if (zoomy!=old)
	{
	  redraw = true;
	  repaint();
	}
    }

  if (down)
    {
      //in move mode, a new selection was created or an old one grabbed 
      //this does the changes with every mouse move...
      int x=e->pos().x();
      if (x>=width) x=width; //check for some bounds
      if (x<0) x=0;
      select->update (x);
    }
  else 
    //yes, this code gives the nifty cursor change....
    if (checkPosition (e->pos().x())) setCursor (sizeHorCursor);
    else  setCursor (arrowCursor);
}
//****************************************************************************
void SignalWidget::drawOverviewSignal (int channel,int begin, int height)
{
  printf ("overview\n");
  int step,max=0,min=0;
  int div=65536;
  div=(int)((double)div/zoomy);

  for (int i=0;i<width;i++)
    {
      step=((int) (((double)i)*zoom))+offset;

      signalmanage->getMaxMin (channel,max,min,step,(int) zoom+2);

      max=(max/256)*height/div;
      min=(min/256)*height/div;
      p.drawLine (i,begin+max,i,begin+min);
    }
}
//****************************************************************************
void SignalWidget::drawInterpolatedSignal (int channel,int begin, int height)
{
  double f;
  int 	j,lx,x,x1,x2;
  int div=65536;

  div=(int)((double)div/zoomy);

  lx=begin+(signalmanage->getSingleSample(channel,offset)/256)*height/div;
  for (int i=1;i<width;i++)
    {
      j=(int) floor((double) i*zoom);
      f=(i*zoom)-j;
			
      x1=(signalmanage->getSingleSample(channel,offset+j)/256)*height/div;
      x2=(signalmanage->getSingleSample(channel,offset+j+1)/256)*height/div;

      x=begin+(int)(x2*f+x1*(1-f));

      p.drawLine (i-1,lx,i,x);
      lx=x;
    }
}
//****************************************************************************
void SignalWidget::paintEvent  (QPaintEvent *event)
{
  int update[2]={-1,-1};
  int updateall=false;

  ///if pixmap has to be resized ... or is not yet allocated ...

  if ((rect().height()!=height)||(rect().width()!=width)||(pixmap==0))
    {
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());
      pixmap->fill (this,0,0);
      updateall=true;
    }

  if (pixmap) //final security check for the case of low memory (->rare thing)
    {
      p.begin (pixmap);
      p.setPen (QPen(NoPen));
 
      if (updateall||redraw)
	{
	  if (redraw)
	    {
	      p.fillRect	(0,0,width,height,black);
	      redraw=false;
	    }	

	  p.setPen (white);

	  if (signalmanage)
	    {
	      int channels=signalmanage->getChannels ();
	      int chanheight=height/channels;
	      int begin=+chanheight/2;

	      //check and correction of current zoom value if needed
	      if (zoom*width>signalmanage->getLength())
		{
		  zoom=((double)signalmanage->getLength ())/width;
		  select->setZoom (zoom); //notify selection
		}
	      for (int i=0;i<channels;i++)
		{
		  if (zoom<=1)
		    drawInterpolatedSignal(i,begin,chanheight);
		  else if (zoom>1)
		    drawOverviewSignal (i,begin,chanheight);

		  p.setPen (green);
		  p.drawLine (0,begin,width,begin);
		  p.setPen (white);
		  begin+=chanheight;
		}

	      // show selected range ...
	      select->drawSelection (&p,width,height);
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
		  p.drawLine (x,0,x,height);

		  if (act->name)
		    {
		      int w=p.fontMetrics().width (act->name->data());
		      int h=8;
		      h=p.fontMetrics().height();

		      p.fillRect (x-w/2-1,1,w+2,h+2,QBrush(gray));
		      p.setPen (white);
		      p.drawLine (x-w/2-2,1,x+w/2+1,1);
		      p.drawLine (x-w/2-2,1,x-w/2-2,1+h);
		      p.setPen (black);
		      p.drawLine (x+w/2+1,1,x+w/2+1,1+h);
		      p.drawText (x-w/2,3,w,h,AlignCenter,act->name->data());
		    }
		}
	    }
	}
      else
	{	//no full repaint needed...
	  //only the playing marker or the range markers gets updated
	  if ((down)&&(!playing))
	    {
	      p.setBrush (yellow);
	      p.setPen (yellow);
	      p.setRasterOp (XorROP);

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
	  if (signalmanage)
	    {
	      int maxofs=((int) (((double)width)*zoom+.5));
	      emit viewInfo	(offset,maxofs,signalmanage->getLength());
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
}

















