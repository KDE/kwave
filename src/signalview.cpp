//methods for SignalWidget the view for MSignal objects.
//methods concerning markers may be found in markers.cpp
#include <qobject.h>
#include <qpainter.h>
#include <qcursor.h>
#include <math.h>
#include <limits.h>
#include "signalview.h"
#include "signalmanager.h"
#include <libkwave/dialogoperation.h>
#include <libkwave/parser.h>
#include <libkwave/globals.h>
#include <libkwave/dynamicloader.h>
#include "../libgui/kwavemenu.h"
#include "../libgui/kwavedialog.h"
#include "dialog_progress.h"

extern Global globals;
//****************************************************************************
void *createProgressDialog (TimeOperation *operation,const char *caption)
{
  ProgressDialog *dialog=new ProgressDialog (operation,caption);
  if (dialog)
    {
      dialog->show ();
      return (void *) dialog;
    }
  return 0;
}
//****************************************************************************
SignalWidget::SignalWidget (QWidget *parent,MenuManager *manage) : QWidget (parent)
{
  playing=false;
  redraw=false;

  this->manage=manage;

  manage->addNumberedMenu ("labeltypes");

  for (MarkerType *tmp=globals.markertypes.first();tmp;tmp=globals.markertypes.next())
    manage->addNumberedMenuEntry ("labeltypes",tmp->name);

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

  labels=new MarkerList;
  labels->setAutoDelete (true);

  markertype=globals.markertypes.first();

  setBackgroundColor (black);

  setMouseTracking (true);

  connect (this,SIGNAL(channelReset()),this->parent(),SLOT(resetChannels()));
}
//****************************************************************************
SignalWidget::~SignalWidget ()
{
  if (pixmap==0)    delete pixmap;
  if (signalmanage) delete signalmanage;
  if (labels)      delete labels;
}
//****************************************************************************
void SignalWidget::saveSignal  (const char *filename,int bit,int selection)
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
      int channels=signalmanage->getChannelCount ();

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
bool SignalWidget::checkForNavigationCommand (const char *str)
{
  if (matchCommand (str,"zoomin")) zoomIn ();
  else
    if (matchCommand (str,"zoomout")) zoomOut ();
    else
      if (matchCommand (str,"zoomrange")) zoomRange ();
      else
	if (signalmanage)
	  {
	    if (matchCommand (str,"scrollright"))
	      {
		offset+=int (zoom*width)/10;
		if (offset>(int)(signalmanage->getLength()-width*zoom))
		  offset=(int)(signalmanage->getLength()-width*zoom);
		refresh();
	      }
	    else
	      if (matchCommand (str,"viewnext"))
		{
		  offset+=int (zoom*width);
		  if (offset>(int)(signalmanage->getLength()-width*zoom))
		    offset=(int)(signalmanage->getLength()-width*zoom);
		  refresh();
		}
	      else
		if (matchCommand (str,"viewprev"))
		  {
		    offset-=int (zoom*width);
		    if (offset<0) offset=0;
		    refresh();
		  }
		else
		  if (matchCommand (str,"scrollleft"))
		    {
		      offset-=int (zoom*width)/10;
		      if (offset<0) offset=0;
		      refresh();
		    }
		  else
		    if (matchCommand (str,"selectall"))
		      setRange (0,signalmanage->getLength());
		    else
		      if (matchCommand (str,"selectnext"))
			{
			  int r=signalmanage->getRMarker();
			  int l=signalmanage->getLMarker();
			  
			  setRange (r+1,r+1+(r-l));
			}
		      else
			if (matchCommand (str,"selectprev"))
			  {
			    int r=signalmanage->getRMarker();
			    int l=signalmanage->getLMarker();
			    setRange (l-(r-l)-1,l-1);
			  }
			else
			  if (matchCommand (str,"selectvisible"))  
			    setRange (offset,(int)((double)width*zoom));
			  else
			    if (matchCommand (str,"selectnone"))	
			      setRange (offset,offset);
			    else
			      if (matchCommand (str,"selectrange"))	
				selectRange ();
			      else return false;
	  }
  
  return true;
}
//****************************************************************************
bool SignalWidget::checkForLabelCommand (const char *str)
{
  if (matchCommand (str,"chooselabel"))
    {
      KwaveParser parser (str);
      markertype=globals.markertypes.at(parser.toInt());
    }
  else
    if (matchCommand (str,"amptolabel")) markSignal (str);
    else
      if (matchCommand (str,"pitch")) markPeriods (str);
      else
	//	if (matchCommand (str,"labeltopitch"))   convertMarkstoPitch (str);
	//	else
	  if (matchCommand (str,"deletelabel"))   deleteLabel ();
	  else
	    if (matchCommand (str,"insertlabel"))   appendLabel ();
	    else
	      if (matchCommand (str,"loadlabel"))  loadLabel ();
	      else
		if (matchCommand (str,"savelabel"))  saveLabel (str);
		else
		  if (matchCommand (str,"label"))   addLabel (str);
		  else
		    if (matchCommand (str,"newlabeltype")) addLabelType (str);
		    else
		      if (matchCommand (str,"expandtolabel")) jumptoLabel ();
		      else
			if (matchCommand (str,"mark")) markSignal (str);
			else 
			  if (matchCommand (str,"markperiod")) markPeriods (str);
			  else
			    if (matchCommand (str,"saveperiods")) savePeriods ();
			    else return false;

  return true;
}
//****************************************************************************
int SignalWidget::doCommand (const char *str)
{
  printf ("%s\n",str);
  if (matchCommand (str,"dialog"))
    {
      KwaveParser parser (str);
      const char *name=parser.getFirstParam ();
      printf ("loading %s\n",name);
      showDialog (name);
    }
  else
    if (matchCommand (str,"refresh")) refresh ();
    else
    if (matchCommand (str,"newsignal")) createSignal (str);
    else
      if (checkForLabelCommand (str));
      else
	if (checkForNavigationCommand (str));
	else
	  if (matchCommand (str,"cut")) //set range after cutting
	    {
	      bool x=signalmanage->doCommand (str);
	      setRange (signalmanage->getLMarker(),signalmanage->getLMarker());
	      return x;
	    }
	  else
	    return signalmanage->doCommand (str);
  return false;
}
//**********************************************************
void SignalWidget::showDialog (const char *name)
{
  int length=0;
  int rate=44100;
  int channels=0;
  if (signalmanage) length=signalmanage->getLength ();
  if (signalmanage) rate=signalmanage->getRate ();
  if (signalmanage) channels=signalmanage->getChannelCount ();

  DialogOperation *operation=
    new DialogOperation (&globals,length,rate,channels);

  if (operation)
    {
      KwaveDialog *dialog=DynamicLoader::getDialog (name,operation);
      if (dialog)
	{
	  connect (dialog,SIGNAL(command(const char*)),
		   parent(),SLOT(doCommand(const char *)));
	  dialog->show();
	}
      else debug ("error: could not get dialog !\n");

      delete operation;
    }
}
//****************************************************************************
void SignalWidget::setOp (int op)
  //this one catches all functions from mainwidget and topwidget,
  //that should not be delivered to SignalManage
{
  if (signalmanage)
    {
      signalmanage->setOp (op);

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
	}
    }
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
  // connect (signalmanage,SIGNAL(signalinserted(int,int)),
  //	   this,SLOT(signalinserted(int,int)));
  //  connect (signalmanage,SIGNAL(signaldeleted(int,int)),this,
  //	   SLOT(signaldeleted(int,int)));
  connect (this,SIGNAL(selectedTimeInfo(int)),
  	   parent(),SLOT(setSelectedTimeInfo(int)));

  connect (this,SIGNAL(rateInfo(int)),parent(),
	   SLOT(setRateInfo(int)));
  connect (this,SIGNAL(lengthInfo(int)),parent(),
  	   SLOT(setLengthInfo(int)));
  connect (this,SIGNAL(timeInfo(int)),parent(),
  	   SLOT(setTimeInfo( int)));
  connect (select,SIGNAL(selection(int,int)),this,
  	   SLOT(estimateRange( int,int)));

  signalmanage->info ();
}
//****************************************************************************
int SignalWidget::getSignalCount ()
{
  if (signalmanage) return signalmanage->getChannelCount ();
    return 0;
}
//****************************************************************************
void SignalWidget::createSignal (const char *str)
{
  KwaveParser parser (str);

  int rate=parser.toInt();
  double ms=parser.toDouble();

  int numsamples=(int)(ms*rate/1000);

  if (signalmanage) delete signalmanage;
  labels->clear ();	  

  signalmanage=new SignalManager (this,numsamples,rate,1);

  if (signalmanage) 
    {
      connectSignal ();
      emit	channelReset	();
      // ### emit checkMenu("bits(8)", true);
      setZoom (100);
      refresh ();
    }
}
//****************************************************************************
void SignalWidget::estimateRange  (int l,int r)
{
      emit selectedTimeInfo((int)(((long long)(r-l))*10000/signalmanage->getRate()));
}
//****************************************************************************
void SignalWidget::setRange  (int l,int r,bool set)
{
  if (set)  select->set (((l-offset)/zoom),((r-offset)/zoom));
  if (signalmanage)
    {
      signalmanage->setRange (l,r);
      estimateRange (l,r);
    }
}
//****************************************************************************
void SignalWidget::setSignal  (SignalManager *sigs)
{
  if (signalmanage) delete signalmanage;
  labels->clear ();
  signalmanage=sigs;
  signalmanage->setParent (this);
  offset=0;
  if ((signalmanage)&&(signalmanage->getLength()))
    {
      connectSignal ();
      emit channelReset	();
      // ### debug("emit checkMenu(bits(8), true);\n");
      // ### emit checkMenu("bits(8)", true);
    }
}
//****************************************************************************
void SignalWidget::setSignal  (const char *filename,int type)
{
  if (signalmanage) delete signalmanage;  //get rid of old signal
  signalmanage=new SignalManager (this,filename,type);
  labels->clear ();

  if (signalmanage)
    {
      offset=0;

      connectSignal ();
      setRange (0,0);

      emit channelReset	();
      // ### emit checkMenu("bits(8)", true);
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
  if (signalmanage) this->zoom=(((double) signalmanage->getLength())/width)*(zoom/100);
  else this->zoom=1.0;
  
  if (offset+zoom*width>signalmanage->getLength())
    zoom=((double)signalmanage->getLength()-offset)/width;

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
      int rate=signalmanage->getRate();
      int length=signalmanage->getLength();

      if (rate==0)
	{
	  debug("SignalWidget::refresh:rate==0");
	  // return; // ###
	}

      select->setOffset (offset);
      select->setLength (length);
      select->setZoom (zoom);

      if (rate) emit timeInfo((int)(((long long)(length))*10000/rate));
      if (rate) emit rateInfo (rate);
      emit lengthInfo (length);
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
      setRange (select->getLeft(),select->getRight(),false);
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
  debug("SignalWidget::drawOverviewSignal(channel=%d,begin=%d,height=%d)",
    channel, begin, height); // ###
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
  debug("SignalWidget::drawOvdrawOverviewSignalerviewSignal(channel=%d,begin=%d,height=%d)",
    channel, begin, height); // ###
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
	      int channels=signalmanage->getChannelCount ();
	      int chanheight=height/channels;
	      int begin=+chanheight/2;

	      //check and correction of current begin if needed
	      if (zoom*width > signalmanage->getLength() + begin)
		{
/* ###
	the check above does not work. please correct and insert some clever code here...
*/
		}

	      //check and correction of current zoom value if needed
	      if (zoom*width>signalmanage->getLength())
		{
		  zoom=((double)signalmanage->getLength ())/width;
		  select->setZoom (zoom); //notify selection
		}
	      for (int i=0;i<channels;i++)
		{
		  if (!signalmanage->getSignal(i)) continue; // skip non-existent signals
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
	  for (act=labels->first();act;act=labels->next())
	    {
	      int pos=mstosamples(act->pos);
	      if ((pos>=offset)&&(pos<lastpos))
		{
		  int x=(int)((pos-offset)/zoom);
		  //		  printf ("%d %d %d %d\n",x,pos,offset,lastpos);
		  p.setPen (*(act->getType()->color));
		  p.drawLine (x,0,x,height);

		  if (act->getName())
		    {
		      int w=p.fontMetrics().width (act->getName());
		      int h=8;
		      h=p.fontMetrics().height();

		      p.fillRect (x-w/2-1,1,w+2,h+2,QBrush(gray));
		      p.setPen (white);
		      p.drawLine (x-w/2-2,1,x+w/2+1,1);
		      p.drawLine (x-w/2-2,1,x-w/2-2,1+h);
		      p.setPen (black);
		      p.drawLine (x+w/2+1,1,x+w/2+1,1+h);
		      p.drawText (x-w/2,3,w,h,AlignCenter,act->getName());
		    }
		}
	    }
	}
      else
	{	//no full repaint needed...
	  //only the playing marker or the range labels gets updated
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











