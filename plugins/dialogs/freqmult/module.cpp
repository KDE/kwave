#include <stdio.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="freqmult";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new FrequencyMultDialog(operation->getRate(),operation->isModal());
}
//**********************************************************
FrequencyMultDialog::FrequencyMultDialog (int rate,bool modal): Dialog(modal)
{
  comstr=0;
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
const char *FrequencyMultDialog::getCommand ()
{
  deleteString (comstr);
  comstr=catString ("multiply (",curve->getCommand(),")");

  return comstr;
}
//**********************************************************
void FrequencyMultDialog::addPoint ()
{
  double freq=((double)x->value())/rate;
  double amp=((double)y->value())/100;
  curve->addPoint (freq,amp);
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
  deleteString (comstr);
  delete curve ;
}
//**********************************************************













