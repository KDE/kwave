#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include "interpolation.h"
#include "classes.h"

extern QList<MarkerType>*markertypes;
extern QString mstotime (int ms); 
extern char*   mstotimec (int ms); 
static const char *OK="&Ok";
static const char *CANCEL="Cancel";
int fancy=false;

static const char *ratetext[]={"48000","44100","32000","22050","12000","10000",0}; 
static const char *devicetext[]={"/dev/dsp","/dev/dio",0}; 
static const char *notetext[]=
{
  "C 0","C# 0","D 0","D# 0","E 0","F 0","F# 0","G 0","G# 0","A 0","A# 0","B 0",
  "C 1","C# 1","D 1","D# 1","E 1","F 1","F# 1","G 1","G# 1","A 1","A# 1","B 1",
  "C 2","C# 2","D 2","D# 2","E 2","F 2","F# 2","G 2","G# 2","A 2","A# 2","B 2",
  "C 3","C# 3","D 3","D# 3","E 3","F 3","F# 3","G 3","G# 3","A 3","A# 3","B 3",
  "C 4","C# 4","D 4","D# 4","E 4","F 4","F# 4","G 4","G# 4","A 4","A# 4","B 4",
  "C 5","C# 5","D 5","D# 5","E 5","F 5","F# 5","G 5","G# 5","A 5","A# 5","B 5",
  "C 6","C# 6","D 6","D# 6","E 6","F 6","F# 6","G 6","G# 6","A 6","A# 6","B 6",
  "C 7","C# 7","D 7","D# 7","E 7","F 7","F# 7","G 7","G# 7","A 7","A# 7","B 7",
  "C 8","C# 8","D 8","D# 8","E 8","F 8","F# 8","G 8","G# 8","A 8","A# 8","B 8",
  0
}; 
float notefreq[]=
{
  16.4,  17.3,  18.4,  19.4,  20.6,  21.8,  23.1,  24.5,  26.0,  27.5,  29.1,  30.9,
  32.7,  34.6,  36.7,  38.9,  41.2,  43.7,  46.2,  49.0,  51.9,  55.0,  58.3,  61.7,
  65.4,  69.3,  73.4,  77.8,  82.4,  87.3,  92.5,  98.0,  103.8, 110,   116.5, 123.5,
  130.8, 138.6, 146.8, 155.6, 164.8, 174.6, 185,   196,   207.7, 220,   233.1, 246.9,
  261.4, 277.2, 293.7, 311.1, 329.6, 349.2, 370,   392,   415.3, 440,   466.2, 493.9,
  523.3, 554.4, 587.3, 622.3, 659.3, 698.5, 740,   784,   830.6, 880,   932.3, 987.8,
  1046.5,1108.7,1174.7,1244.5,1318.5,1369.9,1480,  1568,  1661.2,1760,  1864.7,1975.5,
  2093,  2217.5,2349.3,2489,  2637,  2793.8,2960,  3136,  3322.4,3520,  3729.3,3951,
  4186,  4435,  4699.6,4978,  5274,  5587.5,5920,  6272,  6644.8,7040,  7458.6,7902
};

static const char *FFT_Sizes[]={"64","128","256","512","1024","2048","4096",0};
//**********************************************************
TimeLine::TimeLine (QWidget *parent,int rate):KRestrictedLine (parent)
{
  this->rate=rate;
  mode=1;
  menu=new QPopupMenu ();
  value=1;
  
  menu->insertItem	(klocale->translate("as number of samples"),this,SLOT(setSampleMode()));
  menu->insertItem	(klocale->translate("in ms"),this,SLOT(setMsMode()));
  menu->insertItem	(klocale->translate("in s"), this,SLOT(setSMode()));
  menu->insertItem	(klocale->translate("in kb"),this,SLOT(setKbMode()));

  menu->setCheckable (true);


  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),true);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);

  connect( this, SIGNAL(textChanged(const char *)),SLOT(setValue(const char *)) ); 
};
//**********************************************************
int TimeLine::getValue ()
{
  return value;
}
//**********************************************************
void TimeLine::setRate (int newrate)
{
  rate=newrate;
  setSamples (value);
}
//**********************************************************
void TimeLine::setValue (const char *newvalue)
{
  switch (mode)
    {
    case 0:
      value=strtol (newvalue,0,0); 
      break;
    case 1:
      value=(int) ((double)(rate*strtod (newvalue,0)/1000)+.5);
      break;
    case 2:
      value=(int) ((double)(rate*strtod (newvalue,0))+.5);
      break;
    case 3:
      value=(int) ((double)(strtod (newvalue,0)*1024)/sizeof(int)+.5);
      break;
    }
}
//**********************************************************
void TimeLine::setSampleMode ()
{
  menu->setItemChecked (menu->idAt(0),true);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789");
  mode=0;
  setSamples (value);
}
//**********************************************************
void TimeLine::setMsMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),true);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789.");
  mode=1;
  setSamples (value);
}
//**********************************************************
void TimeLine::setSMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),true);
  menu->setItemChecked (menu->idAt(3),false);
  setValidChars ("0123456789.");
  mode=2;
  setSamples (value);
}
//**********************************************************
void TimeLine::setKbMode ()
{
  menu->setItemChecked (menu->idAt(0),false);
  menu->setItemChecked (menu->idAt(1),false);
  menu->setItemChecked (menu->idAt(2),false);
  menu->setItemChecked (menu->idAt(3),true);
  setValidChars ("0123456789.");
  mode=3;
  setSamples (value);
}
//**********************************************************
void TimeLine::setSamples (int samples)
{
  char buf[64];

  value=samples;

  switch (mode)
    {
    case 0:
      sprintf (buf,"%d samples",value);
      this->setText (buf);
      break;
    case 1:
      {
	double pr=((double)value)*1000/rate;
	sprintf (buf,"%.03f ms",pr);
	this->setText (buf);
      }
      break;
    case 2:
      {
	double pr=((double)value)/rate;
	sprintf (buf,"%.3f s",pr);
	this->setText (buf);
      }
      break;
    case 3:
      {
	double pr=((double)(value))*sizeof(int)/1024;
	sprintf (buf,"%.3f kb",pr);
	this->setText (buf);
      }
      break;
    }
}
//**********************************************************
void TimeLine::setMs (int ms)
{
  char buf[16];

  value=(int) ((double)(rate*ms/1000)+.5);
  if (mode==0)
    {
      sprintf (buf,"%d samples",value);
      this->setText (buf);
    }
  else
    {
      sprintf (buf,"%d.%d ms",ms,0);
      this->setText (buf);
    }
}
//**********************************************************
void TimeLine::mousePressEvent( QMouseEvent *e)
{
  if (e->button()==RightButton)
    {
      QPoint popup=QCursor::pos();
      menu->popup(popup);
    }            
}
//**********************************************************
TimeLine::~TimeLine ()
{
};
//**********************************************************
NewSampleDialog::NewSampleDialog (QWidget *par=NULL): QDialog(par, 0,true)
{

  setCaption	(klocale->translate("Choose Length and Rate :"));
  timelabel	=new QLabel	(klocale->translate("Time :"),this);
  time	=new TimeLine (this,44100);
  time->setMs (1000);

  ratelabel	=new QLabel 	(klocale->translate("Rate in Hz :"),this);
  ratefield	=new QComboBox  (true,this);
  ratefield->insertStrList (ratetext,6);
  ratefield->setCurrentItem (1);

  ok		=new QPushButton (OK,this);
  cancel	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize (320,bsize*5);

  ok->setFocus	();
  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);

  connect (ok	    ,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel   ,SIGNAL(clicked()),SLOT (reject()));
  connect (ratefield,SIGNAL(activated (const char *)),SLOT(setRate(const char *)));
}
//**********************************************************
void NewSampleDialog::setRate (const char *)
{
  time->setRate (getRate());
}
//**********************************************************
int NewSampleDialog::getLength ()
{
 return time->getValue();
}
//**********************************************************
int NewSampleDialog::getRate ()
{
  const char *buf=ratefield->currentText();
  return ((int) (strtol(buf,0,0)));
}
//**********************************************************
void NewSampleDialog::setLength (int)
{
}
//**********************************************************
void NewSampleDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*3/10,bsize);  
 time->setGeometry      (width()*4/10,	bsize/2,width()*5/10,bsize);  

 ratelabel->setGeometry	(width()/10,	bsize*2,width()*3/10,bsize);  
 ratefield->setGeometry	(width()*4/10,	bsize*2,width()/2,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
NewSampleDialog::~NewSampleDialog ()
{
}
//**********************************************************
DelayDialog::DelayDialog (QWidget *par=NULL,int rate): QDialog(par,0,true)
{
  resize (320,200);
  setCaption (klocale->translate("Choose Length and Rate :"));
  delaylabel =new QLabel	(klocale->translate("Delay :"),this);
  delay      =new TimeLine      (this,rate);
  delay->setMs (200);

  ampllabel  =new QLabel	(klocale->translate("Amplitude of delayed signal :50 %"),this);
  amplslider =new QScrollBar (1,200,1,10,50,QScrollBar::Horizontal,this);

  recursive  =new QCheckBox  (klocale->translate("do recursive delaying"),this);   

  ok=new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();
  setMinimumSize (320,bsize*7);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect (ok,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
  connect (amplslider,SIGNAL(valueChanged(int)),SLOT(setAmpl(int)));
}
//**********************************************************
void DelayDialog::setAmpl (int percent)
{
  char buf[64];

  sprintf (buf,klocale->translate("Amplitude of delayed signal :%d %%"),percent);
  ampllabel->setText (buf);
}
//**********************************************************
int DelayDialog::getDelay ()
{
 return delay->getValue();
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

 delaylabel->setGeometry (width()/10,	bsize/2,width()*3/10,bsize);  
 delay->setGeometry(width()*5/10,	bsize/2,width()*4/10,bsize);  
 ampllabel->setGeometry  (width()/10,	bsize*2,width()*8/10,bsize);  
 amplslider->setGeometry (width()/10,	bsize*3,width()*8/10,bsize);  

 recursive->setGeometry  (width()/10,	bsize*9/2,width()*8/10,bsize);  

 ok->setGeometry	 (width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	 (width()*6/10,height()-bsize*3/2,width()*3/10,bsize); }
//**********************************************************
DelayDialog::~DelayDialog ()
{
}
//**********************************************************
//KProgresss doaes to much flickering, is unstable for high numbers, so here
//follows my own implementation... 
//**********************************************************
ProgressDialog::ProgressDialog (int max,char *caption): QDialog(0,caption)
{
  if (fancy)  
    {
      last=0;
      setMinimumSize (128,128);
      setMaximumSize (128,128);
      setCaption (caption);
      this->max=max;
      act=0;
      lastx=0;
      lasty=0;
      setBackgroundColor (black);
    }
  else
    {
      resize (320,20);
      setCaption (caption);
      this->max=max;
      act=0;
    }
}
//**********************************************************
void ProgressDialog::setProgress (int x)
{
  act=x;

  repaint(false);
}
//**********************************************************
void ProgressDialog::paintEvent (QPaintEvent *)
{
  QPainter p;
  int perc=(int)(((double)act)*100/max);


  if (fancy)  
    {
      int width=128;
      int height=128;

      p.begin (this);
      int col=(int)((sin(60*M_PI*(double)act/max)+1)*64);
      p.setPen (QPen(QColor(col,col,col),width/16));
      //  p.setRasterOp (XorROP);
      p.translate (width/2,height/2);
  
      int newx=(int)(sin(64*M_PI*(double)act/max)*width/sqrt(2)*act/max);
      int newy=(int)(cos(64*M_PI*(double)act/max)*height/sqrt(2)*act/max);
  
      p.drawLine   (lastx,lasty,newx,newy);

      if (perc>last)
	{
	  char buf[8];
	  sprintf (buf,"%d %%",perc);
	  last=perc;

	  p.setRasterOp (CopyROP);

	  QFont newfont(font());

	  newfont.setPointSize (width/4);
	  newfont.setBold (true);

	  p.setPen (black);
	  p.setFont  (newfont);
	  p.drawText (-width/2+3,-height/2,width,height,AlignCenter,buf,5);
	  p.setFont  (newfont);
	  p.drawText (-width/2-3,-height/2,width,height,AlignCenter,buf,5);
	  p.setFont  (newfont);
	  p.drawText (-width/2,-height/2+3,width,height,AlignCenter,buf,5);
	  p.setFont  (newfont);
	  p.drawText (-width/2,-height/2-3,width,height,AlignCenter,buf,5);

	  newfont.setPointSize (width/4);
	  newfont.setBold (true);

	  p.setPen (white);
	  p.setFont  (newfont);
	  p.drawText (-width/2,-height/2,width,height,AlignCenter,buf,5);
	}

      lastx=newx;
      lasty=newy;
      p.end();
    }
  else 
    {
      int col=(int)((((double)act)/max)*(512+64));
      int w=(int)((((double)act)/max)*(width()-2));
      QPixmap map (width(),height());
      map.fill (colorGroup().background());
      char buf[8];
      sprintf (buf,"%d %%",perc);

      p.begin (&map);
      if (col<256)
	{
	  p.setPen (QColor(255,col,0));
	  p.setBrush (QColor(255,col,0));
	}
      else
	if (col<512)
	  {
	    p.setPen (QColor(511-col,255,0));
	    p.setBrush (QColor(511-col,255,0));
	  }
      else
	  {
	    p.setPen (QColor(0,767-col,0));
	    p.setBrush (QColor(0,767-col,0));
	  }
      p.drawRect (1,1,w,height()-2);
      p.setPen (colorGroup().light());
      p.drawLine (0,height()-1,width(),height()-1);
      p.drawLine (width()-1,0,width()-1,height()-2);
      p.setPen (colorGroup().dark());
      p.drawLine (0,0,width(),0);
      p.drawLine (0,0,0,height()-1);
      p.end ();

      bitBlt (this,0,0,&map);

      p.begin (this);
      p.setPen (colorGroup().text());

      p.drawText (0,0,width(),height(),AlignCenter,buf,5);
      p.end ();
    }

}
//**********************************************************
ProgressDialog::~ProgressDialog ()
{
}
//**********************************************************
RateDialog::RateDialog (QWidget *par): QDialog(par, 0,true)
{
  setCaption	(klocale->translate("Choose New Rate :"));

  ratelabel	=new QLabel 	(klocale->translate("Rate in Hz :"),this);
  ratefield	=new QComboBox  (true,this);
  ratefield->insertStrList (ratetext,-1);

  ok		=new QPushButton (OK,this);
  cancel		=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*7/2);
  resize	 (320,bsize*7/2);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
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
PlayBackDialog::PlayBackDialog (QWidget *par,int play16bit,int bufbase): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Playback Options :"));


  devicelabel	=new QLabel 	(klocale->translate("Playback Device :"),this);
  devicebox	=new QComboBox  (true,this);
  devicebox->insertStrList (devicetext,-1);

  QVBoxLayout *vbox;
  bg = new QButtonGroup( this);
  bg->setTitle(klocale->translate("Resolution"));  
  vbox = new QVBoxLayout(bg, 10);

  vbox->addSpacing( bg->fontMetrics().height() );
  b16 = new QRadioButton( bg );
  b16->setText( "16 Bit");
  vbox->addWidget(b16);
  b16->setMinimumSize(b16->sizeHint());
  QToolTip::add( b16, klocale->translate("Set Playback-Mode to 16 Bit\nNote that not all sound hardware supports this mode !"));
  b16->setMinimumSize( b16->sizeHint());

  b8 = new QRadioButton( bg );
  b8->setText( "8 Bit" );
  vbox->addWidget(b8);
  b8->setMinimumSize( b8->sizeHint() );
 
  buffersize=new QSlider (4,16,1,bufbase,QSlider::Horizontal,this);
  bufferlabel=new QLabel ("",this); 
  setBufferSize (bufbase);
  QToolTip::add( buffersize, klocale->translate("This is the size of the buffer used for communication with the sound driver\nIf your computer is rather slow select a big buffer"));

  stereo=new QCheckBox (klocale->translate("enable stereo playback"),this);
  stereo->setEnabled (false);

  if (play16bit) b16->setChecked( TRUE );
  else b8->setChecked( TRUE );

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();
  int lsize=ok->sizeHint().width();

  setMinimumSize (lsize*10,bsize*12);
  resize	 (lsize*10,bsize*12);

  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect (buffersize,SIGNAL(valueChanged(int)),SLOT(setBufferSize(int)));
}
//**********************************************************
void  PlayBackDialog::setBufferSize (int exp)
{
  char buf [64];
  int val=1<<exp;

  sprintf (buf,klocale->translate("Buffer Size : %d Samples"),val);
  bufferlabel->setText (buf);
}
//**********************************************************
int  PlayBackDialog::getResolution ()
{
return b16->isChecked();
}
//**********************************************************
int  PlayBackDialog::getBufferSize ()
{
  return buffersize->value();
}
//**********************************************************
void PlayBackDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=0;

 bg->setGeometry	        (width()/20,bsize/2,width()*18/20,bsize*4);  

 offset+=bsize*5;
 devicelabel->setGeometry	(width()*1/10,offset,width()*7/20,bsize);  
 devicebox->setGeometry	        (width()*1/2,offset,width()*8/20,bsize);  
 offset+=bsize*3/2;

 bufferlabel->setGeometry	(width()/10,offset,width()*4/10,bsize);  
 buffersize->setGeometry	(width()*6/10,offset,width()*3/10,bsize);  
 offset+=bsize*3/2;

 stereo->setGeometry	(width()/10,offset,width()*8/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PlayBackDialog::~PlayBackDialog ()
{
}
//**********************************************************
FadeDialog::FadeDialog (QWidget *par,int dir): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Choose Curve :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);
  slider =new QScrollBar (-100,100,1,10,0,QScrollBar::Horizontal,this);     
  fade= new FaderWidget (this,dir);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize	 (320,bsize*8);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(slider	,SIGNAL(valueChanged(int)),fade,SLOT (setCurve(int)));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int FadeDialog::getCurve ()
{
  return (fade->getCurve());
}
//**********************************************************
void FadeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 fade->setGeometry	(width()/20,0,width()*18/20,height()-bsize*7/2);  
 slider->setGeometry	(width()/20,height()-bsize*3,width()*18/20,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FadeDialog::~FadeDialog ()
{
}
//**********************************************************
TimeDialog::TimeDialog (QWidget *par,int rate,char *name): QDialog(par,0,true)
{
  resize 	(320,200);
  setCaption	(name);
  timelabel	=new QLabel	(klocale->translate("Time :"),this);
  time	        =new TimeLine   (this,rate);     
  time->setMs   (1000);

  ok		=new QPushButton (OK,this);
  cancel	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);
  resize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();

  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int TimeDialog::getLength ()
{
 return time->getValue();
}
//**********************************************************
void TimeDialog::setLength (int sam)
{
  time->setSamples (sam);
}
//**********************************************************
void TimeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*3/10,bsize);  
 time->setGeometry(width()*5/10,	bsize/2,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
TimeDialog::~TimeDialog ()
{
}
//**********************************************************
AmplifyCurveDialog::AmplifyCurveDialog (QWidget *par=NULL): QDialog(par, 0,true)
{
  setCaption	(klocale->translate("Choose Curve :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  curve= new CurveWidget (this);
  curve->setBackgroundColor	(QColor(black) );

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize	 (320,bsize*8);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
QList<CPoint> *AmplifyCurveDialog::getPoints ()
{
  return curve->getPoints();
}
//**********************************************************
int AmplifyCurveDialog::getType ()
{
  return curve->getType();
}
//**********************************************************
void AmplifyCurveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 curve->setGeometry	(width()/20,0,width()*18/20,height()-bsize*5/2);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AmplifyCurveDialog::~AmplifyCurveDialog ()
{
  delete curve ;
}
//**********************************************************
HullCurveDialog::HullCurveDialog (QWidget *par,char *name): QDialog(par,0,true)
{
  Interpolation interpolation(0);

  setCaption	(name);
  timelabel	=new QLabel	(klocale->translate("Points are taken 20 times/s"),this);
  timeslider	=new QScrollBar (1,1000,1,1,200,QScrollBar::Horizontal,this);     

  typelabel	=new QLabel	(klocale->translate("Type of Interpolation :"),this);
  typebox	        =new QComboBox  (true,this);
  typebox->insertStrList (interpolation.getTypes(),-1);

  ok		=new QPushButton (OK,this);
  cancel	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);
  resize (320,bsize*6);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect	(timeslider,SIGNAL(valueChanged(int)),SLOT(setTime(int)));
}
//**********************************************************
int HullCurveDialog::getTime ()
{
 return timeslider->value();
}
//**********************************************************
int HullCurveDialog::getType ()
{
 return typebox->currentItem();
}
//**********************************************************
void HullCurveDialog::setTime (int ms)
{
 char buf[32];

 sprintf (buf,klocale->translate("Points are taken %d.%01d times/s"),ms/10,ms%10);
 timelabel->setText (buf);
}
//**********************************************************
void HullCurveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,	bsize/2,width()*8/10,bsize);  
 timeslider->setGeometry(width()/10,	bsize*3/2,width()*8/10,bsize);  

 typelabel->setGeometry	(width()/10,	bsize*3,width()*4/10,bsize);  
 typebox->setGeometry	(width()/2,	bsize*3,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
HullCurveDialog::~HullCurveDialog ()
{
}
//**********************************************************
DistortDialog::DistortDialog (QWidget *par): QDialog(par,0,true)
{
  setCaption	(klocale->translate("Choose Distortion-Line :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  curve= new CurveWidget (this);
  curve->setBackgroundColor	(QColor(black) );

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize	 (320,bsize*8);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
QList<CPoint> *DistortDialog::getPoints ()
{
  return curve->getPoints();
}
//**********************************************************
int DistortDialog::getType ()
{
  return curve->getType();
}
//**********************************************************
void DistortDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 curve->setGeometry	(width()/20,0,width()*18/20,height()-bsize*5/2);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
DistortDialog::~DistortDialog ()
{
  delete curve ;
}
//**********************************************************
AverageDialog::AverageDialog (QWidget *par,char *name): QDialog(par, 0,true)
{
  resize  (320,200);
  setCaption	(name);
  taplabel	=new QLabel	(klocale->translate("# of Filter Taps : 3"),this);
  tapslider	=new QScrollBar (1,255,1,5,3,QScrollBar::Horizontal,this);     

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect		(tapslider,SIGNAL(valueChanged(int)),SLOT(setTaps(int)));
}
//**********************************************************
int AverageDialog::getTaps ()
{
 return tapslider->value();
}
//**********************************************************
void AverageDialog::setTaps (int ms)
{
  char buf[32];
  sprintf (buf,klocale->translate("# of Filter Taps : %d"),ms);  
  taplabel->setText(buf);
}
//**********************************************************
void AverageDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 taplabel->setGeometry	(width()/10,	bsize/2,width()*8/10,bsize);  
 tapslider->setGeometry(width()/10,	bsize*3/2,width()*8/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AverageDialog::~AverageDialog ()
{
}
//**********************************************************
SonagramDialog::SonagramDialog (QWidget *par,int len,int rate,char *name): QDialog(par, 0,true)
{
  this->rate=rate;
  this->length=len;
  resize 	(320,200);
  setCaption	(name);
  pointlabel	=new QLabel	(klocale->translate("Number of FFT points:"),this);
  pointbox	=new QComboBox  (true,this);
  pointbox->insertStrList (FFT_Sizes,-1);
  QToolTip::add(pointbox,klocale->translate("Try to choose numbers with small prime-factors, if choosing big window sizes.\nThe computation will be much faster !"));
  
  windowlabel	=new QLabel	("",this);
  bitmaplabel	=new QLabel	("",this);
  pointslider	=new QScrollBar (2,(len/16),1,5,50,QScrollBar::Horizontal,this);

  setPoints (50);

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect	(pointslider,SIGNAL(valueChanged(int)),SLOT(setPoints(int)));
  connect	(pointbox,SIGNAL   (activated(int))   ,SLOT(setBoxPoints(int)));
}
//**********************************************************
int SonagramDialog::getPoints ()
{
 return pointslider->value()*2;
}
//**********************************************************
void SonagramDialog::setPoints (int points)
{
  char buf[32];
  points*=2;
  
  sprintf (buf,"%d",points);
  pointbox->changeItem (buf,0);
  pointbox->setCurrentItem (0);
  sprintf (buf,klocale->translate("resulting window size: %s"),(mstotimec(points*10000/rate)));  
  windowlabel->setText(buf);
  sprintf (buf,klocale->translate("size of bitmap: %dx%d"),2*length/(points)+1,points/2);  
  bitmaplabel->setText(buf);
}
//**********************************************************
void SonagramDialog::setBoxPoints (int num)
{
  int points=strtol(pointbox->text (num),0,0);
  pointslider->setValue (points/2);
}
//**********************************************************
void SonagramDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 pointlabel->setGeometry (width()/10,	bsize/2,width()*4/10,bsize);  
 pointbox->setGeometry (width()/2,	bsize/2,width()*3/10,bsize);  
 windowlabel->setGeometry (width()/10,	bsize*3/2,width()*8/10,bsize);  
 bitmaplabel->setGeometry (width()/10,	bsize*5/2,width()*8/10,bsize);  
 pointslider->setGeometry(width()/10,	bsize*4,width()*8/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
SonagramDialog::~SonagramDialog ()
{
}
//**********************************************************
MarkerTypeDialog::~MarkerTypeDialog ()
{
}
//**********************************************************
MarkerTypeDialog::MarkerTypeDialog (QWidget *par,char *name): QDialog(par, 0,true)
{
  namelabel	=new QLabel	(klocale->translate("Name :"),this);
  this->name    =new QLineEdit	(this);
  color         =new KColorCombo(this);
  col.setRgb (255,255,0);
  color->setColor (col);

  individual   =new QCheckBox  (klocale->translate("individual name for each label"),this);   

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize (320,bsize*5);

  this->name->setFocus();

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(color	,SIGNAL(activated(const QColor &))
		 ,SLOT (setColor(const QColor &)));
  setCaption (name);
}
//**********************************************************
const char *MarkerTypeDialog::getName (){return name->text();}
int  MarkerTypeDialog::getIndividual () {return individual->isChecked();}
QColor MarkerTypeDialog::getColor ()    {return col;}
//**********************************************************
void MarkerTypeDialog::setColor (const QColor &col)
{
  this->col=col;
}
//**********************************************************
void MarkerTypeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=bsize/2;

 namelabel->setGeometry	(width()/20,offset,width()*3/20,bsize);  
 name->setGeometry	(width()*2/10,offset,width()*5/10,bsize);
 color->setGeometry	(width()*15/20,offset,width()*2/10,bsize);
 offset+=bsize*3/2;
 individual->setGeometry(width()/20,offset,width()*18/20,bsize);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
StringEnterDialog::StringEnterDialog (QWidget *par,char *title,char *init): QDialog(par, 0,true)
{
  setCaption	(title);

  name=new QLineEdit (this);
  if (init) name->setText (init); 

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();
  int lsize=name->sizeHint().height();
  setMinimumSize (320,lsize*2+bsize);
  resize	 (320,lsize*2+bsize);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  name->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *StringEnterDialog::getString ()
{
  return name->text();
}
//**********************************************************
void StringEnterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int lsize=name->sizeHint().height();

 name->setGeometry	(width()/20,0,width()*18/20,lsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
StringEnterDialog::~StringEnterDialog ()
{
}
//**********************************************************
PercentDialog::PercentDialog (QWidget *par,char *title): QDialog(par, 0,true)
{
  setCaption	(title);

  slider=new QSlider (1,1000,1,500,QSlider::Horizontal,this);
  label =new QLabel ("",this);
  setValue  (500);

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);
  resize	 (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(slider	,SIGNAL(valueChanged(int)),SLOT (setValue(int)));
}
//**********************************************************
void PercentDialog::setValue (int num)
{
  char buf[32];
  sprintf (buf,"%d.%d %% :",num/10,num%10);
  label->setText (buf);
}
//**********************************************************
int PercentDialog::getPerMille ()
{
  return slider->value();
}
//**********************************************************
void PercentDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 label->setGeometry	(width()/20,0,width()*8/20,bsize);  
 slider->setGeometry	(width()/2,0,width()*9/20,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PercentDialog::~PercentDialog ()
{
}
//**********************************************************
FixedFrequencyDialog::FixedFrequencyDialog (QWidget *par,int rate,char *title): QDialog(par, 0,true)
{
  setCaption	(title);
  this->rate=rate;
  frequencyslider=new QSlider (1,(rate/2)*9/10,1,440   ,QSlider::Horizontal,this);
  frequencylabel =new QLabel (klocale->translate("Base: 440 Hz"),this);

  timeslider=new QSlider (1,10000,1,100,QSlider::Horizontal,this);
  timelabel =new QLabel (klocale->translate("Time: 1 ms"),this);

  showTime (100);

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize	 (320,bsize*5);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(timeslider	,SIGNAL(valueChanged(int)),SLOT (showTime(int)));
  connect 	(frequencyslider,SIGNAL(valueChanged(int)),SLOT (showFrequency(int)));
}
//**********************************************************
void FixedFrequencyDialog::showFrequency (int newvalue)
{
  char buf[16];
  sprintf (buf,klocale->translate("Base: %d Hz"),newvalue);
  frequencylabel->setText (buf);
  showTime (timeslider->value());
}
//**********************************************************
int FixedFrequencyDialog::getTime (){return timeslider->value();}
int FixedFrequencyDialog::getFrequency () {return frequencyslider->value();}
//**********************************************************
void FixedFrequencyDialog::showTime (int newvalue)
{
  char buf[20];
  sprintf (buf,klocale->translate("Time: %s"),mstotime (10000*newvalue/frequencyslider->value()).data());
  timelabel->setText (buf);
}
//**********************************************************
void FixedFrequencyDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int offset=bsize/2;

 timelabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
 timeslider->setGeometry	(width()/2,offset,width()*9/20,bsize);
 offset+=bsize*3/2;
 frequencylabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
 frequencyslider->setGeometry	(width()/2,offset,width()*9/20,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FixedFrequencyDialog::~FixedFrequencyDialog ()
{
}
//**********************************************************
SweepDialog::SweepDialog (QWidget *par,int rate,char *title): QDialog(par, 0,true)
{
  setCaption	(title);
  this->rate=rate;

  curve=new CurveWidget (this);

  time=      new TimeLine (this,rate);
  timelabel =new QLabel (klocale->translate("Time:"),this);
  time->setMs (100);

  freq1=new KIntegerLine (this);
  freq2=new KIntegerLine (this);

  note1=new QComboBox  (true,this);
  note1->insertStrList (notetext,-1);
  note2=new QComboBox  (true,this);
  note2->insertStrList (notetext,-1);
  note1->setCurrentItem (8);
  note2->setCurrentItem (64);
  showFreq1 (8);
  showFreq2 (64);

  notelabel1=new QLabel (klocale->translate("From"),this);
  notelabel2=new QLabel (klocale->translate("to"),this);

  showTime (100);

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(note1,SIGNAL(activated(int)),SLOT (showFreq1(int)));
  connect 	(note2,SIGNAL(activated(int)),SLOT (showFreq2(int)));
}
//**********************************************************
int SweepDialog::getTime (){return time->getValue();}
//**********************************************************
void SweepDialog::showFreq1 (int val)
{
  char buf[16];

  sprintf (buf,"%4.2f Hz",notefreq[val]);
  freq1->setText (buf);
}
//**********************************************************
void SweepDialog::showFreq2 (int val)
{
  char buf[16];

  sprintf (buf,"%4.2f Hz",notefreq[val]);
  freq2->setText (buf);
}
//**********************************************************
double SweepDialog::getFreq1 ()
{
  QString tmp(freq1->text ());
  return (tmp.toDouble());
}
//**********************************************************
int    SweepDialog::getInterpolationType() {return curve->getType();}
QList<CPoint> *SweepDialog::getPoints ()   {return curve->getPoints();}
//**********************************************************
double SweepDialog::getFreq2 ()
{
  QString tmp(freq2->text ());
  return (tmp.toDouble());
}
//**********************************************************
void SweepDialog::showTime (int)
{
}
//**********************************************************
void SweepDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
 int offset= height()-bsize*4;
 notelabel1->setGeometry	(width()/20,offset+bsize/2,width()*3/20,bsize);
 notelabel2->setGeometry	(width()*11/20,offset+bsize/2,width()*3/20,bsize);  
 note1->setGeometry	 (width()*4/20,offset,width()*5/20,bsize);  
 note2->setGeometry	 (width()*14/20,offset,width()*5/20,bsize);  
 freq1->setGeometry	 (width()*4/20,offset+bsize,width()*5/20,bsize);  
 freq2->setGeometry	 (width()*14/20,offset+bsize,width()*5/20,bsize);  
 offset-=bsize*5/4;
 timelabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
 time->setGeometry(width()/2,offset,width()*9/20,bsize);
 offset-=bsize*1/4;
 curve->setGeometry(width()/20,0,width()*18/20,offset);
}
//**********************************************************
SweepDialog::~SweepDialog ()
{
}
//**********************************************************
StutterDialog::StutterDialog (QWidget *par,int rate,char *name): QDialog(par, 0,true)
{
  resize  (320,200);
  setCaption	(name);

  label1=new QLabel (klocale->translate("length of silence"),this);
  len1=new TimeLine (this,rate);
  len1->setSamples (200);
  label2=new QLabel (klocale->translate("then use # samples"),this);
  len2=new TimeLine (this,rate);
  len2->setSamples (100);
  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int StutterDialog::getLen1 () {return len1->getValue();}
int StutterDialog::getLen2 () {return len2->getValue();}
//**********************************************************
void StutterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 label1->setGeometry	(width()/20,bsize/2,width()*5/10,bsize);  
 len1->setGeometry	(width()*6/10,bsize/2,width()*3/10,bsize);  
 label2->setGeometry	(width()/20,bsize*2,width()*5/10,bsize);  
 len2->setGeometry	(width()*6/10,bsize*2,width()*3/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
StutterDialog::~StutterDialog ()
{
}
//**********************************************************
QuantiseDialog::QuantiseDialog (QWidget *par,char *name): QDialog(par, 0,true)
{
  resize  (320,200);
  setCaption	(name);

  bitlabel=new QLabel (klocale->translate("Number of quantisation steps"),this);
  bits=new KIntegerLine (this);
  bits->setText ("4");

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*4);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int QuantiseDialog::getBits ()
{
  const char *buf=bits->text();
  int i=(strtol(buf,0,0));
  if (i<2) i=2;
  if (i>1<<24)i=1<<24;

  return i;
}
//**********************************************************
void QuantiseDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 bitlabel->setGeometry	(width()/20,bsize/2,width()*5/10,bsize);  
 bits->setGeometry	(width()*6/10,bsize/2,width()*3/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
QuantiseDialog::~QuantiseDialog ()
{
}
//**********************************************************
MarkSaveDialog::MarkSaveDialog (QWidget *par,char *name): QDialog(par, 0,true)
{
  setCaption	(name);

  save=new QListBox (this);

  MarkerType *act;
  for (act=markertypes->first();act;act=markertypes->next())
    {
      save->insertItem (act->name->data());
      act->selected=false;
    }
  save->setMultiSelection (true);
  
  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
void MarkSaveDialog::getSelection ()
{
  int cnt=0;
  MarkerType *act;
  for (act=markertypes->first();act;act=markertypes->next())
    {
      if (save->isSelected (cnt++))
      act->selected=true;
    }
}
//**********************************************************
void MarkSaveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 save->setGeometry	(width()/20,0,width()*18/20,height()-bsize*2);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MarkSaveDialog::~MarkSaveDialog ()
{
}
//**********************************************************
MarkSignalDialog::MarkSignalDialog (QWidget *par,int rate,char *name): QDialog(par, 0,true)
{
  setCaption	(name);
  tflag=false;

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  mark1=new QLabel (klocale->translate("Start label:"),this);
  mark2=new QLabel (klocale->translate("Stop label:"),this);

  timelabel=new QLabel (klocale->translate("Length of silence:"),this);
  time=new TimeLine (this,rate);
  QToolTip::add( time, klocale->translate("this is the timespan below the defined sound level\nthat is assumed to separate two signals ..."));
  time->setMs (400);
  ampllabel=new QLabel (klocale->translate("Max. silence level"),this);
  amplslider=new QSlider (1,1000,1,100,QSlider::Horizontal,this);
  ampl=new KIntegerLine (this);
  ampl->setText ("10.0 %");

  marktype1=new QComboBox (false,this);
  marktype2=new QComboBox (false,this);
  MarkerType *act;
  int cnt=0;

  for (act=markertypes->first();act;act=markertypes->next())
    {
      marktype1->insertItem (act->name->data());
      marktype2->insertItem (act->name->data());
      if (act->name->find("start",0,false)>=0) marktype1->setCurrentItem (cnt);  //just for convenience
      if (act->name->find("stop",0,false)>=0) marktype2->setCurrentItem (cnt);
      cnt++;
    }
  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*10);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect (ok	,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
  connect (amplslider,SIGNAL(valueChanged(int)),SLOT(setAmpl(int)));
  connect (ampl,SIGNAL(textChanged(const char *)),SLOT(setAmpl(const char *)));
}
//**********************************************************
int MarkSignalDialog::getLevel ()
{
  return 1;
}
//**********************************************************
int MarkSignalDialog::getType1 ()
{
  return marktype1->currentItem();
}
//**********************************************************
int MarkSignalDialog::getType2 ()
{
  return marktype2->currentItem();
}
//**********************************************************
void MarkSignalDialog::setAmpl (int val)
{
 if (!tflag)
   {
     char buf[16];
     sprintf (buf,"%d.%d %% ",val/10,val%10);
     ampl->setText (buf);
   }
}
//**********************************************************
void MarkSignalDialog::setAmpl (const char *str)
{
 int val=(int)(strtod(str,0)*10);
 tflag=true;
 amplslider->setValue (val);
 tflag=false;
}
//**********************************************************
int MarkSignalDialog::getTime ()
{
  return time->getValue();
}
//**********************************************************
void MarkSignalDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 timelabel->setGeometry	(width()/10,bsize/2,width()*4/10,bsize);  
 time->setGeometry      (width()/2 ,bsize/2,width()*4/10,bsize);  

 ampllabel->setGeometry	 (width()/10,	bsize*2,width()*4/10,bsize);  
 ampl->setGeometry       (width()/2,	bsize*2,width()*3/20,bsize);
 amplslider->setGeometry (width()*7/10,bsize*2,width()*2/10,bsize);

 mark1->setGeometry      (width()/10   ,bsize*7/2,width()*4/10,bsize);  
 mark2->setGeometry      (width()/10   ,bsize*5,width()*4/10,bsize);  
 marktype1->setGeometry  (width()/2    ,bsize*7/2,width()*4/10,bsize);  
 marktype2->setGeometry  (width()/2    ,bsize*5,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
MarkSignalDialog::~MarkSignalDialog ()
{
}
//**********************************************************
SaveBlockDialog::SaveBlockDialog (QWidget *par,char *dname): QDialog(par, 0,true)
{
  setCaption	(dname);

  dir=new QDir ("./");

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  name=new QLineEdit (this);
  name->setText ("Unnamed");

  namelabel=new QLabel (klocale->translate("Base name:"),this);
  dirlabel=new QLabel (klocale->translate("Directory:"),this);
  dirname=new QLineEdit (this);
  dirname->setText (dir->absPath());

  mark1=new QLabel (klocale->translate("Start label:"),this);
  mark2=new QLabel (klocale->translate("Stop label:"),this);

  marktype1=new QComboBox (false,this);
  marktype2=new QComboBox (false,this);

  MarkerType *act;
  int cnt=0;

  for (act=markertypes->first();act;act=markertypes->next())
    {
      marktype1->insertItem (act->name->data());
      marktype2->insertItem (act->name->data());
      if (act->name->find("start",0,false)>=0) marktype1->setCurrentItem (cnt);  //just for convenience
      if (act->name->find("stop",0,false)>=0) marktype2->setCurrentItem (cnt);
      cnt++;
    }
  int bsize=ok->sizeHint().height();

  setMinimumSize (bsize*8,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect (ok	,SIGNAL(clicked()),SLOT (check()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *SaveBlockDialog::getName ()
{
  return name->text();
}
//**********************************************************
void SaveBlockDialog::check ()
{
  if (dir) delete dir;
  dir =new QDir (dirname->text());
  if (dir->exists()) accept ();
  else
    {
      QString nix="The directory "+dir->absPath()+"does not exist\n What shall I do ?";
      int res=KMsgBox::yesNoCancel(this,"Attention",nix.data(),KMsgBox::QUESTION,"Create","Back","Cancel");

      switch (res)
	{
	case 1:
	  {
	    QString nox=dir->absPath(); //make copy of selected directory name
	    nix        =dir->absPath(); //make copy of selected directory name

	    while (!(QDir(nix.data()).exists()))
	      {
		nix.truncate (nix.findRev('/'));
	      }

	    int err=false;

	    while ((nox!=nix)&&(err==false))
	      {
		QString newdir;
		int ofs=nox.find ('/',nix.length()+1);

		if (ofs>0) newdir=nox.mid(nix.length()+1,ofs-nix.length()-1);
		else newdir=nox.right (nox.length()-nix.length()-1);

		if (!newdir.isEmpty())
		  {
		    if ((QDir (nix)).mkdir (newdir.data())) nix=nix+'/'+newdir;
		    else
		      {
			nix="Could not create "+newdir+" in "+nix;
			KMsgBox::message (this,"Info",nix,2);
			err=true;
		      }
		  }
		else
		  err=true;
	      }
	    if (err==false) accept();

	  }
	  break;
	case 3:
	  reject ();
	  break;
	}
    }
}
//**********************************************************
int SaveBlockDialog::getType1 ()
{
  return marktype1->currentItem();
}
//**********************************************************
QDir *SaveBlockDialog::getDir ()
{
  return dir;
}
//**********************************************************
int SaveBlockDialog::getType2 ()
{
  return marktype2->currentItem();
}
//**********************************************************
void SaveBlockDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 namelabel->setGeometry (width()/10  ,bsize/2,width()*4/10,bsize);
 name->setGeometry      (width()/2   ,bsize/2,width()*4/10,bsize);
 dirlabel->setGeometry  (width()/10  ,bsize*2,width()*4/10,bsize);
 dirname->setGeometry   (width()/2   ,bsize*2,width()*4/10,bsize);

 mark1->setGeometry      (width()/10   ,bsize*7/2,width()*4/10,bsize);  
 mark2->setGeometry      (width()/10   ,bsize*5,width()*4/10,bsize);  
 marktype1->setGeometry  (width()/2    ,bsize*7/2,width()*4/10,bsize);  
 marktype2->setGeometry  (width()/2    ,bsize*5,width()*4/10,bsize);  

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
SaveBlockDialog::~SaveBlockDialog ()
{
  if (dir) delete dir;
}
//**********************************************************






