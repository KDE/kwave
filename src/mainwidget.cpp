#include "mainwidget.h"
#include "sample.h"
#include <qkeycode.h>
#include <qframe.h>
#include <qimage.h>
#include <qaccel.h>
#include "configdialogs.h"

QPixmap  *lightoff=0;
QPixmap  *lighton=0;

static const int keys[10]={Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9,Key_0};

const char  *zoomtext[]={"100 %","33 %","10 %","3 %","1 %","0.1 %"};
extern int  play16bit;
extern int  bufbase;
extern int  mmap_threshold;
extern char *mmap_dir;     //storage of dir name

KWaveMenuItem channel_menus[]=
{
  {0              ,"&Edit"              ,KMENU ,-1   ,KSHARED},
  {0              ,"&Channel"           ,KMENU ,-1   ,KEXCLUSIVE},
  {ADDCHANNEL     ,"&Add"               ,KITEM ,-1   ,SHIFT+Key_A},

  {0              ,"&Delete"            ,KMENU ,-1   ,KEXCLUSIVE},
  {DELETECHANNEL  ,"Channels"           ,KREF  ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {ALLCHANNEL     ,"&Select all"        ,KITEM ,-1   ,SHIFT+CTRL+Key_A},
  {INVERTCHANNEL  ,"&Invert Selection"  ,KITEM ,-1   ,SHIFT+Key_I},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};
//*****************************************************************************
char *mstotimec (int ms)
{
  static char buf[32];

  if (ms<10000)		sprintf (buf,"%d.%d ms",ms/10,ms%10);
  else
    {
      ms/=10;
      if (ms<60*1000)	sprintf (buf,"%d.%03d s",ms/1000,ms%1000);
      else if (ms<60*60*1000)	sprintf (buf,"%d:%02d.%02d m",
					 ms/(60*1000),			//minutes
					 (ms%(60*1000))/1000,		//seconds
					 (ms%1000)/10);			//ms
      else if (ms<24*60*60*1000)	sprintf (buf,"%d h %d:%d.%d m",
       					 ms/(60*60*1000),		//hours
	       				 ms%(60*60*1000)/(60*1000),	//minutes
		       			 (ms%(60*1000))/1000,		//seconds
			       		 (ms%1000)/10);			//ms
    }
  return (buf);
}
//*****************************************************************************
QString mstotime (int ms)
{
  return (QString(mstotimec(ms)));
}
//*****************************************************************************
OverViewWidget::OverViewWidget (QWidget *parent,const char *name)
 : QWidget (parent,name)
{
  pixmap=0;
  this->parent=parent;
  this->mparent=0;
  grabbed=false;
  timer=0;
}
//*****************************************************************************
OverViewWidget::OverViewWidget (MainWidget *parent,const char *name)
 : QWidget (parent,name)
{
  pixmap=0;
  this->parent=parent;
  this->mparent=parent;
  grabbed=false;
  timer=0;
}
//*****************************************************************************
OverViewWidget::~OverViewWidget ()
{
  if (pixmap) delete pixmap;
}
//*****************************************************************************
void OverViewWidget::mousePressEvent( QMouseEvent *e)
{
  int x1=(int)(((double)act)*width/len);
  int x2=(int)(((double)max)*width/len);
  if (x2<8) x2=8;
  if (e->x()>x1+x2)
    {
      dir=max/2;
      timer=new QTimer (this);
      connect (timer,SIGNAL(timeout()),this,SLOT(increase()));
      timer->start( 50); 
    }
  else
  if (e->x()<x1)
    {
      dir=-max/2;
      timer=new QTimer (this);
      connect (timer,SIGNAL(timeout()),this,SLOT(increase()));
      timer->start( 50); 
    }
  else grabbed=e->x()-x1;
}
//****************************************************************************
void OverViewWidget::increase()
{
  act+=dir;
  if (act<0) act=0;
  if (act>len-max) act=len-max;
  repaint (false);
  emit valueChanged (act);
}
//****************************************************************************
void OverViewWidget::mouseReleaseEvent( QMouseEvent *)
{
  grabbed=false;
  if (timer)
    {
      timer->stop();
      delete timer;
      timer=0;
    }
}
//****************************************************************************
void OverViewWidget::mouseMoveEvent( QMouseEvent *e)
{
  if (grabbed) 
    {
      int pos=e->x()-grabbed;
      if (pos <0) pos=0;
      if (pos>width) pos=width;
      act=(int)(((double) len)*pos/width);
      if (act>len-max) act=len-max;
      repaint (false);
      emit valueChanged (act);
    }
}
//*****************************************************************************
void OverViewWidget::refresh  ()
{
  redraw=true;
  repaint (false);
}
//*****************************************************************************
void OverViewWidget::setRange  (int newval,int x,int y)
{
  if ((newval!=act)||(len!=y)||(x!=max))
    {
      if ((len==y)&&(x==max))
	{
	  act=newval;
	  repaint (false);
	}
      else
	{
	  act=newval;
	  max=x;
	  len=y;
	  refresh ();
	}
    }
}
//*****************************************************************************
void OverViewWidget::setValue  (int newval)
{
  if (act!=newval)
    {
      act=newval;
      repaint (false);
    }
}
//*****************************************************************************
void OverViewWidget::paintEvent  (QPaintEvent *)
{
  QPainter p;
  ///if pixmap has to be resized ...
  if ((rect().height()!=height)||(rect().width()!=width)||redraw)
    {
      redraw=false;
      height=rect().height();
      width=rect().width();

      if (pixmap) delete pixmap;
      pixmap=new QPixmap (size());

      pixmap->fill (colorGroup().background());

      p.begin (pixmap);
      p.setPen (colorGroup().midlight());

      if (mparent)
	{
	  unsigned char *overview=mparent->getOverView(width);
	  if (overview)
	    {
	      for (int i=0;i<width;i++)
		p.drawLine (i,height-(((int)overview[i])*height)/128,i,height);
	      delete overview;
	    }
	}

      p.end ();
    }
  if (pixmap) bitBlt (this,0,0,pixmap);

  p.begin (this);

  int x1=(int)(((double)act)*width/len);
  int x2=(int)(((double)max)*width/len);
  if (x2<8) x2=8;   //so there is at least something

  p.setPen (colorGroup().light());
  p.drawLine (0,0,width,0);
  p.drawLine (0,0,0,height);

  p.drawLine (x1,0,x1+x2,0);
  p.drawLine (x1,0,x1,height);
  p.drawLine (x1+1,0,x1+1,height);
  p.setBrush (colorGroup().background());
  p.drawRect (x1,0,x2,height);
  p.setPen (colorGroup().dark());
  p.drawLine (1,height-1,width,height-1);
  p.drawLine (width-1,1,width-1,height-1);

  p.drawLine (x1+1,height-2,x1+x2,height-2);
  p.drawLine (x1+x2,1,x1+x2,height);
  p.drawLine (x1+x2-1,1,x1+x2-1,height);

  p.end ();
}
//*****************************************************************************
MainWidget::~MainWidget () 
{
  if (lamps) delete lamps;
}
//*****************************************************************************
MainWidget::MainWidget (QWidget *parent,MenuManager *manage,KStatusBar *status) :QWidget (parent)
{
  int s[3];
  numsignals=0;

  this->manage=manage;
  menushown=false;

  if (manage->addNumberedMenu ("Channels")) manage->addNumberedMenuEntry ("Channels","none");

  this->parent=parent;
  lamps=new MultiStateWidget*[1];
  speakers=new MultiStateWidget*[1];

  lamps[0]=new MultiStateWidget(this,0);
  s[0]=lamps[0]->addPixmap("light_on.xpm");
  s[1]=lamps[0]->addPixmap("light_off.xpm");
  lamps[0]->setStates(s);

  speakers[0]=new MultiStateWidget(this,0,3);
  s[0]=speakers[0]->addPixmap("rspeaker.xpm");
  s[1]=speakers[0]->addPixmap("lspeaker.xpm");
  s[2]=speakers[0]->addPixmap("xspeaker.xpm");
  speakers[0]->setStates(s);

  QAccel *key=new QAccel (this);
  for (int i=0;i<10;i++) key->insertItem (keys[i]);

  QObject::connect (key,SIGNAL(activated(int)),this,SLOT(parseKey(int)));

  slider	=new OverViewWidget (this); 
  buttons	=new KButtonBox (this,KButtonBox::HORIZONTAL);
  zoomselect	=new QComboBox	(true,this);
  signalview	=new SignalWidget (this,manage);

  QObject::connect (lamps[0],SIGNAL(clicked(int)),signalview,SLOT(toggleChannel(int)));

  zoomselect->insertStrList (zoomtext,6);
  QObject::connect      (slider,SIGNAL(valueChanged(int)),signalview,SLOT(setOffset(int)));
  QObject::connect	(signalview,SIGNAL(channelInfo(int)),
			 this,SLOT(getChannelInfo(int)));
  QObject::connect	(signalview,SIGNAL(viewInfo(int,int,int)),
			 slider,SLOT(setRange(int,int,int)));
  QObject::connect	(signalview,SIGNAL(rateInfo(int)),this,
			 SLOT(setRateInfo(int)));
  QObject::connect 	(signalview,SIGNAL(lengthInfo(int)),this,
			 SLOT(setLengthInfo(int)));
  QObject::connect 	(signalview,SIGNAL(timeInfo(int)),
			 this,SLOT(setTimeInfo( int)));
  QObject::connect 	(signalview,SIGNAL(playingfinished()),
			 this,SLOT(stop()));
  QObject::connect 	(signalview,SIGNAL(selectedtimeInfo(int)),
			 this,SLOT(setSelectedTimeInfo( int)));
  QObject::connect	(zoomselect,SIGNAL(activated(int)),
			 this,SLOT(selectedZoom(int)));
  QObject::connect	(this,SIGNAL(setOperation(int)),
			 signalview,SLOT(setOp(int)));

  buttons->addStretch	();
  this->connect	(playbutton=buttons->addButton	("Play"),SIGNAL(pressed()),
		 this,SLOT(play()));
  playbutton->setAccel (Key_Space);

  this->connect	(loopbutton=buttons->addButton	("&Loop"),SIGNAL(pressed()),
		 this,SLOT(loop()));
  loopbutton->setAccel (Key_L);
  buttons->addStretch		();
  this->connect	( zoombutton=buttons->addButton ("&Zoom"),SIGNAL(pressed()),
		  signalview,SLOT(zoomRange()));
  zoombutton->setAccel (Key_Z);
  this->connect	( plusbutton=buttons->addButton	  ("+"),SIGNAL(pressed()),
		  signalview,SLOT(zoomIn()));
  plusbutton->setAccel (Key_Plus);
  this->connect	( minusbutton=buttons->addButton  ("-"),SIGNAL(pressed()),
		  signalview,SLOT(zoomOut()));
  minusbutton->setAccel (Key_Minus);
  this->connect	( nozoombutton=buttons->addButton("1:1"),SIGNAL(pressed()),
		  signalview,SLOT(zoomNormal()));

  buttons->addStretch ();

  this->status=status;
}
//*****************************************************************************
void MainWidget::saveSignal  (QString *filename,int bit,int selection)
{
  signalview->saveSignal (filename,bit,selection);
}
//*****************************************************************************
void MainWidget::setSignal  (QString *filename,int type)
{
  signalview->setSignal	(filename,type);
  signalview->setZoom 	(100.0);
  slider->refresh();
}
//*****************************************************************************
void MainWidget::setSignal  (MSignal *signal)
{
  signal->setMenuManager (manage);
  signalview->setSignal	(signal);
  signalview->setZoom 	(100.0);
}
//*****************************************************************************
void MainWidget::setRateInfo (int rate)
{
  char buf[64];
  sprintf (buf,"Rate : %d.%d kHz",rate/1000,(rate%1000)/100);
  status->changeItem (buf,2);
}
//*****************************************************************************
void MainWidget::setLengthInfo (int len)
{
  char buf[64];
  sprintf (buf,"Samples :%d",len);
  status->changeItem (buf,3);
}
//*****************************************************************************
void MainWidget::selectedZoom (int num)
{
  if (num<6) signalview->setZoom (strtod(zoomtext[num],0));
}
//*****************************************************************************
unsigned char *MainWidget::getOverView	(int val)
{
  return signalview->getOverview(val);
}
//*****************************************************************************
void MainWidget::resetChannels	()
{
  for (int i=0;i<numsignals;i++) lamps[i]->setState (0);
}
//*****************************************************************************
void MainWidget::parseKey 	(int key)
{
  if (key<numsignals)
    {
      lamps[key]->nextState();
      emit setOperation (TOGGLECHANNEL+key);
    }
}
//*****************************************************************************
void MainWidget::setOp (int op)
{
  if (manage) op=manage->translateId (channel_menus,op);

  switch (op)
    {
    case PLAYBACKOPTIONS:
      {
	PlayBackDialog dialog (parent,play16bit,bufbase);
	if (dialog.exec())
	  {
	    play16bit=dialog.getResolution();
	    bufbase=dialog.getBufferSize();
	  }  
      break;
      }
    case MEMORYOPTIONS:
      {
	MemoryDialog dialog (parent);
	if (dialog.exec())
	  {
	    mmap_threshold=dialog.getThreshold();
	    mmap_dir=dialog.getDir();
	  }  
      break;
      }
    case ALLCHANNEL:
      for (int i=0;i<numsignals;i++) lamps[i]->setState (0);
      break;
    case INVERTCHANNEL:
      for (int i=0;i<numsignals;i++) lamps[i]->nextState ();
      break;
    }
  emit setOperation (op);
}
//*****************************************************************************
void MainWidget::loop()
{
  emit setOperation	(LOOP);
  playbutton->setText	("Stop");
  loopbutton->setText   ("Halt");  // halt feature by gerhard Zint
  this->disconnect      (playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
  this->connect         (playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->connect         (loopbutton,SIGNAL(pressed()),this,SLOT(halt()));
}
//*****************************************************************************
void MainWidget::play ()
{
  emit setOperation 	(PLAY);
  playbutton->setText	("Stop");
  loopbutton->setText   ("Halt");  // halt feature by gerhard Zint
  this->disconnect      (playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
  this->connect         (playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->connect         (loopbutton,SIGNAL(pressed()),this,SLOT(halt())); 
}
//*****************************************************************************
void MainWidget::halt   ()
{
  playbutton->setText ("Play");
  loopbutton->setText ("&Loop");
  loopbutton->setAccel (Key_L); //seems to neccessary

  emit setOperation (PHALT);

  this->disconnect        (playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->connect           (playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->disconnect        (loopbutton,SIGNAL(pressed()),this,SLOT(halt()));
  this->connect           (loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
}
//*****************************************************************************
void MainWidget::stop ()
{
  playbutton->setText ("Play");
  loopbutton->setText ("&Loop");
  loopbutton->setAccel (Key_L); //seems to neccessary

  emit setOperation (PSTOP);

  this->disconnect	(playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(halt()));
  this->connect		(playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->connect		(loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
}
//*****************************************************************************
void MainWidget::setSelectedTimeInfo ( int ms)
{
 QString buf="selected :"+ mstotime (ms);
 status->changeItem (buf.data(),4);
}
//*****************************************************************************
void MainWidget::setTimeInfo ( int ms)
{
 QString buf="Length :"+ mstotime (ms*10);
 status->changeItem (buf.data(),1);
}
//*****************************************************************************
void MainWidget::getChannelInfo  (int channels)
{
  if (channels!=numsignals)
    {
      if ((!menushown)&&(channels>0))
	{
	  manage->appendMenus (channel_menus);
	  menushown=true;
	}
      char buf[8];
      manage->clearNumberedMenu ("Channels");

      for (int i=0;i<channels;i++)
	{
	  sprintf (buf,"%d",i);
	  manage->addNumberedMenuEntry ("Channels",buf);
	}

      MultiStateWidget **newlamps=new MultiStateWidget*[channels+1];
      MultiStateWidget **newspeakers=new MultiStateWidget*[channels+1];

      if ((numsignals<channels)&&(parent->height()<(channels+4)*40))
	parent->resize (width(),(channels+4)*40);

      for (int i=0;(i<numsignals)&&(i<channels);i++)
	{
	  newlamps[i]=lamps[i];
	  newspeakers[i]=speakers[i];
	  lamps[i]->setGeometry (0,i*(height()-bsize-20)/channels,20,20);
	  speakers[i]->setGeometry (0,i*(height()-bsize-20)/channels+20,20,20);
	}

      for (int i=channels;i<numsignals;i++) delete lamps[i],delete speakers[i];
      delete lamps;
      lamps=newlamps;
      delete speakers;
      speakers=newspeakers;

      for (int i=numsignals;i<channels;i++)
	{
	  int s[3];

	  newlamps[i]=new MultiStateWidget (this,i);
	  s[0]=newlamps[i]->addPixmap("light_on.xpm");
	  s[1]=newlamps[i]->addPixmap("light_off.xpm");
	  QObject::connect (newlamps[i],SIGNAL(clicked(int)),signalview,SLOT(toggleChannel(int)));
	  newlamps[i]->setStates (s);

	  newspeakers[i]=new MultiStateWidget(this,0,3);
	  s[0]=newspeakers[i]->addPixmap("rspeaker.xpm");
	  s[1]=newspeakers[i]->addPixmap("lspeaker.xpm");
	  s[2]=newspeakers[i]->addPixmap("xspeaker.xpm");
	  newspeakers[i]->setStates(s);

	  lamps[i]->setGeometry (0,i*(height()-bsize-20)/channels,20,20);
	  speakers[i]->setGeometry (0,i*(height()-bsize-20)/channels+20,20,20);

	  newlamps[i]->show();
	  newspeakers[i]->show();
	}
      numsignals=channels;
    }
}
//*****************************************************************************
void MainWidget::resizeEvent  (QResizeEvent *)
{
 bsize=buttons->sizeHint().height();

 buttons->setGeometry (0,height()-bsize,width()*3/4,bsize);
 zoomselect->setGeometry (width()*3/4,height()-bsize,width()/4,bsize);
 slider->setGeometry (20,height()-(bsize+20),width()-20,20);
 signalview->setGeometry (20,0,width()-20,height()-(bsize+20));

 for (int i=0;i<numsignals;i++)
   {
     lamps[i]->setGeometry (0,i*(height()-bsize-20)/numsignals,20,20);
     speakers[i]->setGeometry (0,i*(height()-bsize-20)/numsignals+20,20,20);
   }
}
