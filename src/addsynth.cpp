#include <math.h>
#include <limits.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qlayout.h>
#include "sample.h"
#include "guiitems.h"
#include "addsynth.h"
#include "fftview.h"
#include "functions.h"
#include "dialogs.h"

extern int getMaxPrimeFactor (int);
//****************************************************************************
AddSynthWidget::AddSynthWidget (QWidget *parent) : QWidget
(parent)
{
  this->setBackgroundColor (QColor(black));    
  count=0;
  func=0;
}
//****************************************************************************
void AddSynthWidget::setSines (int count,int *power,int *phase,int *mult)
{
  this->count=count;
  this->power=power;
  this->phase=phase;
  this->mult=mult;
  repaint ();
}
//****************************************************************************
void AddSynthWidget::setFunction (int func)
{
  this->func=func;
  repaint ();
}
//****************************************************************************
AddSynthWidget::~AddSynthWidget ()
{
}
//****************************************************************************
void AddSynthWidget::paintEvent  (QPaintEvent *)
{
  Functions foo;
  double (*func)(double)=(double (*)(double))foo.getFunc(this->func);

  int height=rect().height()/2;
  int width=rect().width();
  QPainter p;

  p.begin (this);
  p.setPen (white);
  if (count)
    {
      int i,ay,ly;
      double max=0,y=0,x;
      double *dphase=new double [count];

      if (dphase)
	{
	  for (int j=0;j<count;j++)
	    dphase[j]=((((double)phase[j])/180)*M_PI/mult[j]);

	  for (int j=0;j<count;j++) max+=(double) power[j];
	  max/=1000;

	  x=0;
	  for (int j=0;j<count;j++)
	    y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

	  ly=(int)(y/max*height)+height;

	  for (i=1;i<width;i++)
	    {
	      x=((double)i)/width*2*M_PI;
	      y=0;
	      for (int j=0;j<count;j++)
		y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

	      ay=(int)(y/max*height)+height;
	      p.drawLine (i-1,ly,i,ay);
	      ly=ay;
	    }
	  delete dphase;
	  
	  p.setPen (green);
	  p.drawLine (0,height,width,height);
	}
    }
  p.end();
}
//**********************************************************
AddSynthDialog::AddSynthDialog (QWidget *par,int rate,int time,char *name): QDialog(par,0,true)
{
  num=10;
  menu=new QPopupMenu ();
  QPopupMenu *multmenu=new QPopupMenu();
  QPopupMenu *phasemenu=new QPopupMenu();
  QPopupMenu *powermenu=new QPopupMenu();
  tflag=false;

  menu->insertItem (klocale->translate("&Power"),powermenu);
  menu->insertItem (klocale->translate("&Phase"),phasemenu);
  menu->insertItem (klocale->translate("&Multipliers"),multmenu);
  multmenu->insertItem (klocale->translate("e&numerate"),this,SLOT(enumerateMult()));
  multmenu->insertItem (klocale->translate("&even"),this,SLOT(evenMult()));
  multmenu->insertItem (klocale->translate("&odd"),this,SLOT(oddMult()));
  multmenu->insertItem (klocale->translate("&prime"),this,SLOT(primeMult()));
  multmenu->insertItem (klocale->translate("&fibonacci"),this,SLOT(fibonacciMult()));
  powermenu->insertItem (klocale->translate("&max"),this,SLOT(maxPower()));
  powermenu->insertItem (klocale->translate("&db/octave"),this,SLOT(dbPower()));
  powermenu->insertItem (klocale->translate("&invert"),this,SLOT(invertPower()));
  powermenu->insertItem (klocale->translate("&Sinus"),this,SLOT(sinusPower()));
  powermenu->insertItem (klocale->translate("&random"),this,SLOT(randomizePower()));
  phasemenu->insertItem (klocale->translate("&zero"),this,SLOT(zeroPhase()));
  phasemenu->insertItem (klocale->translate("&negate"),this,SLOT(negatePhase()));
  phasemenu->insertItem (klocale->translate("&sinus"),this,SLOT(sinusPhase()));
  phasemenu->insertItem (klocale->translate("&random"),this,SLOT(randomizePhase()));

  menu->insertItem (klocale->translate("&Reset all"));

  x=new ScaleWidget (this,0,360,"°");
  y=new ScaleWidget (this,100,-100,"%");
  corner=new CornerPatchWidget (this);

  Functions func;
  this->rate=rate;

  setCaption	(name);
  multlab       =new QLabel	(klocale->translate("Multiply :"),this);
  phaselab	=new QLabel	(klocale->translate("Phase in degree:"),this);
  powerlab	=new QLabel	(klocale->translate("Power in %:"),this);

  freqbutton    =new QPushButton(klocale->translate("Frequency"),this);  
  calculate     =new QPushButton(klocale->translate("Calculate"),this);  

  aphase=0;
  apower=0;
  amult=0;

  powerlabel=new KIntegerLine *[num];
  phaselabel=new KIntegerLine *[num];
  mult      =new KIntegerLine *[num];
  power     =new KwaveSlider *[num];
  phase     =new KwaveSlider *[num];

  functype=new QComboBox  (false,this);
  functype->insertStrList (func.getTypes());

  view=new AddSynthWidget (this);
  connect 	(functype,SIGNAL(activated(int)),view,SLOT (setFunction(int)));

  channel=new KIntegerLine (this);
  channel->setText ("20");
  channellabel=new QLabel (klocale->translate("# of :"),this);

  ok		=new QPushButton ("Ok",this);
  cancel	=new QPushButton ("Cancel",this);

  getNSlider (20,true);
  powerlabel [0]->setValue (100);


  times=new QList<CPoint>;
  CPoint *newpoint=new CPoint;
  newpoint->x=100;
  newpoint->y=(double)rate/440;
  times->append (newpoint);

  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(channel,SIGNAL(textChanged(const char *)),SLOT (setChannels(const char *)));
  connect 	(freqbutton,SIGNAL(clicked()),SLOT (getFrequency()));
  connect 	(calculate,SIGNAL(clicked()),SLOT(popMenu()));
  updateView();
}
//**********************************************************
void AddSynthDialog::oddMult ()
{
  for (int i=0;i<num;i++) mult[i]->setValue (1+i*2);

  updateView ();
}
//**********************************************************
void AddSynthDialog::fibonacciMult ()
{
  int a=1;
  int b=1;
  int x;
  mult[0]->setValue (1);
  for (int i=1;i<num;i++)
    {
      x=a;
      a=b;
      b=x+b;
      mult[i]->setValue (b);
    }

  updateView ();
}
//**********************************************************
void AddSynthDialog::evenMult ()
{
  for (int i=1;i<num;i++) mult[i]->setValue (i*2);
  mult[0]->setValue (1); 

  updateView ();
}
//**********************************************************
void AddSynthDialog::enumerateMult ()
{
  for (int i=0;i<num;i++) mult[i]->setValue (i+1);

  updateView ();
}
//**********************************************************
void AddSynthDialog::primeMult ()
{
  int max=1;
  for (int i=0;i<num;i++)
    {
      while (getMaxPrimeFactor (max)!=max) max++;

      mult[i]->setValue (max);
      max++;
    }
  updateView ();
}
//**********************************************************
void AddSynthDialog::zeroPhase ()
{
  for (int i=0;i<num;i++) phase[i]->setValue (0);

  updateView ();
}
//**********************************************************
void AddSynthDialog::negatePhase ()
{
  for (int i=0;i<num;i++) phase[i]->setValue (-(phase[i]->value()));

  updateView ();
}
//**********************************************************
void AddSynthDialog::sinusPhase ()
{
  for (int i=0;i<num;i++) phase[i]->setValue (179*sin((double) i*2*M_PI/num));

  updateView ();
}
//**********************************************************
void AddSynthDialog::maxPower ()
{
  for (int i=0;i<num;i++) power[i]->setValue (1000);
  updateView ();
}
//**********************************************************
void AddSynthDialog::dbPower ()
{
  DoubleEnterDialog dialog (this,"Choose dB/octave",6);

  if (dialog.exec())
    {
      double rise=-(dialog.value());
      if (rise>0) 
	for (int i=0;i<num;i++)
	  {
	    double mul=1/(pow (2,i*rise/6));
	    power[i]->setValue (mul*1000);
	  }
      else
	for (int i=0;i<num;i++)
	  {
	    double mul=1/(pow (2,i*-rise/6));
	    power[num-i-1]->setValue (mul*1000);
	  }
      updateView ();
    }
}
//**********************************************************
void AddSynthDialog::sinusPower ()
{
  for (int i=0;i<num;i++) power[i]->setValue (500+500*sin((double) i*2*M_PI/num));
  updateView ();
}
//**********************************************************
void AddSynthDialog::invertPower ()
{
  for (int i=0;i<num;i++) power[i]->setValue (1000-power[i]->value());
  updateView ();
}
//**********************************************************
void AddSynthDialog::randomizePower ()
{
  for (int i=0;i<num;i++) power[i]->setValue ((int)(drand48()*1000));
  updateView ();
}
//**********************************************************
void AddSynthDialog::randomizePhase ()
{
  for (int i=0;i<num;i++) phase[i]->setValue ((int)(drand48()*360)-180);
  updateView ();
}
//**********************************************************
void AddSynthDialog::popMenu ()
{
  QPoint popup=QCursor::pos();
  menu->popup (popup);
}
//**********************************************************
void AddSynthDialog::getFrequency ()
{
  FrequencyDialog dialog(this,rate);
  if (dialog.exec())
    {
      if (times) delete times;
      times=dialog.getFrequency ();
    }
}
//**********************************************************
void AddSynthDialog::setChannels (const char *n)
{
  int x=1;
  if (n) x=strtol(n,0,0);
  if (x<1) x=1;
  if (x>100) x=100;         //feel free to change, if you have a big screen ...
  if (x!=num) getNSlider (x,false);
}
//**********************************************************
void AddSynthDialog::getNSlider (int n,int first)
{
  if (!first)
    {
      for (int i=0;i<num;i++)
	{
	  delete powerlabel[i];
	  delete phaselabel[i];
	  delete power[i];
	  delete phase[i];
	  delete mult[i];
	}
      delete powerlabel;
      delete phaselabel;
      delete power;
      delete phase;
      delete mult;
    }

  num=n;
  powerlabel=new KIntegerLine *[num];
  phaselabel=new KIntegerLine *[num];
  mult      =new KIntegerLine *[num];
  power     =new KwaveSlider *[num];
  phase     =new KwaveSlider *[num];

  for (int i=0;i<num;i++)
    {
      powerlabel[i]=new KIntegerLine (this);
      powerlabel[i]->setText ("0");
      phaselabel[i]=new KIntegerLine (this);
      phaselabel[i]->setText ("0");
      mult[i]      =new KIntegerLine (this);
      mult[i]->setValue (i+1);
    }
  for (int i=0;i<num;i++)
    {
      power[i]=new KwaveSlider (0,1000,1,0  ,KwaveSlider::Horizontal,this);
      phase[i]=new KwaveSlider (-180,179,1,0,KwaveSlider::Horizontal,this);
    }

  power[0]->setValue (1000);
  int bsize=ok->sizeHint().height();
  int lsize=powerlab->sizeHint().height();
  int nsize=powerlabel[0]->sizeHint().height();
  int toppart=lsize*12;

  resize (bsize*16,(nsize*(num+1)+bsize*2)+toppart);

  for (int i=0;i<num;i++)
  {
      power[i]->show();
      phase[i]->show();
      mult[i]->show();
      powerlabel[i]->show();
      phaselabel[i]->show();
      connect  (power[i],SIGNAL(valueChanged(int)),SLOT(showPower(int)));
      connect  (phase[i],SIGNAL(valueChanged(int)),SLOT(showPhase(int)));
      connect  (powerlabel[i],SIGNAL(textChanged(const char *)),SLOT(showPower(const char *)));
      connect  (phaselabel[i],SIGNAL(textChanged(const char *)),SLOT(showPhase(const char *)));
      connect  (mult[i],SIGNAL (textChanged(const char *)),SLOT(showMult(const char *)));
  }
  setMinimumSize (bsize*16,(nsize*(num+1)+bsize*2)+toppart);
}
//**********************************************************
void AddSynthDialog::updateView ()
{
  int count=0;
  int x=0;

  for (int i=0;i<num;i++) if (power[i]->value()>0) count++; //count active elements

  if (apower) delete apower;
  if (amult)  delete  amult;
  if (aphase) delete aphase;

  apower=new int[count]; 
  aphase=new int[count]; 
  amult =new int[count]; 

  for (int i=0;i<num;i++) 
    if (this->power[i]->value()>0)
    {
      apower[x]=power[i]->value();
      amult[x] =mult[i]->value();
      aphase[x]=phase[i]->value();
      x++;
    }
  view->setSines (count,apower,aphase,amult);
}
//**********************************************************
int AddSynthDialog::getCount ()
{
  int count=0;

  for (int i=0;i<num;i++) if (power[i]->value()>0) count++; //count active elements

  return count;
}
//**********************************************************
MSignal *AddSynthDialog::getSignal ()
//calculates final signal from choosed parameters...
{
  Functions foo;
  int count=getCount();
  int *phase=aphase;
  int *mult=amult;
  int *power=apower;

  if (times)
    {
      CPoint *t;
      int len=0;
      int dif;
      int lastz;

      //count number of samples
      for (t=times->first();t;t=times->next()) len+=int (t->x*t->y);

      //get new signal
      MSignal *add=new MSignal ((QWidget *)parent(),len,rate);

      if (add&&add->getSample()&&len);
      {
	int *sample=add->getSample();

	//get pointer to choosed function
	double (*func)(double)=(double (*)(double))foo.getFunc(functype->currentItem());

	if (count)
	  {
	    int i;
	    double max=0,y=0,x;

	    for (int j=0;j<count;j++) max+=(double) power[j];
	    max/=1000;

	    int z=0;
	    for (t=times->first();t;t=times->next())
	      {
		lastz=z;
		for (i=0;i<(int)(t->y);i++)
		  {
		    x=(double(i))/t->y*2*M_PI;
		    y=0;
		    for (int j=0;j<count;j++)
		      y+=(func(x*mult[j]+(((double)phase[j])/180)*M_PI)*power[j])/1000;

		    sample[z++]=(int)(y/max*((1<<23)-1));
		  }
		dif=z-lastz;
		for (i=1;i<(int)t->x;i++) memcpy (&sample[lastz+i*dif],&sample[lastz],dif*sizeof (int));
		z+=(int)((t->x-1)*dif);
	      }
	    return add;
	  }
	if (add) delete add;
      }
    }

  return 0;
}
//**********************************************************
void AddSynthDialog::showPower (const char *str)
{
  for (int i=0;i<num;i++) 
    if (strcmp(powerlabel[i]->text(),str)==0)
      {
	tflag=true;
	power[i]->setValue (strtol(str,0,0)*10);
	tflag=false;
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showMult (const char *str)
{
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPhase (const char *str)
{
  for (int i=0;i<num;i++) 
    if (strcmp(phaselabel[i]->text(),str)==0)
      {
	tflag=true;
	phase[i]->setValue (strtol(str,0,0));
	tflag=false;
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPower (int newvalue)
{
  for (int i=0;i<num;i++) 
    if (power[i]->value()==newvalue)
      if (!tflag)
      {
	char buf[8];
	sprintf (buf,"%d.%d",newvalue/10,newvalue%10);
	powerlabel[i]->setText (buf);
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showPhase (int newvalue)
{
  for (int i=0;i<num;i++) 
    if (phase[i]->value()==newvalue)
      if (!tflag)
      {
	char buf[8];
	sprintf (buf,"%d",newvalue);
	phaselabel[i]->setText (buf);
      }
  updateView ();
}
//**********************************************************
void AddSynthDialog::showMult (int newvalue)
{
}
//**********************************************************
void AddSynthDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();
  int lsize=powerlab->sizeHint().height();
  int width=this->width();
  int height=this->height();
  int nsize=powerlabel[0]->sizeHint().height();
  int toppart=height-bsize*3-num*nsize;
  int offset=0;
  int textx=lsize*3;

  x->setGeometry (8+bsize,toppart-bsize,width*2/3-8-bsize,bsize);
  y->setGeometry (8,0,bsize,toppart-bsize);
  corner->setGeometry (8,toppart-bsize,bsize,bsize);
  view->setGeometry (8+bsize,0,width*2/3-8-bsize,toppart-bsize);

  channellabel->setGeometry (width*2/3+8,lsize/2,width/3-bsize*2-lsize,lsize);
  channel->setGeometry  (width*2/3+8+width/3-16-bsize-lsize,0,bsize+lsize,bsize);
  offset+=bsize+4;
  functype->setGeometry      (width*2/3+8,offset,width/3-16,bsize);
  offset+=bsize+4;
  freqbutton->setGeometry    (width*2/3+8,offset,width/3-16,bsize);
  offset+=bsize+4;
  calculate->setGeometry          (width*2/3+8,offset,width/3-16,bsize);

  powerlab->setGeometry	(8            ,toppart,width/3,lsize);
  phaselab->setGeometry	((width-lsize*3)/2,toppart,width/3,lsize);
  multlab->setGeometry	(width-textx-8,toppart,width/3,lsize);

  for (int i=0;i<num;i++)
    {
      int yoffset=nsize*(i+1)+toppart;
      int w=width-textx-16;

      powerlabel[i]->setGeometry (8,yoffset,textx,nsize);
      phaselabel[i]->setGeometry (8+w/2,yoffset,textx,nsize);
      mult [i]->setGeometry      (8+w,yoffset,textx,nsize);
      power[i]->setGeometry      (10+textx,yoffset+2,w/2-textx-10,nsize-4);
      phase[i]->setGeometry      (10+w/2+textx,yoffset+2,w/2-textx-10,nsize-4);
    }
  width=this->width();

  ok->setGeometry	(width/10,height-bsize*3/2,width*3/10,bsize);  
  cancel->setGeometry	(width*6/10,height-bsize*3/2,width*3/10,bsize);  
}
//**********************************************************
void AddSynthDialog::mousePressEvent( QMouseEvent *e)
{
  if (e->button()==RightButton)
    {
      QPoint popup=QCursor::pos();
      menu->popup(popup);
    }
}
//**********************************************************
AddSynthDialog::~AddSynthDialog ()
{
  if (powerlabel) delete powerlabel;
  if (phaselabel) delete phaselabel;
  if (apower) delete apower;
  if (amult)  delete  amult;
  if (aphase) delete aphase;
  if (times) delete times;
}
//**********************************************************
PulseDialog::PulseDialog (QWidget *par,int rate,int time,char *name): QDialog(par,0,true)
{
  x=new ScaleWidget (this,0,360,"°");
  y=new ScaleWidget (this,100,-100,"%");
  corner=new CornerPatchWidget (this);

  QList<CPoint> init;
  CPoint *newpoint=new CPoint;
  newpoint->x=0;
  newpoint->y=.5;
  init.append (newpoint);
  newpoint=new CPoint;
  newpoint->x=.5;
  newpoint->y=1;
  init.append (newpoint);
  newpoint=new CPoint;
  newpoint->x=1;
  newpoint->y=.5;
  init.append (newpoint);

  pulse=new CurveWidget (this,"",&init);

  Functions func;
  this->rate=rate;

  setCaption	(name);

  freqbutton    =new QPushButton(klocale->translate("Frequency"),this);
  hear          =new QPushButton(klocale->translate("Test"),this);  
  pulselabel    =new QLabel	(klocale->translate("Length of pulse :"),this);
  pulselength   =new TimeLine (this,rate);
  pulselength->setMs (5);

  ok		=new QPushButton ("Ok",this);
  cancel	=new QPushButton ("Cancel",this);

  times=new QList<CPoint>;
  newpoint=new CPoint;
  newpoint->x=100;
  newpoint->y=(double)rate/440;
  times->append (newpoint);

  int bsize=ok->sizeHint().height();
  setMinimumSize (bsize*12,bsize*10);
  resize (bsize*12,bsize*10);

  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  connect 	(freqbutton,SIGNAL(clicked()),SLOT(getFrequency()));
}
//**********************************************************
void PulseDialog::getFrequency ()
{
  FrequencyDialog *dialog=new FrequencyDialog(this);
  if (dialog)
  if (dialog->exec())
    {
      if (times) delete times;
       times=dialog->getFrequency ();
    }
}
//**********************************************************
MSignal *PulseDialog::getSignal ()
//calculates final signal from choosed parameters...
{
  if (times)
    {
      CPoint *t;
      int len=0;
      int pulselen=pulselength->getValue();
      QList<CPoint> *points=this->pulse->getPoints ();
      Interpolation interpolation (this->pulse->getInterpolationType());
      double *tmp=interpolation.getInterpolation (points,pulselen);
      int    *pulse=new int[pulselen];

      //count number of samples
      for (t=times->first();t;t=times->next()) len+=int (t->x*t->y);

      //get new signal
      MSignal *add=new MSignal ((QWidget *)parent(),len,rate);

      if (pulse&&add&&add->getSample()&&len);
      {
	int min;
	int cnt=0;
	int i,j;
	int *sample=add->getSample();
	for (int i=0;i<pulselen;i++) pulse[i]=(int)((tmp[i]-.5)*((1<<24)-1));
	for (t=times->first();t;t=times->next())
	  {
	    for (i=0;i<t->x;i++)
	      {
		min=pulselen;
		if (min>t->y) min=(int)t->y;
		for (j=0;j<min;j++) sample[cnt+j]=pulse[j];
		for (;j<t->y;j++) sample[cnt+j]=0;

		cnt+=(int)t->y;
	      }
	  }

       	return add;
      }
    }
  return 0;
}
//**********************************************************
PulseDialog::~PulseDialog ()
{
  if (times) delete times;
}
//**********************************************************
void PulseDialog::resizeEvent (QResizeEvent *)
{
  int bsize=ok->sizeHint().height();
  int width=this->width();
  int height=this->height();
  int toppart=height-bsize*7/2;

  x->setGeometry (8+bsize,toppart-bsize,width-16-bsize,bsize);
  y->setGeometry (8,0,bsize,toppart-bsize);
  corner->setGeometry (8,toppart-bsize,bsize,bsize);
  pulse->setGeometry  (8+bsize,0,width-16-bsize,toppart-bsize);

  freqbutton->setGeometry (8,height-bsize*3,width*3/10-10,bsize);  
  hear      ->setGeometry (width*3/10,height-bsize*3,width*2/10,bsize);
  pulselabel->setGeometry (width*5/10+2,height-bsize*3,width*3/10-2,bsize);  
  pulselength->setGeometry (width*8/10,height-bsize*3,width*2/10-8,bsize);  

  ok->setGeometry	(width/10,height-bsize*3/2,width*3/10,bsize);  
  cancel->setGeometry	(width*6/10,height-bsize*3/2,width*3/10,bsize);  
}
//**********************************************************
























