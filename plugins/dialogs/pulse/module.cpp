#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qlabel.h>

#include <kapp.h>

#include <libkwave/Curve.h>
#include <libkwave/Signal.h>
#include <libkwave/Functions.h>
#include <libkwave/Interpolation.h>
#include "libgui/ScaleWidget.h"
#include "libgui/TimeLine.h"
#include "libgui/Dialog.h"
#include "libgui/CurveWidget.h"
#include "libgui/CornerPatchWidget.h"

#include "module.h"

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="pulse";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new PulseDialog(operation->getRate(),operation->getLength(),operation->isModal());
}
//**********************************************************
PulseDialog::PulseDialog (int rate,int time,bool modal): Dialog(modal)
{
  x=new ScaleWidget (this,0,360,"°");
  y=new ScaleWidget (this,100,-100,"%");
  corner=new CornerPatchWidget (this);

  pulse=new CurveWidget (this,"curve (linear, 0 , 0.5, 0.5, 1, 1,.5)");

  Functions func;
  this->rate=rate;

  setCaption	("Choose pulse properties :");
  //  freqbutton    =new QPushButton(i18n("Frequency"),this);
  pulselabel    =new QLabel	(i18n("Length of pulse :"),this);
  pulselength   =new TimeLine (this,rate);
  pulselength->setMs (5);

  ok		=new QPushButton ("Ok",this);
  cancel	=new QPushButton ("Cancel",this);

  char buf[512];
  sprintf (buf,"curve (linear,100, %f)",((double)rate)/440);
  times=new Curve (buf);

  int bsize=ok->sizeHint().height();
  setMinimumSize (bsize*12,bsize*10);
  resize (bsize*12,bsize*10);

  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
  //  connect 	(freqbutton,SIGNAL(clicked()),SLOT(getFrequency()));
}
//**********************************************************
const char *PulseDialog::getCommand ()
{
  return "requester(Sorry : not implemented yet!)";
}
//**********************************************************
void PulseDialog::getFrequency ()
{
  //  FrequencyDialog *dialog=new FrequencyDialog(this);

  //  if (dialog)
  // if (dialog->exec())
  //  {
  //    if (times) delete times;
  //     times=dialog->getFrequency ();
  //  }
}
//**********************************************************
Signal *PulseDialog::getSignal ()
//calculates final signal from choosed parameters...
{
  if (times)
    {
      Point *t;
      int len=0;
      int pulselen=pulselength->getValue();

      Curve *points=new Curve(pulse->getCommand ());
      Interpolation interpolation (0);
      double *tmp=interpolation.getInterpolation (points,pulselen);
      int    *pulse=new int[pulselen];

      //count number of samples
      for (t=times->first();t;t=times->next(t)) len+=int (t->x*t->y);

      //get new signal
      Signal *add=new Signal (len,rate);

      if (pulse&&add&&add->getSample()&&len);
      {
	int min;
	int cnt=0;
	int i,j;
	int *sample=add->getSample();
	for (int i=0;i<pulselen;i++) pulse[i]=(int)((tmp[i]-.5)*((1<<24)-1));
	for (t=times->first();t;t=times->next(t))
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

  //  freqbutton->setGeometry (8,height-bsize*3,width*3/10-10,bsize);  
  pulselabel->setGeometry (width*5/10+2,height-bsize*3,width*3/10-2,bsize);  
  pulselength->setGeometry (width*8/10,height-bsize*3,width*2/10-8,bsize);  

  ok->setGeometry	(width/10,height-bsize*3/2,width*3/10,bsize);  
  cancel->setGeometry	(width*6/10,height-bsize*3/2,width*3/10,bsize);  
}
//**********************************************************
















