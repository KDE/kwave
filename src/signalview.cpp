//methods for SignalWidget the view for MSignal objects.
//methods concerning markers may be found in markers.cpp
#include <qobject.h>
#include <qpainter.h>
#include <qcursor.h>
#include <math.h>
#include <limits.h>
#include "signalview.h"
#include "dialogs.h"

#define SCROLLLEFT	 8000
#define SCROLLRIGHT	 8001
#define NEXTPAGE	 8002
#define PREVPAGE	 8003
#define ZOOMIN	         8004
#define ZOOMOUT  	 8005
#define ZOOMRANGE	 8006
#define ZOOMFULL	 8007

KWaveMenuItem edit_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut

  {0              ,"&Edit"              ,KMENU ,-1   ,KEXCLUSIVE},
  {CUT            ,"Cu&t"               ,KITEM ,-1   ,CTRL+Key_X},
  {COPY           ,"&Copy"              ,KITEM ,-1   ,CTRL+Key_C},
  {PASTE          ,"&Paste"             ,KITEM ,-1   ,CTRL+Key_V},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"&Selection"         ,KMENU ,-1   ,KEXCLUSIVE},
  {SELECTALL      ,"&All"               ,KITEM ,-1   ,CTRL+Key_A},  
  {SELECTRANGE    ,"&Range"             ,KITEM ,-1   ,Key_R},  
  {SELECTVISIBLE  ,"&Visible area"      ,KITEM ,-1   ,Key_V},  
  {JUMPTOLABEL    ,"&Expand to labels"  ,KITEM ,-1   ,Key_E},  
  {SELECTNEXT     ,"&Next"              ,KITEM ,-1   ,ALT+Key_Plus},  
  {SELECTPREV     ,"&Previous"          ,KITEM ,-1   ,ALT+Key_Minus},  
  {SELECTNONE     ,"N&othing"           ,KITEM ,-1   ,Key_N},  
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};

KWaveMenuItem view_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut

  {0              ,"&View"              ,KMENU ,-1   ,KEXCLUSIVE},
  {NEXTPAGE       ,"&Next page"         ,KITEM ,-1   ,Key_PageDown},
  {PREVPAGE       ,"&previous page"     ,KITEM ,-1   ,Key_PageUp},
  {SCROLLRIGHT    ,"Scroll &right"      ,KITEM ,-1   ,Key_Right},
  {SCROLLLEFT     ,"Scroll &left"       ,KITEM ,-1   ,Key_Left},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {ZOOMIN         ,"Zoom &In"           ,KITEM ,-1   ,CTRL+Key_Plus},
  {ZOOMOUT        ,"Zoom &out"          ,KITEM ,-1   ,CTRL+Key_Minus},
  {ZOOMRANGE      ,"Zoom to Range"      ,KITEM ,-1   ,CTRL+Key_Space},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};

KWaveMenuItem marker_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut

  {0              ,"&Labels"            ,KMENU ,-1   ,KEXCLUSIVE},
  {ADDMARK        ,"&Add"               ,KITEM ,-1   ,Key_A},
  {DELETEMARK     ,"&Delete"            ,KITEM ,-1   ,Key_D},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"&Generate"          ,KMENU ,-1   ,KEXCLUSIVE},
  {MARKSIGNAL     ,"&Signal labels"     ,KITEM ,-1   ,-1},
  {MARKPERIOD     ,"&Period labels"     ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {LOADMARK       ,"&Load"              ,KITEM ,-1   ,-1},
  {APPENDMARK     ,"&Insert"            ,KITEM ,-1   ,-1},

  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"&Save"              ,KMENU ,-1   ,KEXCLUSIVE},
  {SAVEMARK       ,"&Labels"            ,KITEM ,-1   ,-1},
  {SAVEPERIODS    ,"&Periods"           ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {TOPITCH        ,"&Convert to pitch"  ,KITEM ,-1   ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"C&hange type"       ,KCHECK,-1   ,-1},
  {SELECTMARK     ,"MarkerTypes"        ,KREF  ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {ADDMARKTYPE    ,"Create &type"       ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};
extern QList<MarkerType> markertypes;
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
SignalWidget::SignalWidget (QWidget *parent,MenuManager *manage) : QWidget (parent)
{
  playing=false;
  redraw=false;

  this->manage=manage;

  manage->addNumberedMenu ("MarkerTypes");

  for (MarkerType *tmp=markertypes.first();tmp!=0;tmp=markertypes.next())
    manage->addNumberedMenuEntry ("MarkerTypes",tmp->name->data());

  manage->appendMenus (edit_menus);
  manage->appendMenus (view_menus);
  manage->appendMenus (marker_menus);

  timer=0;
  signal=0;
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

  markertype=markertypes.first();

  setBackgroundColor (black);

  setMouseTracking (true);

  connect (this,SIGNAL(channelReset()),this->parent(),SLOT(resetChannels()));
}
//****************************************************************************
SignalWidget::~SignalWidget ()
{
	if (pixmap==0)	delete pixmap;
	if (signal!=0)	delete signal;
	if (markers)    delete markers;
}
//****************************************************************************
void SignalWidget::saveSignal  (QString *filename,int bit,int selection)
{
  if (signal) signal->save (filename,bit,selection);
}
//****************************************************************************
unsigned char *SignalWidget::getOverview (int size)
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
void SignalWidget::toggleChannel (int channel)
{
  if (signal) signal->toggleChannel (0,channel);
}
//****************************************************************************
void SignalWidget::setOp (int op)
  //this one hopefully catches all functions from mainwidget and topwidget,
  //that should not be delivered to MSignal, but otherwise MSignal should
  //ignore them anyway
{
  if (signal)
    {
      if (manage)
	{
	  op=manage->translateId (marker_menus,op);
	  op=manage->translateId (edit_menus,op);
	  op=manage->translateId (view_menus,op);
	}

      signal->setOp (op);

      if (op>=SELECTMARK&&op<SELECTMARK+MENUMAX)
	markertype=markertypes.at(op-SELECTMARK); //set active Markertype

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
            int lmarker=signal->getLMarker(), rmarker=signal->getRMarker();
            lmarker = signal->getPlayPosition();
            if (rmarker<lmarker) rmarker = lmarker;
            setRange(lmarker, rmarker);
            playpointer=-1;
            break;
          }
	case DELETECHANNEL:
	  {
	    //in case the first channels is to be deleted, this routine is
	    //to be used
	    MSignal *next=signal->getNext();
	    int channels=signal->getChannels();
	    signal->detachChannels();
	    delete signal;
	    signal=next;

	    if (signal)
	      {
		signal->setChannels (channels-1);
		//set number of channels to new value
		connectSignal ();

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
		int len=dialog.getLength();
		int siglen=signal->getLength();
		if ((l+len)>siglen) setRange (l,siglen); //overflow check
		else
		  setRange (l,l+len);
	      }
	    break;
	  }
	}
    }
  if (op==NEW) createSignal ();
}
//****************************************************************************
void SignalWidget::connectSignal ()
{
  connect (signal,SIGNAL(sampleChanged()),
	   this,SLOT(refresh()));
  connect (signal,SIGNAL(signalinserted(int,int)),
	   this,SLOT(signalinserted(int,int)));
  connect (signal,SIGNAL(signaldeleted(int,int)),this,
	   SLOT(signaldeleted(int,int)));
  connect (signal,SIGNAL(channelReset()),
	   parent(),SLOT(resetChannels()));
  connect (signal,SIGNAL(selectedTimeInfo(int)),
	   parent(),SLOT(setSelectedTimeInfo(int)));

  connect (signal,SIGNAL(channelInfo(int)),
	   parent(),SLOT(getChannelInfo(int)));

  connect (signal,SIGNAL(rateInfo(int)),parent(),
	   SLOT(setRateInfo(int)));
  connect (signal,SIGNAL(lengthInfo(int)),parent(),
	   SLOT(setLengthInfo(int)));
  connect (signal,SIGNAL(timeInfo(int)),parent(),
	   SLOT(setTimeInfo( int)));
  connect (select,SIGNAL(selection(int,int)),signal,
	   SLOT(setMarkers( int,int)));

  signal->info ();
}
//****************************************************************************
void SignalWidget::createSignal ()
{
  NewSampleDialog *dialog=new NewSampleDialog (this);
  if (dialog->exec())
    {
      if (signal) delete signal;
      markers->clear ();	  

      int rate=dialog->getRate();
      int numsamples=dialog->getLength();

      signal=new MSignal (this,manage,numsamples,rate);

      if ((signal)&&(signal->getLength())) 
	{
	  connectSignal ();
	  emit	channelReset	();
	  setZoom (100);
	  refresh ();
	}
    }
}
//****************************************************************************
void SignalWidget::setRange  (int l,int r)
{
  select->set (((l-offset)/zoom),((r-offset)/zoom));

  if (signal) signal->setMarkers (l,r);
}
//****************************************************************************
void SignalWidget::setSignal  (MSignal *sig)
{
  if (signal) delete signal;
  markers->clear ();
  signal=sig;
  signal->setParent (this);
  offset=0;
  if ((signal)&&(signal->getLength()))
    {
      connectSignal ();
      emit channelReset	();
    }
}
//****************************************************************************
void SignalWidget::setSignal  (QString *filename,int type)
{
  if (signal) delete signal;  //get rid of old signal
  signal=new MSignal (this,manage,filename,1,type);
  markers->clear ();

  if ((signal)&&(signal->getLength()))
    {
      offset=0;

      connectSignal ();
      signal->setMarkers (0,0);

      emit	channelReset	();
    }
}
//****************************************************************************
void SignalWidget::time ()
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
void SignalWidget::setZoom (double zoom)
{
  if (signal) this->zoom=(((double) signal->getLength())/width)*zoom/100;
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
void SignalWidget::zoomOut ()
{
  offset-=int (zoom*width);
  if (offset<0) offset=0;
  zoom*=3;

  if (offset+zoom*width>signal->getLength())
    zoom=((double)signal->getLength()-offset)/width;

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
void SignalWidget::refresh()
{
  if (signal)
    {
      select->setOffset (offset);
      select->setLength (signal->getLength());
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
  if (signal)
    return select->checkPosition (x,width/20);      //5 % of width tolerance

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
	      select->grep (x);
	      down= true;
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
void SignalWidget::drawOverviewSignal (int *sam,int begin, int height)
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
void SignalWidget::drawInterpolatedSignal (int *sam,int begin, int height)
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

	  if ((signal)&&(signal->getLength()))
	    {
	      MSignal *tmp=signal;

	      int chanheight=height/signal->getChannels ();
	      int begin=+chanheight/2;


	      //check and correction of current zoom value if needed
	      if (zoom*width>signal->getLength())
		{
		  zoom=((double)signal->getLength ())/width;
		  select->setZoom (zoom); //notify selection
		}

	      while (tmp!=0)
		{
		  int *sam=tmp->getSample();

		  if (sam)
		    {
		      if (tmp->getLockState()>=0) //signal is not changed meanwhile 
			{
			  if (zoom<=1)
			    drawInterpolatedSignal(sam,begin,chanheight);
			  else if (zoom>1)
			    drawOverviewSignal(sam,begin,chanheight);
			}
		      p.setPen (green);
		      p.drawLine (0,begin,width,begin);
		      p.setPen (white);
		      begin+=chanheight;
		      tmp=tmp->getNext();
		    }
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
}

















