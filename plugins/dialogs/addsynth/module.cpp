#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>
#include <math.h>
#include <limits.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <libkwave/Signal.h>
#include <libkwave/Functions.h>
#include <libkwave/Curve.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/DynamicLoader.h>
#include "../../../libgui/Dialog.h"
#include "../../../libgui/ScaleWidget.h"

#include <kintegerline.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="addsynth";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new AddSynthDialog(operation->getRate(),operation->getLength(),operation->isModal());
}
//**********************************************************
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
AddSynthDialog::AddSynthDialog (int rate,int length,bool modal): Dialog(modal)
{
  num=10;
  command=0;
  sweep=0;
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


  x=new ScaleWidget (this,0,360,"�");
  y=new ScaleWidget (this,100,-100,"%");
  corner=new CornerPatchWidget (this);

  Functions func;
  this->rate=rate;

  setCaption	("Choose Signal Components:");

  multlab       =new QLabel	(klocale->translate("Multiply :"),this);
  phaselab	=new QLabel	(klocale->translate("Phase in degree:"),this);
  powerlab	=new QLabel	(klocale->translate("Power in %:"),this);

  freqbutton    =new QPushButton(klocale->translate("Frequency"),this);  
  calculate     =new QPushButton(klocale->translate("Calculate"),this);  

  aphase=0;
  apower=0;
  amult=0;

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
  printf ("returned\n");
  powerlabel [0]->setValue (100);
  printf ("returned\n");

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
const char *AddSynthDialog::getCommand ()
{
/*
  char *tmp;
  char phasestr[128];
  char multstr [128];
  char powstr  [128];

  if (command) deleteString (command);

  int count=getCount();
  sprintf (multstr,"%d",count);

  if (count)
    {
      command=catString ("addsynth(",multstr);

      for (int i=0;i<count;i++)
	{
	  printf ("%s\n",command);
	  printf ("%d\n",amult[i]);
	  printf ("%d\n",apower[i]);
	  printf ("%d\n",aphase[i]);

	  sprintf (multstr, "%f",amult[i]);
	  sprintf (powstr,  "%f",apower[i]);
	  sprintf (phasestr,"%f",aphase[i]);

	  printf ("%s\n",multstr);
	  printf ("%s\n",powstr);
	  printf ("%s\n",phasestr);

	  tmp=command;
	  command=catString(tmp,",",multstr,",",powstr,",");
	  deleteString (tmp);
	  tmp=command;
	  command=catString(tmp,phasestr);
	  deleteString (tmp);
	}

      tmp=command;
      command=catString(tmp,")");
      deleteString (tmp);

      return command;
    }
*/
  return ("requester(not yet implemented !)");
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
  Dialog *dialog =DynamicLoader::getDialog
    ("stringenter",
     new DialogOperation(klocale->translate("Choose dB/octave"),true));

  if ((dialog)&&(dialog->exec()))
    {
      double rise=-strtod(dialog->getCommand(),0);
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
  if (sweep)
    {
      delete sweep;
      sweep=0;
    }
  else
    {
      sweep=DynamicLoader::getDialog
	("sweep",
	 new DialogOperation(true));

      if ((sweep)&&(sweep->exec()))
	{
      
	}
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
bool AddSynthDialog::getNSlider (int n,bool first)
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
  powerlabel=new (KIntegerLine *)[num];
  phaselabel=new (KIntegerLine *)[num];
  mult      =new (KIntegerLine *)[num];
  power     =new (Slider *)[num];
  phase     =new (Slider *)[num];

  if (!(powerlabel&&phaselabel&&mult&&power&&phase)) return false;

  printf ("allocated\n");

  for (int i=0;i<num;i++)
    {
      powerlabel[i]=new KIntegerLine (this);
      powerlabel[i]->setText ("0");
      phaselabel[i]=new KIntegerLine (this);
      phaselabel[i]->setText ("0");
      mult[i]      =new KIntegerLine (this);
      mult[i]->setValue (i+1);
    }

  printf ("label and mult\n");

  for (int i=0;i<num;i++)
    {
      power[i]=new Slider (0,1000,1,0  ,Slider::Horizontal,this);
      phase[i]=new Slider (-180,179,1,0,Slider::Horizontal,this);
    }
  printf ("slider\n");

  power[0]->setValue (1000);

  int bsize=ok->sizeHint().height();
  int lsize=powerlab->sizeHint().height();
  int nsize=powerlabel[0]->sizeHint().height();
  int toppart=lsize*12;

  if (!first) resize (bsize*16,(nsize*(num+1)+bsize*2)+toppart);

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
  printf ("shown\n");
  return true;
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
Signal *AddSynthDialog::getSignal ()
//calculates final signal from choosed parameters...
{
  Functions foo;
  int count=getCount();
  int *phase=aphase;
  int *mult=amult;
  int *power=apower;

  if (times)
    {
      Point *t;
      int len=0;
      int dif;
      int lastz;

      //count number of samples
      for (t=times->first();t;t=times->next(t)) len+=int (t->x*t->y);

      //get new signal
      Signal *add=new Signal (len,rate);

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
	    for (t=times->first();t;t=times->next(t))
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
  deleteString (command);
  if (powerlabel) delete powerlabel;
  if (phaselabel) delete phaselabel;
  if (apower) delete apower;
  if (amult)  delete amult;
  if (aphase) delete aphase;
  if (times) delete times;
  if (sweep) delete sweep;
}
//**********************************************************



































