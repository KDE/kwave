#include "classes.h"
#include "sample.h"

const char *zoomtext[4]={"Fit","100 %","10 %","1 %"};
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
 zoomselect->insertStrList (zoomtext,4);
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
 this->connect	( playbutton=buttons->addButton	("&Play"),SIGNAL(pressed()),
			  this,SLOT(play()));
 this->connect	( loopbutton=buttons->addButton	("&Loop"),SIGNAL(pressed()),
			  this,SLOT(loop()));
 buttons->addStretch		();
 plusbutton=buttons->addButton	("+");
 minusbutton=buttons->addButton	("-");
 buttons->addStretch		();

 this->status=status;
}
//*****************************************************************************************
void MainWidget::saveSignal  (QString *filename)
{
	filename[1]=0;
//	signalview->saveSignal (filename);
}
//*****************************************************************************************
void MainWidget::saveSelectedSignal  (QString *filename)
{
	filename[1]=0;
}
//*****************************************************************************************
void MainWidget::setSignal  (QString *filename)
{
 signalview->setSignal (filename);
}
//*****************************************************************************************
void MainWidget::setSignal  (MSignal *signal)
{
 signalview->setSignal (signal);
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
 if (num<4)
  {
	if (num==1) signalview->setFit (TRUE);
	else signalview->setZoom (strtod(zoomtext[num],0));
  }
}
//*****************************************************************************************
void MainWidget::setRangeOp 	(int op){emit setOperation (op);}
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
 char buf[64];

 if (ms<1000)
 sprintf (buf,"Selected :%d.%d ms",ms/10,ms%10);
 else if (ms<10000)
 sprintf (buf,"Selected :%d ms",ms/10);
 else if (ms<10000000)
 sprintf (buf,"Selected :%d.%d s",ms/100000,(ms%10000)/10);

 status->changeItem (buf,4);
 }
//*****************************************************************************************
void MainWidget::setTimeInfo ( int ms)
{
 char buf[64];

 printf ("Time :%d ms\n",ms);

 if (ms<1000)
 sprintf (buf,"Length :%d ms",ms);
 else
 sprintf (buf,"Length :%d.%d s",ms/1000,(ms/100)%10);

 status->changeItem (buf,1);
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
