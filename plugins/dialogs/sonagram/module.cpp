#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include "module.h"
#include <libkwave/WindowFunction.h>
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="sonagram";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new SonagramDialog(operation->isModal(),operation->getLength(),operation->getRate());
}
//**********************************************************
const char *FFT_Sizes[]={"64","128","256","512","1024","2048","4096",0};
//**********************************************************
SonagramDialog::SonagramDialog (bool modal,int len,int rate)
: Dialog(modal)
{
  comstr=0;
  WindowFunction w(0);
  this->rate=rate;
  this->length=len;
  resize 	(320,200);
  setCaption	(i18n("Set FFT/time resolution parameter :"));
  pointlabel	=new QLabel	(i18n("Number of FFT points:"),this);
  pointbox	=new QComboBox  (true,this);
  pointbox->insertStrList (FFT_Sizes,-1);
  QToolTip::add(pointbox,i18n("Try to choose numbers with small prime-factors, if choosing big window sizes.\nThe computation will be much faster !"));

  windowtypebox	=new QComboBox (true,this);
  windowtypebox->insertStrList (w.getTypes(),w.getCount());
  QToolTip::add(windowtypebox,i18n("Choose windowing function here. If fourier transformation should stay reversible, use the type <none>"));
  
  windowlabel	=new QLabel	("",this);
  bitmaplabel	=new QLabel	("",this);
  pointslider	=new Slider (2,(len/16),1,5,Slider::Horizontal,this);
  windowtypelabel=new QLabel	(i18n("Window Function :"),this);

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
const char *SonagramDialog::getCommand ()
{
  deleteString (comstr);
  //  return windowtypebox->currentText();
  return comstr;
}
//**********************************************************
void SonagramDialog::setPoints (int points)
{
  char buf[32];
  points*=2;
  
  sprintf (buf,"%d",points);
  pointbox->changeItem (buf,0);
  pointbox->setCurrentItem (0);
  sprintf (buf,i18n("resulting window size: %s"),(mstotimec(points*10000/rate)));  
  windowlabel->setText(buf);
  sprintf (buf,i18n("size of bitmap: %dx%d"),(length/points)+1,points/2);  
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
  deleteString (comstr);
}













