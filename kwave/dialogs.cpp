#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <qlayout.h>
#include <qtooltip.h>

extern QString mstotime (int ms); 
const char *ratetext[]={"48000","44100","32000","22050","12000","10000"}; 
//**********************************************************
NewSampleDialog::NewSampleDialog (QWidget *par=NULL): QDialog(par, "Choose Length and Rate",true)
{
	resize 		(320,200);
	setCaption	("Choose Length and Rate :");
	timelabel	=new QLabel	("Time : 1 s",this);
	timeslider	=new QScrollBar (1,3000,1,1000,1000,QScrollBar::Horizontal,this,"timeslider");     

	ratelabel	=new QLabel 	("Rate in Hz :",this);
	ratefield	=new QComboBox  (true,this,"RateComboBox");
	ratefield->insertStrList (ratetext,6);

	ok		=new QPushButton ("Ok",this);
	cancel		=new QPushButton ("Cancel",this);

	int bsize=ok->sizeHint().height();

	setMinimumSize (320,bsize*6);

	ok->setFocus	();
	connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
	connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
	connect		(timeslider,SIGNAL(valueChanged(int)),SLOT(setLength(int)));
}
//**********************************************************
int NewSampleDialog::getLength ()
{
 int ms=timeslider->value();
 if (ms<1000) ;						//don´t wonder,
 else if (ms<2000) ms=(ms-999)*1000;			//this system was invented by random testing
 	else	   ms=(ms-1999)*60000+(16*60*1000);	//err, I mean by intensive scientific research
 return ms;
}
//**********************************************************
int NewSampleDialog::getRate ()
{
	const char *buf=ratefield->currentText();
	return ((int) (strtol(buf,0,0)));
}
//**********************************************************
void NewSampleDialog::setLength (int ms)
{
 if (ms<1000) ;						//don´t wonder,
 else if (ms<2000) ms=(ms-999)*1000;			//this system was invented by random testing
 	else	   ms=(ms-1999)*60000+(16*60*1000);	//err, I mean by intensive scientific research
							//ok, i was just to lazy for a continuous logarithmic scaling...
 QString buf="Time :"+ mstotime (ms);
 timelabel->setText (buf.data());
}
//**********************************************************
void NewSampleDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*8/10,bsize);  
 timeslider->setGeometry(width()/10,	bsize*3/2,width()*8/10,bsize);  

 ratelabel->setGeometry	(width()/10,	bsize*3,width()*3/10,bsize);  
 ratefield->setGeometry	(width()*4/10,	bsize*3,width()/2,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
NewSampleDialog::~NewSampleDialog ()
{
}
//**********************************************************
DelayDialog::DelayDialog (QWidget *par=NULL): QDialog(par, "Choose Delay",true)
{
  resize (320,200);
  setCaption ("Choose Length and Rate :");
  delaylabel =new QLabel	("Delay : 50 ms",this);
  delayslider=new QScrollBar (1,20000,1,100,500,QScrollBar::Horizontal,this,"Choose Delay");

  ampllabel  =new QLabel	("Amplitude of delayed signal :50 %",this);
  amplslider =new QScrollBar (1,100,1,10,50,QScrollBar::Horizontal,this,"Choose Delay");

  recursive  =new QCheckBox  ("do recursive delaying",this);   

  ok=new QPushButton ("Ok",this);
  cancel =new QPushButton ("Cancel",this);

  int bsize=ok->sizeHint().height();
  setMinimumSize (320,bsize*8);

  ok->setFocus	();
  connect (ok,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
  connect (delayslider,SIGNAL(valueChanged(int)),SLOT(setDelay(int)));
  connect (amplslider,SIGNAL(valueChanged(int)),SLOT(setAmpl(int)));
}
//**********************************************************
void DelayDialog::setDelay (int ms_10)
{
  char buf[64];

  sprintf (buf,"Delay :%d.%d ms",ms_10/10,ms_10%10);
  delaylabel->setText (buf);
}
//**********************************************************
void DelayDialog::setAmpl (int percent)
{
  char buf[64];

  sprintf (buf,"Amplitude of delayed signal :%d %%",percent);
  ampllabel->setText (buf);
}
//**********************************************************
int DelayDialog::getDelay ()
{
 return delayslider->value();
}
//**********************************************************
int DelayDialog::getAmpl ()
{
 return amplslider->value();
}
//**********************************************************
int DelayDialog::isRecursive ()
{
 return recursive->isChecked();
}
//**********************************************************
void DelayDialog::resizeEvent (QResizeEvent *)
{
int bsize=ok->sizeHint().height();

 delaylabel->setGeometry (width()/10,	bsize/2,width()*8/10,bsize);  
 delayslider->setGeometry(width()/10,	bsize*3/2,width()*8/10,bsize);  
 ampllabel->setGeometry  (width()/10,	bsize*3,width()*8/10,bsize);  
 amplslider->setGeometry (width()/10,	bsize*4,width()*8/10,bsize);  

 recursive->setGeometry  (width()/10,	bsize*11/2,width()*8/10,bsize);  

 ok->setGeometry	 (width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	 (width()*6/10,height()-bsize*3/2,width()*3/10,bsize); }
//**********************************************************
DelayDialog::~DelayDialog ()
{
}
//**********************************************************
ProgressDialog::ProgressDialog (int max=100,char
*caption="Progress"): QDialog(0,caption)
{
  resize (320,20);
  setCaption (caption);
  progress =new KProgress (0,max,0,KProgress::Horizontal,this,"Progress so
far");
}
//**********************************************************
void ProgressDialog::setProgress (int x)
{
	progress->setValue (x);
}
//**********************************************************
void ProgressDialog::resizeEvent (QResizeEvent *)
{
 progress->setGeometry (0,0,width(),height());  
}
//**********************************************************
ProgressDialog::~ProgressDialog ()
{
}
//**********************************************************
RateDialog::RateDialog (QWidget *par=NULL): QDialog(par, "Choose Length and Rate",true)
{
  setCaption	("Choose New Rate :");

  ratelabel	=new QLabel 	("Rate in Hz :",this);
  ratefield	=new QComboBox  (true,this,"RateComboBox");
  ratefield->insertStrList (ratetext,6);

  ok		=new QPushButton ("Ok",this);
  cancel		=new QPushButton ("Cancel",this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*7/2);
  resize	 (320,bsize*7/2);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int RateDialog::getRate ()
{
	const char *buf=ratefield->currentText();
	return ((int) (strtol(buf,0,0)));
}
//**********************************************************
void RateDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 ratelabel->setGeometry	(width()/10,	bsize/2,width()*3/10,bsize);  
 ratefield->setGeometry	(width()*4/10,	bsize/2,width()/2,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
RateDialog::~RateDialog ()
{
}
//**********************************************************
PlayBackDialog::PlayBackDialog (QWidget *par=NULL,int play16bit=false): QDialog(par, "Choose
Playback parameters",true)
{
  setCaption	("Playback Options :");
  QVBoxLayout *vbox;

  bg = new QButtonGroup( this, "Resolution" );
  bg->setTitle( "Resolution" );  
  vbox = new QVBoxLayout(bg, 10);

  vbox->addSpacing( bg->fontMetrics().height() );
  b16 = new QRadioButton( bg );
  b16->setText( "16 Bit" );

  vbox->addWidget(b16);
  b16->setMinimumSize( b16->sizeHint() );
  QToolTip::add( b16, "radio button 1" );
  b8 = new QRadioButton( bg );
  b8->setText( "8 Bit" );
  vbox->addWidget(b8);
  b8->setMinimumSize( b8->sizeHint() );
  QToolTip::add( b8, "radio button 2" );
    
  if (play16bit) b16->setChecked( TRUE );
  else b8->setChecked( TRUE );

  ok		=new QPushButton ("Ok",this);
  cancel		=new QPushButton ("Cancel",this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (200,bsize*6);
  resize	 (200,bsize*6);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int  PlayBackDialog::getResolution ()
{return b16->isChecked();}
//**********************************************************
void PlayBackDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 bg->setGeometry	(width()/20,bsize/2,width()*18/20,height()-(bsize*5/2));  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PlayBackDialog::~PlayBackDialog ()
{
}






