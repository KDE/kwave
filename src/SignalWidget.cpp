//methods for SignalWidget the view for MSignal objects.
//methods concerning markers may be found in markers.cpp

#include <math.h>
#include <qobject.h>
#include <qtimer.h>
#include <qfiledlg.h>

#include <kmsgbox.h>
#include <kapp.h>

#include <libkwave/Label.h>
#include <libkwave/LabelList.h>
#include <libkwave/Parser.h>
#include <libkwave/Global.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/Signal.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/MessagePort.h>
#include <libkwave/Color.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"

#include "SignalWidget.h"
#include "SignalManager.h"
#include "MouseMark.h"
#include "ProgressDialog.h"

#include "sampleop.h"

extern Global globals;
//****************************************************************************
ProgressDialog *createProgressDialog (TimeOperation *operation,const char *caption)
{
  ProgressDialog *dialog=new ProgressDialog (operation,caption);
  if (dialog)
    {
      dialog->show ();
      return dialog;
    }
  return 0;
}
//****************************************************************************
SignalWidget::SignalWidget (QWidget *parent,MenuManager *manage) : QWidget (parent)
{
  playing=false;
  redraw=false;

  this->manage=manage;

  manage->clearNumberedMenu("ID_LABELS_TYPE");
  for (LabelType *tmp=globals.markertypes.first();tmp;tmp=globals.markertypes.next())
    manage->addNumberedMenuEntry ("ID_LABELS_TYPE", (char *)tmp->name);

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

  labels=new LabelList;
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
void SignalWidget::saveSignal(const char *filename, int bits,
                              int type, bool selection)
{
    if (!signalmanage) return;
    if (type == ASCII) {
	signalmanage->exportAscii(filename);
    } else {
	signalmanage->save(filename, bits, selection);
    }
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
      Parser parser (str);
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
      Parser parser (str);
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
      Dialog *dialog=DynamicLoader::getDialog (name,operation);
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

      Dialog *dialog = DynamicLoader::getDialog
	("time",new DialogOperation (rate,true));
      if ((dialog)&&(!dialog->exec()))
	{
	  int l=signalmanage->getLMarker();

	  Parser parser (dialog->getCommand());
  
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

  signalmanage->refresh();
}
//****************************************************************************
int SignalWidget::getSignalCount ()
{
  return (signalmanage) ? signalmanage->getChannelCount() : 0;
}
//****************************************************************************
int SignalWidget::getBitsPerSample ()
{
  return (signalmanage) ? signalmanage->getBitsPerSample() : 0;
}
//****************************************************************************
void SignalWidget::createSignal (const char *str)
{
  Parser parser (str);

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
      if (x>width) x=width; //check for some bounds
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
      if (x>width) x=width; //check for some bounds
      if (x<0) x=0;
      select->update (x);
    }
  else 
    //yes, this code gives the nifty cursor change....
    if (checkPosition (e->pos().x())) setCursor (sizeHorCursor);
    else  setCursor (arrowCursor);
}

//****************************************************************************
void SignalWidget::drawOverviewSignal (int channel,int middle, int height,
	int first, int last)
{
    int step,max=0,min=0;

    int div=65536;
    div=(int)((double)div/zoomy); // == 65536/zoomy
//  int scale_y = (int)(height*zoomy);

    for (int i=0;i<width;i++) {
	step=((int) (((double)i)*zoom))+offset;

	signalmanage->getMaxMin (channel,max,min,step,(int) zoom+2);

	max=(max/256)*height/div;
	min=(min/256)*height/div;

//      max=((max>>16)*scale_y) >> 8;
//      min=((min>>16)*scale_y) >> 8;

	p.drawLine (i,middle+max,i,middle+min);
    }
}

//****************************************************************************
void SignalWidget::drawInterpolatedSignal (int channel,int begin, int height)
{
//  debug("SignalWidget::drawInterpolatedSignal(channel=%d,begin=%d,height=%d)",
//    channel, begin, height); // ###
  double f;
  int 	j,lx,x,x1,x2;
  int div=65536;

  div=(int)((double)div/zoomy);

  lx=begin+(signalmanage->getSingleSample(channel,offset)/256)*height/div;
  for (int i=0; i<width; i++)
    {
      j=(int) floor((double) i*zoom);
      f=(i*zoom)-j;
			
      x1=(signalmanage->getSingleSample(channel,offset+j)/256)*height/div;
      x2=(signalmanage->getSingleSample(channel,offset+j+1)/256)*height/div;

      x=begin+(int)(x2*f+x1*(1-f));

      p.drawLine (i,lx,i+1,x);
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

  debug("width=%d, height=%d", width, height); // ###

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
	      if ((int)(offset+zoom*width) > signalmanage->getLength())
		{
		  offset-=(int)(offset+zoom*width-signalmanage->getLength());
		  if (offset < 0) offset=0;
		}

	      //check and correction of current zoom value if needed
	      if ((int)(zoom*width) > signalmanage->getLength())
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
		    drawOverviewSignal (i,begin,chanheight,0,zoom*width);

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
	  Label *act;
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

//below are the methods of class SignalWidget that deal with labels

#define	AUTOKORRWIN 320 
//windowsize for autocorellation, propably a little bit too short for
//lower frequencies, but this will get configurable somewhere in another
//dimension or for those of you who can't zap to other dimensions, it will
//be done in future

int findNextRepeat       (int *,int);
int findNextRepeatOctave (int *,int,double =1.005);
int findFirstMark  (int *,int);

float autotable  [AUTOKORRWIN];
float weighttable[AUTOKORRWIN];

int SignalWidget::mstosamples (double ms)
{
  return   (int)(ms*signalmanage->getRate()/1000);
}
//****************************************************************************
void selectMarkers (const char *command)
{
  Parser parser(command);
} 
//****************************************************************************
LabelType *findMarkerType (const char *txt)
{
  int cnt=0;
  LabelType *act;

  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
    {
      if (strcmp (act->name,txt)==0) return act;
      cnt++;
    }
  debug ("could not find Labeltype %s\n",txt);
  return 0;
}
//****************************************************************************
void SignalWidget::signalinserted (int start, int len)
{
  Label *tmp;
  for (tmp=labels->first();tmp;tmp=labels->next()) 
      if (tmp->pos>start) tmp->pos+=len;
  setRange (start,start+len); 
  refresh ();
}
//****************************************************************************
void SignalWidget::signaldeleted (int start, int len)
{
  Label *tmp;
  for (tmp=labels->first();tmp;tmp=labels->next())
    {
      if ((tmp->pos>start)&&(tmp->pos<start+len)) //if Label position is within selected boundaries
	{
	  labels->remove ();
	  tmp=labels->first();
	}
      if (tmp->pos>=start+len) tmp->pos-=len;  //if it is greater correct position
    }
  setRange (start,start); 
  refresh ();
}
//****************************************************************************
void SignalWidget::deleteLabel ()
{
  if (signalmanage)
    {
      Label *tmp;
      int l=signalmanage->getLMarker();
      int r=signalmanage->getRMarker();

      for (tmp=labels->first();tmp;tmp=labels->next()) 
	{
	  int pos=mstosamples (tmp->pos);
	  if ((pos>=l)&&(pos<r))
	    {
	      labels->remove (tmp);
	      tmp=labels->first();
	    }
	}
      refresh ();
    }
}
//****************************************************************************
void SignalWidget::loadLabel ()
{
  labels->clear(); //remove old Label...

  appendLabel ();
}
//****************************************************************************
void SignalWidget::appendLabel ()
{
  QString name=QFileDialog::getOpenFileName (0,"*.label",this);
  if (!name.isNull())
    {
      char *comstr=catString ("loadbatch (",name,")");
      globals.port->putMessage (comstr);
    }
  refresh ();
}
//****************************************************************************
void SignalWidget::saveLabel (const char *typestring)
{
  QString name=QFileDialog::getSaveFileName (0,"*.label",this);
  if (!name.isNull())
    {
      FILE *out;
      out=fopen (name.data(),"w");

      Parser parser (typestring);
      Label     *tmp;
      LabelType *act;

      const char *actstring=parser.getFirstParam();

      while (actstring)
	{
	  printf ("selecting %s\n",actstring);
	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	    if (strcmp(act->name,actstring)==0)
	      {
		printf ("selected\n");
		act->selected=true;
		break;
	      }
	  actstring=parser.getNextParam();
	}

      for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	//write out all selected label types
	if (act->selected)
	  fprintf (out,"%s\n",act->getCommand());

      //ended writing of types, so go on with the labels...

      for (tmp=labels->first();tmp;tmp=labels->next())  //write out labels
	{
	  fprintf (out,tmp->getCommand());
	  fprintf (out,"\n");
	}

      fclose (out);
    }
}
//****************************************************************************
void SignalWidget::addLabel (const char *params)
{
  if (signalmanage&&markertype)
    {
      Parser parser(params);
      Label *newmark;

      if (parser.countParams()>0)
	{
	  newmark=new Label (params);
	}
      else
	{
	  double pos=((double)signalmanage->getLMarker())*1000/signalmanage->getRate();
	  newmark=new Label (pos,markertype);

	  //should it need a name ?
	  if (markertype->named)
	    {
	      Dialog *dialog =
		DynamicLoader::getDialog ("stringenter",new DialogOperation("Enter name of label :",true)); //create a modal dialog
	      
	      if (dialog)
		{
		  dialog->show ();
	
		  if (dialog->result())
		    {
		      printf ("dialog:%s\n",dialog->getCommand());
		      newmark->setName (dialog->getCommand());
		      delete dialog;
		    }
		  else
		    {
		      delete newmark;
		      newmark=0;
		    }
		}
	      else
		{
		  KMsgBox::message (this,"Error",i18n("Dialog not loaded !"));
		  delete newmark;
		  newmark=0;
		}
	    }
	}

      if (newmark)
	{
	  labels->inSort (newmark);
      
	  refresh();
	}
    }
}
//****************************************************************************
void SignalWidget::jumptoLabel ()
// another fine function contributed by Gerhard Zintel
// if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// rmarker jumps to next higher label or end
{
  if (signalmanage)
    {
      int lmarker=signalmanage->getLMarker(), rmarker=signalmanage->getRMarker();
      bool RangeSelected = (rmarker - lmarker) > 0;
      if (labels)
      {
        Label *tmp;
	int position = 0;
	for (tmp=labels->first();tmp;tmp=labels->next())
	  if (RangeSelected) {
	    if (tmp->pos < lmarker)
	      if (abs(lmarker-position)>abs(lmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
	}
	else if (abs(lmarker-position)>abs(lmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
	lmarker = position;
	position = signalmanage->getLength();
	for (tmp=labels->first();tmp;tmp=labels->next())
	  if (tmp->pos > rmarker)
	    if (abs(rmarker-position)>abs(rmarker-mstosamples(tmp->pos))) position = mstosamples(tmp->pos);
	rmarker = position;
	if (RangeSelected) setRange (lmarker,rmarker);
	else setRange (lmarker,lmarker);
	refresh ();
      }
    }
}   
//****************************************************************************
void SignalWidget::savePeriods ()
{
  if (signalmanage)
    {
      Dialog *dialog =
	DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  selectMarkers (dialog->getCommand());

	  LabelType *act;
	  Label *tmp;
	  int last=0;
	  int rate=signalmanage->getRate ();

	  QString name=QFileDialog::getSaveFileName (0,"*.dat",this);
	  if (!name.isNull())
	    {
	      QFile out(name.data());
	      char buf[160];
	      float freq=0,time,lastfreq=0;
	      out.open (IO_WriteOnly);
	      int first=true;

	      for (act=globals.markertypes.first();act;act=globals.markertypes.next())
		//write only selected label type
		if (act->selected)
		  //traverse list of all labels
		  for (tmp=labels->first();tmp;tmp=labels->next())
		    {
		      if (tmp->getType()==act)
			{
			  freq=tmp->pos-last;
			  time=last*1000/rate;

			  if ((!first)&&(freq!=lastfreq))
			    {
			      lastfreq=freq;
			      freq=1/(freq/rate);
			      sprintf (buf,"%f %f\n",time,freq);
			      out.writeBlock (&buf[0],strlen(buf));
			    }
			  else lastfreq=freq;
			  first=false;
			  last=mstosamples(tmp->pos);
			}
		    }

	      if (!first) //make sure last tone gets its length
		{
		  time=last*1000/rate;
		  sprintf (buf,"%f %f\n",time,freq);
		  out.writeBlock (&buf[0],strlen(buf));
		}

	      out.close ();
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::saveBlocks (int bit)
{
    if (signalmanage)
    {
      Dialog *dialog =
	DynamicLoader::getDialog ("saveblock",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  Parser parser (dialog->getCommand());

	  const char *filename=parser.getFirstParam();
	  QDir *savedir=new QDir (parser.getNextParam());

	  LabelType *start=findMarkerType(parser.getNextParam());
	  LabelType *stop=findMarkerType (parser.getNextParam());
	  
	  Label *tmp;
	  Label *tmp2;
	  int count=0;
	  int l=signalmanage->getLMarker(); //save old marker positions...
	  int r=signalmanage->getRMarker(); //

	  for (tmp=labels->first();tmp;tmp=labels->next())  //traverse list of labels
	    {
	      if (tmp->getType()==start)
		{
		  for (tmp2=tmp;tmp2;tmp2=labels->next())  //traverse rest of list to find next stop marker
		    if (tmp2->getType()==stop)
		      {
			char buf[128];
			sprintf (buf,"%s%04d.wav",filename,count);
			//lets hope noone tries to save more than 10000 blocks...

			signalmanage->setRange (tmp->pos,tmp2->pos); //changes don't have to be visible...
			filename=savedir->absFilePath(buf);
			signalmanage->save (filename,bit,true);  //save selected range...
			count++;
			break;
		      }
		}
	    }
	  signalmanage->setRange (l,r);
	}
    }
}
//****************************************************************************
void SignalWidget::markSignal (const char *str)
{
  if (signalmanage)
    {
      Label *newmark;

      Parser parser (str);
	  
      int level=(int) (parser.toDouble()/100*(1<<23));

      int len=signalmanage->getLength();
      int *sam=signalmanage->getSignal()->getSample();
      LabelType *start=findMarkerType(parser.getNextParam());
      LabelType *stop=findMarkerType (parser.getNextParam());
      int time=(int) (parser.toDouble ()*signalmanage->getRate()/1000);

      printf ("%d %d\n",level,time);
      printf ("%s %s\n",start->name,stop->name);

      ProgressDialog *dialog=
	new ProgressDialog (len,"Searching for Signal portions...");

      if (dialog&&start&&stop)
	{
	  dialog->show();

	  newmark=new Label(0,start);  //generate initial Label

	  labels->inSort (newmark);

	  for (int i=0;i<len;i++)
	    {
	      if (abs(sam[i])<level)
		{
		  int j=i;
		  while ((i<len) &&(abs(sam[i])<level)) i++;

		  if (i-j>time)
		    {
		      //insert labels...
		      newmark=new Label(i,start);
		      labels->inSort (newmark);

		      if (start!=stop)
			{
			  newmark=new Label(j,stop);
			  labels->inSort (newmark);
			}
		    }
		}
	      dialog->setProgress (i);
	    }

	  newmark=new Label(len-1,stop);
	  labels->inSort (newmark);

	  refresh ();
	  delete dialog;
	}
    }
}
//****************************************************************************
void SignalWidget::markPeriods (const char *str)
{
  if (signalmanage)
    {
      Parser parser (str);

      int high   =signalmanage->getRate()/parser.toInt();
      int low    =signalmanage->getRate()/parser.toInt();
      int octave =parser.toBool ("true");
      double adjust=parser.toDouble ();

      for (int i=0;i<AUTOKORRWIN;i++)
	autotable[i]=1-(((double)i*i*i)/(AUTOKORRWIN*AUTOKORRWIN*AUTOKORRWIN)); //generate static weighting function

      if (octave) for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]=1; //initialise moving weight table

      Label *newmark;
      int next;
      int len=signalmanage->getLength();
      int *sam=signalmanage->getSignal()->getSample();
      LabelType *start=markertype;
      int cnt=findFirstMark (sam,len);

      ProgressDialog *dialog=new ProgressDialog (len-AUTOKORRWIN,"Correlating Signal to find Periods:");
      if (dialog)
	{
	  dialog->show();

	  newmark=new Label(cnt,start);
	  labels->inSort (newmark);

	  while (cnt<len-2*AUTOKORRWIN)
	    {
	      if (octave)
		next=findNextRepeatOctave (&sam[cnt],high,adjust);
	      else
		next=findNextRepeat (&sam[cnt],high);

	      if ((next<low)&&(next>high))
		{
		  newmark=new Label(cnt,start);

		  labels->inSort (newmark);
		}
	      if (next<AUTOKORRWIN) cnt+=next;
	      else
		if (cnt<len-AUTOKORRWIN)
		  {
		    int a=findFirstMark (&sam[cnt],len-cnt);
		    if (a>0) cnt+=a;
		    else cnt+=high;
		  }
		else cnt=len;

	      dialog->setProgress (cnt);
	    }

	  delete dialog;

	  refresh ();
	}
    }
}
//*****************************************************************************
int findNextRepeat (int *sample,int high)
  //autocorellation of a windowed part of the sample
  //returns length of period, if found
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]; //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  return maxpos;
} 
//*****************************************************************************
int findNextRepeatOctave (int *sample,int high,double adjust=1.005)
  //autocorellation of a windowed part of the sample
  //same as above only with an adaptive weighting to decrease fast period changes
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]*weighttable[i];
      //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  
    for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]/=adjust;

  weighttable[maxpos]=1;
  weighttable[maxpos+1]=.9;
  weighttable[maxpos-1]=.9;
  weighttable[maxpos+2]=.8;
  weighttable[maxpos-2]=.8;

  float buf[7];

  for (int i=0;i<7;buf[i++]=.1)

    //low pass filter
  for (int i=high;i<AUTOKORRWIN-3;i++)
    {
      buf[i%7]=weighttable[i+3];
       weighttable[i]=(buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6])/7;
    }

  return maxpos;
} 
//*****************************************************************************
int findFirstMark (int *sample,int len)
  //finds first sample that is non-zero, or one that preceeds a zero crossing
{
  int i=1;
  int last=sample[0];
  int act=last;
  if ((last<100)&&(last>-100)) i=0;
  else
    while (i<len)
      {
	act=sample[i];
	if ((act<0)&&(last>=0)) break;
	if ((act>0)&&(last<=0)) break;
	last=act;
	i++;
      }
  return i;
}
//*****************************************************************************
void SignalWidget::addLabelType (LabelType *marker)
{
  globals.markertypes.append (marker);
  if (manage) manage->addNumberedMenuEntry ("ID_LABELS_TYPE", (char *)marker->name);
}

//*****************************************************************************
void SignalWidget::addLabelType (const char *str)
{
  LabelType *marker=new LabelType(str);
  if (marker) addLabelType (marker);
}

/* end of src/SignalWidget.cpp */
