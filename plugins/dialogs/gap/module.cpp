#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="gap";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new GapDialog (operation->getRate(),operation->isModal());
}
//**********************************************************
GapDialog::GapDialog (int rate,bool modal): KwaveDialog(modal)
{
  resize  (320,200);
  setCaption	(klocale->translate("Set Length of Gap & Signal"));

  label1=new QLabel (klocale->translate("length of gap"),this);
  len1=new TimeLine (this,rate);
  len1->setSamples (200);
  label2=new QLabel (klocale->translate("length of signal"),this);
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
const char *GapDialog::getCommand ()
{
  if (comstr) free (comstr);
  char buf[512];
  sprintf (buf,"gap (%f %f)",len1->getMs(),len2->getMs());
  comstr=strdup (buf);
  return comstr;
}
//**********************************************************
void GapDialog::resizeEvent (QResizeEvent *)
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
GapDialog::~GapDialog ()
{
  if (comstr) free (comstr);
}
//**********************************************************









