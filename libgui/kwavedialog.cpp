#include <unistd.h>
#include "sample.h"
#include <qcursor.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include "interpolation.h"
#include "windowfunction.h"
#include "markers.h"
#include <kmsgbox.h>

extern QList<MarkerType>*markertypes;
extern QString mstotime (int ms);
extern char*   mstotimec (int ms);
const char *OK="&Ok";
const char *CANCEL="Cancel";

                                 
//uncomment this to get a fancy Progress indicator...
//#define FANCY

static const char *ratetext[]={"48000","44100","32000","22050","16000","12000","10000",0}; 
static const char *symtext[]={"symmetric application","only upper half","only lower half",0}; 
static const char *typetext[]={"Lowpass filter","Highpass filter",0}; 
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
float notefreq[]= //frequency in Hz for the above named notes...
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
FloatLine::FloatLine (QWidget *parent,double value):KRestrictedLine (parent)
{
  setValue (value);
  setValidChars ("0123456789.E");
  digits=1;
}
//**********************************************************
FloatLine::FloatLine (QWidget *parent):KRestrictedLine (parent)
{
  setValue (0);
  setValidChars ("0123456789.E");
}
//**********************************************************
void FloatLine::setValue (double value)
{
  char buf[64];
  char conv[32];
  
  sprintf (conv,"%%.%df",digits);
  sprintf (buf,conv,value);
  setText (buf);
}
//**********************************************************
double FloatLine::value ()
{
  return strtod (text(),0); 
}
//**********************************************************
FloatLine::~FloatLine ()
{
}
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
      value=(int) ((double)(strtod (newvalue,0)*1024)/sizeof(int)-.5);
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
NewSampleDialog::NewSampleDialog (QWidget *parent): QDialog(parent, 0,true)
{

  setCaption	(klocale->translate("Choose Length and Rate :"));
  timelabel	=new QLabel   (klocale->translate("Time :"),this);
  time       	=new TimeLine (this,44100);
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
DelayDialog::DelayDialog (QWidget *parent,int rate): QDialog(parent,0,true)
{
  resize (320,200);
  setCaption (klocale->translate("Choose Length and Rate :"));
  delaylabel =new QLabel	(klocale->translate("Delay :"),this);
  delay      =new TimeLine      (this,rate);
  delay->setMs (200);

  ampllabel  =new QLabel	(klocale->translate("Amplitude of delayed signal :50 %"),this);
  amplslider =new KwaveSlider (1,200,1,10,KwaveSlider::Horizontal,this);
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
//KProgresss does flicker a lot, has bugs, that appear when using high numbers, so here
//follows my own implementation... 
//**********************************************************
ProgressDialog::ProgressDialog (int max,int *counter,char *caption): QDialog(0,caption)
{
  //timerbased variant
#ifdef FANCY  
  last=0;
  setMinimumSize (128,128);
  setMaximumSize (128,128);
  setCaption (caption);
  this->max=max;
  act=0;
  lastx=0;
  lasty=0;
  setBackgroundColor (black);
#else
  resize (320,20);
  setCaption (caption);
  this->max=max;
  act=0;
#endif

  this->counter=counter;
  timer=new QTimer (this);
  connect( timer, SIGNAL(timeout()),SLOT(timedProgress()));
  timer->start( 200); //5 times per second should be enough
}
//**********************************************************
ProgressDialog::ProgressDialog (int max,char *caption): QDialog(0,caption)
{
#ifdef FANCY  
  last=0;
  setMinimumSize (128,128);
  setMaximumSize (128,128);
  setCaption (caption);
  this->max=max;
  act=0;
  lastx=0;
  lasty=0;
  setBackgroundColor (black);
#else
  resize (320,20);
  setCaption (caption);
  this->max=max;
  act=0;
#endif

  setCursor (waitCursor);
  timer=0;
  counter=0;
}
//**********************************************************
void ProgressDialog::timedProgress ()
{
  if (counter)
    {
      setProgress (*counter);
      if ((*counter)<0) delete this; //signal for ending...
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

#ifdef FANCY
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
      char buf[10];
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
#else

  int col=(int)((((double)act)/max)*(512+64));
  int w=(int)((((double)act)/max)*(width()-2));
  if (w!=oldw)
    {
      QPixmap map (width(),height());
      map.fill (colorGroup().background());
      char buf[10]="         ";
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
      w=oldw;
    }
#endif
}
//**********************************************************
ProgressDialog::~ProgressDialog ()
{
  emit done ();
  if (timer) timer->stop();
  setCursor (arrowCursor);
}
//**********************************************************
RateDialog::RateDialog (QWidget *parent): QDialog(parent, 0,true)
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
TimeDialog::TimeDialog (QWidget *parent,int rate,const char *name): QDialog(parent,0,true)
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
AmplifyCurveDialog::AmplifyCurveDialog (QWidget *parent,int time): QDialog(parent, 0,true)
{
  setCaption	(klocale->translate("Choose Amplification Curve :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  xscale=new ScaleWidget (this,0,time,"ms");
  yscale=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  curve= new CurveWidget (this);

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
  return curve->getInterpolationType();
}
//**********************************************************
void AmplifyCurveDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 curve->setGeometry	(8+bsize,0,width()-bsize-16,height()-bsize*3);  
 xscale->setGeometry	(8+bsize,height()-bsize*3,width()-bsize-16,bsize);  
 yscale->setGeometry	(8,0,bsize,height()-bsize*3);
 corner->setGeometry	(8,height()-bsize*3,bsize,bsize);

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AmplifyCurveDialog::~AmplifyCurveDialog ()
{
  delete curve ;
}
//**********************************************************
FrequencyMultDialog::FrequencyMultDialog (QWidget *parent,int rate): QDialog(parent, 0,true)
{
  setCaption	(klocale->translate("Select Function :"));

  this->rate=rate;

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  xscale=new ScaleWidget (this,0,rate/2,"Hz");
  yscale=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  curve= new CurveWidget (this);

  x=new KIntegerLine (this);
  y=new KIntegerLine (this);
  x->setValue (2000);
  y->setValue (100);

  add	 = new QPushButton (klocale->translate("&Add"),this);
  xlabel = new QLabel (klocale->translate("Freq. in Hz"),this);
  ylabel = new QLabel (klocale->translate("Ampl. in %"),this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*10);
  resize	 (320,bsize*10);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(add	,SIGNAL(clicked()),SLOT (addPoint()));
}
//**********************************************************
QList<CPoint> *FrequencyMultDialog::getPoints ()
{
  return curve->getPoints();
}
//**********************************************************
void FrequencyMultDialog::addPoint ()
{
  double freq=((double)x->value())/rate;
  double amp=((double)y->value())/100;
  curve->addPoint (freq,amp);
}
//**********************************************************
int FrequencyMultDialog::getType ()
{
  return curve->getInterpolationType();
}
//**********************************************************
void FrequencyMultDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int ch=height()-bsize*11/2;

 curve->setGeometry	(bsize,0,width()-bsize,ch);  
 xscale->setGeometry	(bsize,ch,width()-bsize,bsize);  
 yscale->setGeometry	(0,0,bsize,ch);
 corner->setGeometry	(0,ch,bsize,bsize);

 xlabel->setGeometry	(8,height()-bsize*4,width()*3/10-8,bsize);
 x->setGeometry         (8,height()-bsize*3,width()*3/10-8,bsize);  
 ylabel->setGeometry	(width()*4/10,height()-bsize*4,width()*3/10-8,bsize);  
 y->setGeometry         (width()*4/10,height()-bsize*3,width()*3/10-8,bsize);  
 add->setGeometry       (width()*7/10,height()-bsize*3,width()*3/10-8,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FrequencyMultDialog::~FrequencyMultDialog ()
{
  delete curve ;
}
//**********************************************************
PitchDialog::PitchDialog (QWidget *parent,int rate): QDialog(parent, 0,true)
{
  setCaption	(klocale->translate("Select frequency range :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  high=  new KIntegerLine (this);
  low =  new KIntegerLine (this);
  adjust=new KIntegerLine (this);
  highlabel = new QLabel (klocale->translate("Highest Freq. in Hz"),this);
  lowlabel  = new QLabel (klocale->translate("Lowest Freq. in Hz"),this);
  octave    = new QCheckBox  (klocale->translate("avoid octave jumps with twiddle factor"),this);
  octave->setChecked (true);

  low->setValue    (200);
  high->setValue   (2000);
  adjust->setValue (50);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*6);
  resize	 (320,bsize*6);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();

  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int PitchDialog::getAdjust ()
{
  int i=adjust->value();
  if (i<0) i=0;
  return i;
}
//**********************************************************
int PitchDialog::getLow ()
{
  return low->value();
}
//**********************************************************
int PitchDialog::getOctave ()
{
  return octave->isChecked();
}
//**********************************************************
int PitchDialog::getHigh ()
{
  return high->value();
}
//**********************************************************
void PitchDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();

 lowlabel->setGeometry	(8,8,width()/2-8,bsize);
 low->setGeometry       (width()/2,8,width()/2-8,bsize);  
 highlabel->setGeometry	(8,16+bsize,width()/2-8,bsize);
 high->setGeometry      (width()/2,16+bsize,width()/2-8,bsize);  
 octave->setGeometry	(8,24+bsize*2,width()*3/4-8,bsize);
 adjust->setGeometry    (width()*3/4,24+bsize*2,width()/4-8,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
PitchDialog::~PitchDialog ()
{
}
//**********************************************************
HullCurveDialog::HullCurveDialog (QWidget *parent,char *name): QDialog(parent,0,true)
{
  Interpolation interpolation(0);

  setCaption	(name);
  timelabel	=new QLabel	(klocale->translate("Points are taken 20 times/s"),this);
  timeslider	=new KwaveSlider (1,1000,1,10,KwaveSlider::Horizontal,this);     
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
DistortDialog::DistortDialog (QWidget *parent): QDialog(parent,0,true)
{
  setCaption	(klocale->translate("Choose Distortion-Line :"));

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  curve= new CurveWidget (this);
  curve->setBackgroundColor (QColor(black) );

  sym=new QComboBox (this);
  sym->insertStrList (symtext,-1);

  xscale=new ScaleWidget (this);
  yscale=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
QList<CPoint> *DistortDialog::getPoints ()
{
  return curve->getPoints();
}
//**********************************************************
int DistortDialog::getInterpolationType ()
{
  return curve->getInterpolationType();
}
//**********************************************************
int DistortDialog::getSymmetryType ()
{
  return sym->currentItem();
}
//**********************************************************
void DistortDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int h=height()-bsize*4;

 curve->setGeometry	(8+bsize,0,width()-16-bsize,h);
 xscale->setGeometry	(8+bsize,h,width()-16-bsize,bsize);  
 corner->setGeometry	(8,h,bsize,bsize);   
 yscale->setGeometry	(8,0,bsize,h);
 sym->setGeometry	(8,height()-bsize*3,width()-16,bsize);
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
DistortDialog::~DistortDialog ()
{
  delete curve ;
}
//**********************************************************
AverageDialog::AverageDialog (QWidget *parent,char *name): QDialog(parent, 0,true)
{
  resize  (320,200);
  setCaption	(name);
  taplabel	=new QLabel	 (klocale->translate("# of Filter Taps :"),this);
  typelabel	=new QLabel	 (klocale->translate("Filter Type :"),this);
  taps       	=new KIntegerLine(this);     

  taps->setValue (3);
  type	=new QComboBox  (false,this);
  type->insertStrList   (typetext,-1);

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9/2);
  resize (320,bsize*9/2);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int AverageDialog::getTaps ()
{
  return taps->value();
}
//**********************************************************
int AverageDialog::getType ()
{
  return type->currentItem();
}
//**********************************************************
void AverageDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();

  taplabel->setGeometry	(8,8,width()*5/10-8,bsize);  
  taps->setGeometry     (width()/2,8,width()*5/10-8,bsize);  
  typelabel->setGeometry(8,bsize*3/2,width()*5/10-8,bsize);  
  type->setGeometry     (width()/2,bsize*3/2,width()*5/10-8,bsize);  

  ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
  cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AverageDialog::~AverageDialog ()
{
}
//**********************************************************
AverageFFTDialog::AverageFFTDialog (QWidget *parent,int len,int rate,char *name): QDialog(parent, 0,true)
{
  WindowFunction w(0);
  this->rate=rate;
  this->length=len;
  resize 	(320,200);
  setCaption	(name);

  pointlabel	=new QLabel	(klocale->translate("Length of window :"),this);
  points	=new TimeLine (this);
  points->setMs (100);

  windowtypebox	=new QComboBox (true,this);
  windowtypebox->insertStrList (w.getTypes(),w.getCount());
  QToolTip::add(windowtypebox,klocale->translate("Choose windowing function here."));
  
  windowtypelabel=new QLabel	(klocale->translate("Window Function :"),this);

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize (320,bsize*3);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
int AverageFFTDialog::getWindowType ()
{
  return windowtypebox->currentItem();
}
//**********************************************************
int AverageFFTDialog::getPoints ()
{
  return points->getValue();
}
//**********************************************************
void AverageFFTDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();

  pointlabel->setGeometry  (8,	bsize/2,width()/2-8,bsize);  
  points->setGeometry      (width()/2,	bsize/2,width()*3/10,bsize);  

  windowtypelabel->setGeometry(8,	bsize*3/2+8,width()/2-8,bsize);  
  windowtypebox->setGeometry (width()/2,bsize*3/2+8,width()/2-8,bsize);  

  ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
  cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
AverageFFTDialog::~AverageFFTDialog ()
{
}
//**********************************************************
SonagramDialog::SonagramDialog (QWidget *parent,int len,int rate,char *name): QDialog(parent, 0,true)
{
  WindowFunction w(0);
  this->rate=rate;
  this->length=len;
  resize 	(320,200);
  setCaption	(name);
  pointlabel	=new QLabel	(klocale->translate("Number of FFT points:"),this);
  pointbox	=new QComboBox  (true,this);
  pointbox->insertStrList (FFT_Sizes,-1);
  QToolTip::add(pointbox,klocale->translate("Try to choose numbers with small prime-factors, if choosing big window sizes.\nThe computation will be much faster !"));

  windowtypebox	=new QComboBox (true,this);
  windowtypebox->insertStrList (w.getTypes(),w.getCount());
  QToolTip::add(windowtypebox,klocale->translate("Choose windowing function here. If fourier transformation should stay reversible, use the type <none>"));
  
  windowlabel	=new QLabel	("",this);
  bitmaplabel	=new QLabel	("",this);
  pointslider	=new KwaveSlider (2,(len/16),1,5,KwaveSlider::Horizontal,this);
  windowtypelabel=new QLabel	(klocale->translate("Window Function :"),this);

  setPoints (50);
  setBoxPoints (0);

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*8);
  resize (320,bsize*8);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect	(pointslider,SIGNAL(valueChanged(int)),SLOT(setPoints(int)));
  connect	(pointbox,SIGNAL   (activated(int))   ,SLOT(setBoxPoints(int)));
}
//**********************************************************
int SonagramDialog::getWindowType ()
{
  return windowtypebox->currentItem();
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
  sprintf (buf,klocale->translate("size of bitmap: %dx%d"),(length/points)+1,points/2);  
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

 pointlabel->setGeometry  (8,	bsize/2,width()/2-8,bsize);  
 pointbox->setGeometry    (width()/2,	bsize/2,width()*3/10,bsize);  
 windowlabel->setGeometry (8,	bsize*3/2,width()*8/10,bsize);  
 bitmaplabel->setGeometry (8,	bsize*5/2,width()/2-8,bsize);  
 pointslider->setGeometry (width()/2,	bsize*5/2,width()/2-8,bsize);  
 windowtypelabel->setGeometry(8,	bsize*4,width()/2-8,bsize);  
 windowtypebox->setGeometry (width()/2,	bsize*4,width()/2-8,bsize);  
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
MarkerTypeDialog::MarkerTypeDialog (QWidget *parent,char *name): QDialog(parent, 0,true)
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
extern const char *allow_double;
DoubleEnterDialog::DoubleEnterDialog (QWidget *parent,char *title,double init): QDialog(parent, 0,true)
{
  char buf[64];
  setCaption	(title);

  val=new KRestrictedLine (this);
  val->setValidChars (allow_double);
  sprintf (buf,"%f",init);
  val->setText (buf);

  ok	 = new QPushButton (OK,this);
  cancel = new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();
  int lsize=val->sizeHint().height();
  setMinimumSize (320,lsize*2+bsize);
  resize	 (320,lsize*2+bsize);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  val->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
double DoubleEnterDialog::value ()
{
  return strtod (val->text(),0);
}
//**********************************************************
void DoubleEnterDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int lsize=val->sizeHint().height();

 val->setGeometry	(width()/20,0,width()*18/20,lsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
StringEnterDialog::~StringEnterDialog ()
{
}
//**********************************************************
StringEnterDialog::StringEnterDialog (QWidget *parent,char *title,char *init): QDialog(parent, 0,true)
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
DoubleEnterDialog::~DoubleEnterDialog ()
{
}
//**********************************************************
PercentDialog::PercentDialog (QWidget *parent,char *title): QDialog(parent, 0,true)
{
  setCaption	(title);

  slider=new KwaveSlider (1,1000,1,500,KwaveSlider::Horizontal,this);
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
FrequencyDialog::FrequencyDialog (QWidget *parent,int rate,char *title): QTabDialog(parent, 0,true)
{
  setCaption	(title);
  this->rate=rate;

  fixed=new FixedFrequencyDialog (this,rate,"Choose Frequency");
  sweep=new SweepDialog (this,rate,"Choose Parameters");
  connect 	(this	,SIGNAL(selected(const char *)),SLOT (setSelected(const char *)));
  addTab (fixed,klocale->translate("Fixed"));
  addTab (sweep,klocale->translate("Sweep"));
}
//**********************************************************
void FrequencyDialog::setSelected (const char *type)
{
  this->type=type;
}
//**********************************************************
FrequencyDialog::~FrequencyDialog ()
{
}
//**********************************************************
QList<CPoint> *FrequencyDialog::getFrequency ()
//gets frequency list based on the selected tab and gives it back to caller
{
  QList<CPoint> *times=new QList<CPoint>;

  if (times)
    {
       if (strcmp (type,klocale->translate("Fixed"))==0)
	{
	  CPoint *newpoint=new CPoint;
	
	  newpoint->x=(double)fixed->getTime();
	  newpoint->y=(double)fixed->getFrequency();
	  times->append (newpoint);
	}
      else
      if (strcmp (type,klocale->translate("Sweep"))==0)
	  {
	    double lowfreq=sweep->getLowFreq();
	    double highfreq=sweep->getHighFreq();
	    double freq;
	    Interpolation interpolation(sweep->getInterpolationType());

	    interpolation.prepareInterpolation (sweep->getPoints());

	    double time=sweep->getTime();
	    double count=0;
            
	    while (count<time)
	      {
		double dif=count/time;
		dif=interpolation.getSingleInterpolation (dif);
		freq=lowfreq+(highfreq-lowfreq)*dif;

		CPoint *newpoint=new CPoint;

		newpoint->x=1;
		newpoint->y=floor (rate/freq);
		count+=newpoint->y;
		times->append (newpoint);
	      }
	  }
    }
  return times;
}
//**********************************************************
void FrequencyDialog::resizeEvent (QResizeEvent *)
{
}
//**********************************************************
FixedFrequencyDialog::FixedFrequencyDialog (QWidget *parent,int rate,char *title): QWidget (parent, 0)
{
  setCaption	(title);
  this->rate=rate;
  frequencyslider=new KwaveSlider (1,(rate/2)*9/10,1,440   ,KwaveSlider::Horizontal,this);
  frequencylabel =new QLabel (klocale->translate("Base: 440 Hz"),this);

  timeslider=new KwaveSlider (1,10000,1,100,KwaveSlider::Horizontal,this);
  timelabel =new QLabel (klocale->translate("Time: 1 ms"),this);

  showTime (100);

  QPushButton button(this);
  int bsize=button.sizeHint().height();

  setMinimumSize (320,bsize*5);
  resize	 (320,bsize*5);

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
  QPushButton button(this);
  int bsize=button.sizeHint().height();
  int offset=bsize/2;

  timelabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
  timeslider->setGeometry(width()/2,offset,width()*9/20,bsize);
  offset+=bsize*3/2;
  frequencylabel->setGeometry	(width()/20,offset,width()*8/20,bsize);  
  frequencyslider->setGeometry	(width()/2,offset,width()*9/20,bsize);  
}
//**********************************************************
FixedFrequencyDialog::~FixedFrequencyDialog ()
{
}
//**********************************************************
SweepDialog::SweepDialog (QWidget *parent,int rate,char *title): QWidget(parent, 0)
{
  setCaption	(title);
  this->rate=rate;

  curve=new CurveWidget (this);

  time=      new TimeLine (this,rate);
  timelabel =new QLabel (klocale->translate("Time:"),this);
  time->setMs (5000);

  lowfreq=new KIntegerLine (this);
  highfreq=new KIntegerLine (this);

  note1=new QComboBox  (true,this);
  note1->insertStrList (notetext,-1);
  note2=new QComboBox  (true,this);
  note2->insertStrList (notetext,-1);
  note1->setCurrentItem (12);
  note2->setCurrentItem (64);
  showLowFreq (12);
  showHighFreq (64);

  notelabel1=new QLabel (klocale->translate("From"),this);
  notelabel2=new QLabel (klocale->translate("to"),this);

  showTime (100);

  load=new QPushButton (klocale->translate("Import"),this);
  int bsize=load->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  connect 	(load,SIGNAL(clicked()),SLOT (import()));
  connect 	(note1,SIGNAL(activated(int)),SLOT (showLowFreq(int)));
  connect 	(note2,SIGNAL(activated(int)),SLOT (showHighFreq(int)));
}
//**********************************************************
void SweepDialog::convert (QList<CPoint> *times)
//converts file format list to standard list with points with  range [0-1] for
//frequency (y) and time (x)
{
  double time=times->last()->x;
  double max=0;
  double min=20000;

  //get maximum and minim...
  for (CPoint *tmp=times->first();tmp!=0;tmp=times->next())
    {
      if (tmp->y>max) max=ceil(tmp->y);
      if (tmp->y<min) min=floor(tmp->y);
    }
  this->time->setMs (time);

  //rescale points for use with curvewidget...
  for (CPoint *tmp=times->first();tmp!=0;tmp=times->next())
    {
      tmp->x/=time;
      tmp->y=(tmp->y-min)/(max-min);
    }

  curve->setCurve (times);

  lowfreq->setValue (min);
  highfreq->setValue (max);
}
//**********************************************************
void SweepDialog::import ()
//reads file as generated by label->savefrequency into list
{
  QString name=QFileDialog::getOpenFileName (0,"*.dat",this);
  if (!name.isNull())
    {
      QList<CPoint> *times=new QList<CPoint>;
      CPoint *newpoint;
      QFile in(name.data());
      if (times&&in.exists())
	{
	  if (in.open(IO_ReadWrite))
	    {
	      CPoint *lastpoint=0;

	      char buf[80];
	      float time;
	      float freq;

	      times=new QList<CPoint>;

	      while (in.readLine(&buf[0],80)>0)
		{
		  if ((buf[0]!='#')&&(buf[0]!='/'))
		    //check for commented lines
		    {
		      char *p;
		      sscanf (buf,"%e",&time);
		      p=strchr (buf,'\t');
		      if (p==0) p=strchr (buf,' ');
		      sscanf (p,"%e",&freq);

		      newpoint=new CPoint;

		      newpoint->x=(double)time;
		      newpoint->y=(double)freq;

		      
		      times->append (newpoint);
		      lastpoint=newpoint;
		    }
		}
	      convert (times);
	      delete times;
	    }
	  else printf (klocale->translate("could not open file !\n"));
	}
      else printf (klocale->translate("file %s does not exist!\n"),name.data());
    }
}
//**********************************************************
int SweepDialog::getTime (){return time->getValue();}
//**********************************************************
void SweepDialog::showLowFreq (int val)
{
  char buf[16];

  sprintf (buf,"%4.2f Hz",notefreq[val]);
  lowfreq->setText (buf);
}
//**********************************************************
void SweepDialog::showHighFreq (int val)
{
  char buf[16];

  sprintf (buf,"%4.2f Hz",notefreq[val]);
  highfreq->setText (buf);
}
//**********************************************************
double SweepDialog::getLowFreq ()
{
  QString tmp(lowfreq->text ());
  return (tmp.toDouble());
}
//**********************************************************
int SweepDialog::getInterpolationType() {return curve->getInterpolationType();}
QList<CPoint> *SweepDialog::getPoints (){return curve->getPoints();}
//**********************************************************
double SweepDialog::getHighFreq ()
{
  QString tmp(highfreq->text ());
  return (tmp.toDouble());
}
//**********************************************************
void SweepDialog::showTime (int)
{
}
//**********************************************************
void SweepDialog::resizeEvent (QResizeEvent *)
{
  int bsize=load->sizeHint().height();
  int offset=height()-bsize*2-8;
  notelabel1->setGeometry(8,offset+bsize/2,width()*3/20,bsize);
  notelabel2->setGeometry(width()*11/20,offset+bsize/2,width()*3/20,bsize);  
  note1->setGeometry	 (width()*4/20,offset,width()*5/20,bsize);  
  note2->setGeometry	 (width()*14/20,offset,width()*5/20,bsize);  
  lowfreq->setGeometry	 (width()*4/20,offset+bsize,width()*5/20,bsize);  
  highfreq->setGeometry	 (width()*14/20,offset+bsize,width()*5/20,bsize);  
  offset-=bsize*5/4;
  time->setGeometry      (width()*4/20,offset,width()*5/20,bsize);  
  timelabel->setGeometry (8,offset,width()*3/20-8,bsize);
  load->setGeometry      (width()*14/20,offset,width()*5/20,bsize);  

  offset-=bsize*1/4;
  curve->setGeometry(8,8,width()-16,offset-8);
}
//**********************************************************
SweepDialog::~SweepDialog ()
{
}
//**********************************************************
StutterDialog::StutterDialog (QWidget *parent,int rate,char *name): QDialog(parent, 0,true)
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
QuantiseDialog::QuantiseDialog (QWidget *parent,char *name): QDialog(parent, 0,true)
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
MarkSaveDialog::MarkSaveDialog (QWidget *parent,char *name,bool multi): QDialog(parent, 0,true)
  //dialog of markertypes to be selected...
  //multi is true, if Multiselection shall be enabled....
{
  setCaption	(name);

  save=new QListBox (this);

  MarkerType *act;
  //traverse all markertypes, add them to widget, selection state set to  false
  for (act=markertypes->first();act;act=markertypes->next())
    {
      save->insertItem (act->name->data());
      act->selected=false;
    }

  save->setMultiSelection (multi);
  
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
MarkSignalDialog::MarkSignalDialog (QWidget *parent,int rate,char *name): QDialog(parent, 0,true)
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
  amplslider=new KwaveSlider (1,1000,1,100,KwaveSlider::Horizontal,this);
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
SaveBlockDialog::SaveBlockDialog (QWidget *parent,char *dname): QDialog(parent, 0,true)
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
ChannelMixDialog::ChannelMixDialog (QWidget *parent,int channels): QDialog(parent, 0,true)
{
  dbmode=true;
  tflag=false;

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  int bsize=ok->sizeHint().height();


  this->channels=channels;
  channelname=new QLabel *[channels]; 
  value=new double[channels];
  valuebox=new FloatLine *[channels]; 
  slider=new KwaveSlider *[channels];
  
  tochannellabel=new QLabel (klocale->translate("mix to channel :"),this);
  tochannel=new QComboBox (this);
  usedb=new QCheckBox (klocale->translate ("use dB scale"),this);
  usedb->setChecked (true);
  tochannel->insertItem (klocale->translate("none"));

  if (slider&&channelname&&value&&valuebox)
    for (int i=0;i<channels;i++)
      {
	value[i]=0;
	char buf [32];
	sprintf (buf,"Channel %d :",i+1);
	channelname[i]=new QLabel (buf,this);
	slider[i]=new KwaveSlider (-24*60,60,1,-24*60,KwaveSlider::Horizontal,this);
	valuebox[i]=new FloatLine (this);
	connect (valuebox[i],SIGNAL(textChanged(const char *)),this,SLOT(setValue(const char *)));
	connect (slider[i],SIGNAL(valueChanged(int)),this,SLOT(setValue(int)));
	sprintf (buf,"%d",i+1);
	tochannel->insertItem (buf);
      }
  setValue (-24*60);
   
  setMinimumSize (bsize*8,(bsize+8)*(4+channels));
  resize (bsize*8,(bsize+8)*(4+channels));

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect (ok	,SIGNAL(clicked()),SLOT (accept()));
  connect (cancel,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
void ChannelMixDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int y=8;

 if (slider&&channelname&&valuebox)
   {
     int labelsize=channelname[channels-1]->sizeHint().width();

     for (int i=0;i<channels;i++)
       {
	 channelname[i]->setGeometry	(8,y,labelsize,bsize);  
	 slider[i]->setGeometry	        (labelsize+16,y,width()*8/10-labelsize-16,bsize);  
	 valuebox[i]->setGeometry       (width()*8/10+8,y,width()*2/10-16,bsize);
	 y+=bsize+8;
       }
     tochannellabel->setGeometry (8,y,width()*8/10-16,bsize);  
     tochannel->setGeometry      (8+width()*8/10,y,width()*2/10-16,bsize);  
     y+=bsize+8;
     usedb->setGeometry (8,y,width()-16,bsize);  
   }

 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
void ChannelMixDialog::setValue (int val)
{
  if (!tflag)
    for (int i=0;i<channels;i++)
      if (slider[i]->value()==val)
	{
	  char buf[32];

	  if (dbmode)
	    {
	      if (val==-144*60) strcpy (buf,"-Inf dB");
	      else
		sprintf (buf,"%.1f dB",((double)val)/10);
	    }
	  else sprintf (buf,"%.1f %%",((double)val)/10);

	  valuebox[i]->setText (buf);
	}
}
//**********************************************************
void ChannelMixDialog::setValue (const char* text)
{
  tflag=true;
  for (int i=0;i<channels;i++)
    if (strcmp(valuebox[i]->text(),text)==0)
      {
	double val=strtod (text,0);

	if (dbmode) slider[i]->setValue (-(int)(val*10));
	else slider[i]->setValue ((int)(val*10));
      }
  tflag=false;
}
//**********************************************************
void ChannelMixDialog::setdBMode ()
{
  dbmode=!dbmode;

  if (dbmode)
    {
      //convert values...

    }
}
//**********************************************************
ChannelMixDialog::~ChannelMixDialog ()
{
  if (slider) delete slider;
  if (channelname) delete channelname;
  if (value) delete value;
  if (valuebox) delete valuebox;
}





