
#include <qkeycode.h>
#include <qframe.h>
#include <qimage.h>
#include <qaccel.h>
#include <qwidget.h>

#include <kapp.h>
#include <kbuttonbox.h>

#include <libkwave/String.h>

#include "libgui/MenuManager.h"
#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"

#include "sampleop.h"
#include "SignalWidget.h"
#include "SignalManager.h"
#include "MainWidget.h"

static const int keys[10]={Key_1,Key_2,Key_3,Key_4,Key_5,Key_6,Key_7,Key_8,Key_9,Key_0};

const char *zoomtext[]={"100 %","33 %","10 %","3 %","1 %","0.1 %"};
int         playbit;
extern int  bufbase;
int         mmap_threshold;
char*       mmap_dir;     //storage of dir name
QPixmap  *  lightoff=0;
QPixmap  *  lighton=0;
//*****************************************************************************
QString mstotime (int ms)
{
  return (QString(mstotimec(ms)));
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

  manage->clearNumberedMenu ("ID_EDIT_CHANNEL_DELETE");
  manage->addNumberedMenuEntry("ID_EDIT_CHANNEL_DELETE", "none");
  updateMenu();

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

  QObject::connect(
    lamps[0],SIGNAL(clicked(int)),
    signalview,SLOT(toggleChannel(int))
  );

  zoomselect->insertStrList (zoomtext,6);
  connect       (slider,SIGNAL(valueChanged(int)),signalview,SLOT(setOffset(int)));
  connect	(signalview,SIGNAL(viewInfo(int,int,int)),
			 slider,SLOT(setRange(int,int,int)));
  connect 	(signalview,SIGNAL(playingfinished()),
			 this,SLOT(stop()));
  connect	(zoomselect,SIGNAL(activated(int)),
			 this,SLOT(selectedZoom(int)));
  connect	(this,SIGNAL(setOperation(int)),
			 signalview,SLOT(setOp(int)));

  buttons->addStretch	();
  this->connect	(playbutton=buttons->addButton	(i18n("Play")),SIGNAL(pressed()),
		 this,SLOT(play()));
  playbutton->setAccel (Key_Space);

  this->connect	(loopbutton=buttons->addButton	(i18n("&Loop")),SIGNAL(pressed()),
		 this,SLOT(loop()));
  loopbutton->setAccel (Key_L);
  buttons->addStretch		();
  this->connect	( zoombutton=buttons->addButton (i18n("&Zoom")),SIGNAL(pressed()),
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
void MainWidget::updateMenu ()
{
  if (! manage) return;

  bool have_signal = (numsignals != 0);
  manage->setItemEnabled("@SIGNAL", have_signal);

}
//*****************************************************************************
void MainWidget::updateChannels (int cnt)
  // generates menu entries 
{
  manage->clearNumberedMenu ("ID_EDIT_CHANNEL_DELETE");
  for (int i =0 ; i < cnt; i++)
    {
      char buf[16];
      sprintf (buf,"%d",i);
      manage->addNumberedMenuEntry ("ID_EDIT_CHANNEL_DELETE",buf);
    }
}
//**********************************************************
void MainWidget::saveSignal  (const char *filename,int bits,bool selection)
{
  signalview->saveSignal (filename,bits,selection);
}
//*****************************************************************************
void MainWidget::setSignal  (const char *filename,int type)
{
  signalview->setSignal	(filename,type);
// ###  if (signal) setBitsPerSample(signal->getBitsPerSample());
  signalview->setZoom 	(100.0);
  slider->refresh();
  updateMenu();
}
//*****************************************************************************
void MainWidget::setSignal  (SignalManager *signal)
{
  signalview->setSignal	(signal);
// ###  if (signal) setBitsPerSample(signal->getBitsPerSample());
  signalview->setZoom 	(100.0);
  updateMenu();
}
//*****************************************************************************
void MainWidget::setRateInfo (int rate)
{
  char buf[64];
  sprintf (buf,i18n("Rate : %d.%d kHz"),rate/1000,(rate%1000)/100);
  status->changeItem (buf,2);
}
//*****************************************************************************
void MainWidget::setLengthInfo (int len)
{
  char buf[64];
  sprintf (buf,i18n("Samples :%d"),len);
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
  updateChannels (numsignals);
}
//*****************************************************************************
void MainWidget::parseKey  (int key)
{
  if (key<numsignals)
    {
      lamps[key]->nextState();
      emit setOperation (TOGGLECHANNEL+key);
    }
}
//*****************************************************************************
#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include "../libgui/Dialog.h"
//*****************************************************************************
int MainWidget::doCommand (const char *str)
{
  if (matchCommand (str,"refreshchannels"))
    {
      resetChannels();
      signalview->refresh();
      setChannelInfo (signalview->getSignalCount());
    }
  else
  if (matchCommand (str,"setplayback"))
    {
      Parser parser (str);
      playbit=parser.toInt();
      bufbase=parser.toInt();
    }
  else
    if (matchCommand (str,"setmemory"))
      {
	Parser parser (str);

	mmap_threshold=parser.toInt();
	mmap_dir=strdup(parser.getNextParam());
      }
    else
      {
	if (matchCommand (str,"selectchannels"))
	  for (int i=0;i<numsignals;i++) lamps[i]->setState (0);

	if (matchCommand (str,"invertchannels"))
	  for (int i=0;i<numsignals;i++) lamps[i]->nextState ();

	return signalview->doCommand (str);
      }
  return true;
}
//*****************************************************************************
void MainWidget::loop()
{
  emit setOperation	(LOOP);
  playbutton->setText	(i18n("Stop"));
  loopbutton->setText   (i18n("Halt"));  // halt feature by gerhard Zint
  this->disconnect      (playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
  this->connect         (playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->connect         (loopbutton,SIGNAL(pressed()),this,SLOT(halt()));
}
//*****************************************************************************
void MainWidget::play ()
{
  emit setOperation 	(PLAY);
  playbutton->setText	(i18n("Stop"));
  loopbutton->setText   (i18n("Halt"));  // halt feature by gerhard Zint
  this->disconnect      (playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
  this->connect         (playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->connect         (loopbutton,SIGNAL(pressed()),this,SLOT(halt())); 
}
//*****************************************************************************
void MainWidget::halt   ()
{
  playbutton->setText (i18n("Play"));
  loopbutton->setText (i18n("&Loop"));
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
  playbutton->setText (i18n("Play"));
  loopbutton->setText (i18n("&Loop"));
  loopbutton->setAccel (Key_L); //seems to be neccessary

  emit setOperation (PSTOP);

  this->disconnect	(playbutton,SIGNAL(pressed()),this,SLOT(stop()));
  this->disconnect      (loopbutton,SIGNAL(pressed()),this,SLOT(halt()));
  this->connect		(playbutton,SIGNAL(pressed()),this,SLOT(play()));
  this->connect		(loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
}
//*****************************************************************************
void MainWidget::setSelectedTimeInfo ( int ms)
{
  QString buf=i18n("selected :")+ mstotime (ms);
  status->changeItem (buf.data(),4);
}
//*****************************************************************************
void MainWidget::setTimeInfo ( int ms)
{
 QString buf=i18n("Length :")+ mstotime (ms);
 status->changeItem (buf.data(),1);
}
//*****************************************************************************
void MainWidget::setChannelInfo  (int channels)
{
  if (channels!=numsignals)
    {
      if ((!menushown)&&(channels>0))
	{
	  menushown=true;
	}
      char buf[8];
      manage->clearNumberedMenu ("ID_EDIT_CHANNEL_DELETE");

      for (int i=0;i<channels;i++)
	{
	  sprintf (buf,"%d",i);
	  manage->addNumberedMenuEntry ("ID_EDIT_CHANNEL_DELETE",buf);
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

  updateMenu();
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
//*****************************************************************************
int MainWidget::getBitsPerSample()
{
  return (signalview) ? signalview->getBitsPerSample() : 0;
}
//*****************************************************************************

