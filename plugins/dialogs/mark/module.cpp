#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <qkeycode.h>
#include <libkwave/markers.h>
#include <libkwave/globals.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="amptolabel";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new MarkSignalDialog(operation->getGlobals(),operation->getRate(),operation->isModal());
}
//**********************************************************
MarkSignalDialog::MarkSignalDialog (const Global *globals,int rate,bool modal): KwaveDialog(modal)
{
  comstr=0;
  setCaption	(klocale->translate("Choose labeling criteria"));
  tflag=false;

  ok		=new QPushButton (OK,this);
  cancel       	=new QPushButton (CANCEL,this);

  mark1=new QLabel (klocale->translate("Start label:"),this);
  mark2=new QLabel (klocale->translate("Stop label:"),this);

  timelabel=new QLabel (klocale->translate("Length of silence:"),this);
  time=new TimeLine (this,rate);
  QToolTip::add( time, klocale->translate("this is the timespan below the defined sound level\nthat is assumed to separate two signals ..."));
  time->setMs (400);
  ampllabel=new QLabel (klocale->translate("Max. silence level in %"),this);
  amplslider=new KwaveSlider (1,1000,1,100,KwaveSlider::Horizontal,this);
  ampl=new FloatLine (this);
  ampl->setText ("10.0");

  marktype1=new QComboBox (false,this);
  marktype2=new QComboBox (false,this);
  const MarkerType *act;
  int cnt=0;

  for (act=(globals->markertypes).first();act;act=(globals->markertypes).next())
    {
      marktype1->insertItem (act->name);
      marktype2->insertItem (act->name);

      //just for convenience check for these to label names
      if (strcasecmp("start",act->name)==0) marktype1->setCurrentItem (cnt);
      if (strcasecmp("stop",act->name)==0) marktype2->setCurrentItem (cnt);
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
const char *MarkSignalDialog::getCommand ()
{
  deleteString (comstr);
  comstr=catString ("amptolabel (100.0",
		    ",", //level
		    marktype1->currentText(),
		    ",",
		    marktype2->currentText(),
		    ",",
		    time->getMsStr(),
		    ")");
  return comstr;
};
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
  deleteString (comstr);
}
//**********************************************************













