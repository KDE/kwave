#include <stdio.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"

#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="fadeout";
//**********************************************************
Dialog *getDialog (DialogOperation *operation)
{
  return new FadeDialog(operation->isModal(),operation->getLength());
}
//**********************************************************
FadeDialog::FadeDialog (bool modal,int ms): Dialog(modal)
{
  comstr=0;
  setCaption	(i18n("Choose fading degree :"));

  ok	 =new QPushButton (OK,this);
  cancel =new QPushButton (CANCEL,this);
  slider =new Slider (-100,100,1,0,Slider::Horizontal,this);     
  fade   =new FaderWidget (this,0);

  x=new ScaleWidget (this,0,ms,"ms");
  y=new ScaleWidget (this,100,0,"%");
  corner=new CornerPatchWidget (this);

  int bsize=ok->sizeHint().height();

  setMinimumSize (320,bsize*9);
  resize	 (320,bsize*9);

  ok->setAccel	(Key_Return);
  cancel->setAccel(Key_Escape);
  ok->setFocus	();
  connect 	(ok	,SIGNAL(clicked()),SLOT (accept()));
  connect 	(slider	,SIGNAL(valueChanged(int)),fade,SLOT (setCurve(int)));
  connect 	(cancel	,SIGNAL(clicked()),SLOT (reject()));
}
//**********************************************************
const char *FadeDialog::getCommand ()
{
  deleteString (comstr);
  comstr=catString ("fadeout (",fade->getDegree(),")");
  return comstr;
}
//**********************************************************
void FadeDialog::resizeEvent (QResizeEvent *)
{
 int bsize=ok->sizeHint().height();
 int half=height()-bsize*7/2;

 fade   ->setGeometry	(8+bsize,0,width()-16-bsize,half-bsize);  
 y      ->setGeometry	(8,0,bsize,half-bsize);
 x      ->setGeometry	(8+bsize,half-bsize,width()-16,bsize);
 corner ->setGeometry   (8,half-bsize,bsize,bsize);

 slider->setGeometry	(width()/20,height()-bsize*3,width()*18/20,bsize);  
 ok->setGeometry	(width()/10,height()-bsize*3/2,width()*3/10,bsize);  
 cancel->setGeometry	(width()*6/10,height()-bsize*3/2,width()*3/10,bsize);  
}
//**********************************************************
FadeDialog::~FadeDialog ()
{
  deleteString (comstr);
}









