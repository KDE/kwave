#include "classes.h"
#include "sample.h"
#include "qkeycode.h"

const char *zoomtext[]={"100 %","33 %","10 %","3 %","1 %","0.1 %"};
//*****************************************************************************************
QString mstotime (int ms)
{
 char buf[32];

 if (ms<10000)		sprintf (buf,"%d.%d ms",ms/10,ms%10);
 else
  {
	ms/=10;
 if (ms<60*1000)	sprintf (buf,"%d.%03d s",ms/1000,ms%1000);
 else if (ms<60*60*1000)	sprintf (buf,"%d:%d.%d m",
				ms/(60*1000),			//minutes
				(ms%(60*1000))/1000,		//seconds
				(ms%1000)/10);			//ms
 else if (ms<24*60*60*1000)	sprintf (buf,"%d h %d:%d.%d m",
				ms/(60*60*1000),		//hours
				ms%(60*60*1000)/(60*1000),	//minutes
				(ms%(60*1000))/1000,		//seconds
				(ms%1000)/10);			//ms
}
 return (QString(buf));
}
//*****************************************************************************************
MainWidget::MainWidget (QWidget *parent=0,const char *name,KStatusBar *status) :
QWidget (parent,name)
{
 setMinimumSize (320,100);

 signalviews= new QList<SigWidget>;
 signalviews->setAutoDelete (TRUE);

 slider		=new QScrollBar (1,1,1,10,0,QScrollBar::Horizontal,this,"mainscroller"); 
 buttons	=new KButtonBox (this,KButtonBox::HORIZONTAL);
 zoomselect	=new QComboBox	(true,this,"ZoomComboBox");
 signalview	=new SigWidget	(this,"signalView");

 signalviews->append (signalview);

 signalview->setBackgroundColor	(QColor(black) );
 zoomselect->insertStrList (zoomtext,6);
 QObject::connect (slider,SIGNAL(valueChanged(int)),signalview,SLOT(setOffset(int)));
 QObject::connect	(signalview,SIGNAL(viewInfo(int,int,int)),
			this,SLOT(getSliderInfo(int,int,int)));
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
			signalview,SLOT(setRangeOp(int)));

 buttons->addStretch	();
 this->connect	(playbutton=buttons->addButton	("&Play"),SIGNAL(pressed()),
			  this,SLOT(play()));
 playbutton->setAccel (Key_Space);
 this->connect	(loopbutton=buttons->addButton	("&Loop"),SIGNAL(pressed()),
			  this,SLOT(loop()));
 buttons->addStretch		();
 this->connect	( zoombutton=buttons->addButton	  ("Zoom"),SIGNAL(pressed()),
			  signalview,SLOT(zoomRange()));
 this->connect	( plusbutton=buttons->addButton	  ("+"),SIGNAL(pressed()),
			  signalview,SLOT(zoomIn()));
 this->connect	( minusbutton=buttons->addButton  ("-"),SIGNAL(pressed()),
			  signalview,SLOT(zoomOut()));
 this->connect	( nozoombutton=buttons->addButton("1:1"),SIGNAL(pressed()),
			  signalview,SLOT(zoomNormal()));

 buttons->addStretch		();

 this->status=status;
}
//*****************************************************************************************
void MainWidget::saveSignal  (QString *filename)
{
  signalview->saveSignal (filename);
}
//*****************************************************************************************
void MainWidget::saveSelectedSignal  (QString *filename)
{
  filename[1]=0;
}
//*****************************************************************************************
void MainWidget::setSignal  (QString *filename)
{
 signalview->setSignal	(filename);
 signalview->setZoom 	(100);
}
//*****************************************************************************************
void MainWidget::setSignal  (MSignal *signal)
{
 signalview->setSignal	(signal);
 signalview->setZoom 	(100);
}
//*****************************************************************************************
void MainWidget::setRateInfo (int rate)
{
 char buf[64];
 sprintf (buf,"Rate : %d.%d kHz",rate/1000,(rate%1000)/100);
 status->changeItem (buf,2);
}
//*****************************************************************************************
void MainWidget::setLengthInfo (int len)
{
 char buf[64];
 sprintf (buf,"Samples :%d",len);
 status->changeItem (buf,3);
}
//*****************************************************************************************
void MainWidget::selectedZoom (int num)
{
 if (num<6) signalview->setZoom (strtod(zoomtext[num],0));
}
//*****************************************************************************************
void MainWidget::setRangeOp 	(int op)
{
  emit setOperation (op);
}
//*****************************************************************************************
void MainWidget::loop		()
 {
	emit setOperation	(LOOP);
	loopbutton->setText	("Stop");
	playbutton->setText	("Stop");
	this->disconnect	(playbutton,SIGNAL(pressed()),this,SLOT(play()));
	this->connect		(playbutton,SIGNAL(pressed()),this,SLOT(stop()));
	this->disconnect	(loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
	this->connect		(loopbutton,SIGNAL(pressed()),this,SLOT(stop()));
 }
//*****************************************************************************************
void MainWidget::play		()
{
	emit setOperation 	(PLAY);
	playbutton->setText	("Stop");
	loopbutton->setText	("Stop");
	this->disconnect	(playbutton,SIGNAL(pressed()),this,SLOT(play()));
	this->connect		(playbutton,SIGNAL(pressed()),this,SLOT(stop()));
	this->disconnect	(loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
	this->connect		(loopbutton,SIGNAL(pressed()),this,SLOT(stop()));
}
//*****************************************************************************************
void MainWidget::stop	()
{
	playbutton->setText ("Play");
	loopbutton->setText ("Loop");

	emit setOperation (PSTOP);

	this->disconnect	(playbutton,SIGNAL(pressed()),this,SLOT(stop()));
	this->connect		(playbutton,SIGNAL(pressed()),this,SLOT(play()));
	this->disconnect	(loopbutton,SIGNAL(pressed()),this,SLOT(stop()));
	this->connect		(loopbutton,SIGNAL(pressed()),this,SLOT(loop()));
}
//*****************************************************************************************
void MainWidget::setSelectedTimeInfo ( int ms)
{
 QString buf="selected :"+ mstotime (ms);
 status->changeItem (buf.data(),4);
}
//*****************************************************************************************
void MainWidget::setTimeInfo ( int ms)
{
 QString buf="Length :"+ mstotime (ms*10);
 status->changeItem (buf.data(),1);
}
//*****************************************************************************************
void MainWidget::getSliderInfo  (int offset,int step,int max)
{
 slider->setRange	(0,max);
 slider->setValue	(offset);
 slider->setSteps	(step,step/2);
}
//*****************************************************************************************
void MainWidget::resizeEvent  (QResizeEvent *)
{
 int bsize=buttons->sizeHint().height();

 buttons->setGeometry (0,height()-bsize,width()*3/4,bsize);
 zoomselect->setGeometry (width()*3/4,height()-bsize,width()/4,bsize);
 slider->setGeometry (0,height()-(bsize+20),width(),20);
 signalview->setGeometry (0,0,width(),height()-(bsize+20));
}
